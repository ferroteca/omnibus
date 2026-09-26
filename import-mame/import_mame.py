#!/usr/bin/env python3
"""Import the MAME sources needed to build the machines in machines.lst.

machines.lst, in the same directory as this script, lists the MAME
machine names to build, one per line. Blank lines and lines starting
with # are ignored. This script reads one commit of a MAME git
repository, finds where each listed machine is defined, works out
which source files those machines need, and copies those files into
the directory above this script. It then deletes the tracked files
that an earlier run imported but this run did not.

Usage:

    import_mame.py [MAME_DIR] [--rev TAG_OR_HASH]

MAME_DIR is the root of a MAME git repository. The revision is a tag
such as mame0289 or a commit hash. The files are read from that commit
with git, so it does not matter what the repository has checked out.

Both values can come from local.toml in the same directory as the
script instead. A command line value wins over the file. local.toml is
ignored by git because the path differs per machine. It holds:

    mame_dir = "D:/Projects/mame-mirror"
    mame_rev = "mame0289"

Copy local.example.toml to local.toml and edit the values.

WHAT GETS IMPORTED
------------------
The selected machines are the ones named in machines.lst, plus any
parent of one of them that is not itself listed. "mame -validate"
reports a clone whose parent is missing as an error, so the parent
comes along, and the run says so. A name that no driver defines stops
the run. MAME defines a machine with one of the GAME(), GAMEL(),
CONS(), COMP() and SYST() macros at the end of its driver source file
(see src/emu/gamedrv.h); the script reads those to find the driver
file and the parent of each machine.

The script follows #include lines from the selected drivers the way
scripts/build/makedep.py does for a SOURCES= build: a quoted include
is looked up in the including file's directory, then src/devices,
src/mame/shared and src/lib. A header pulls in the .cpp, .ipp and
.hxx files with the same base name, and under src/mame also the _a,
_v and _m variants. The files reached this way are the include
closure.

MAME compiles device sources in blocks that scripts/src/cpu.lua,
sound.lua, video.lua, machine.lua, bus.lua and formats.lua guard with
flags such as CPUS["Z80"]. Each of those files also carries lines of
the form

    --@src/devices/cpu/z80/z80.h,CPUS["Z80"] = true

that name the header which turns a flag on. A flag is on when its
header is in the include closure. The script evaluates the guarded
blocks with those flags, adds the files they name, follows their
includes too, and repeats until nothing new turns up.

Four parts of the tree are pruned to what that closure needs:

- src/mame: the selected drivers, the files they include, and
  src/mame/mame.cpp, which the build compiles as the target's main
  file. src/mame/mame.lst is written fresh with only the selected
  machines, because every name in it must have a compiled driver or
  the link fails.
- src/mame/layout: the layouts that an imported file includes as a
  .lh file.
- src/devices and src/lib/formats: the files in the include closure
  plus the files named in the guarded blocks whose flag is on. That
  includes generator scripts and tables named in custombuildtask
  entries.
- src/lib/netlist: all of it when MACHINES["NETLIST"] is on, none of
  it otherwise.

Everything else in the tree is copied as is: src/emu, src/lib/util,
src/osd, src/frontend, src/tools, 3rdparty, scripts, docs, hash and
so on. The full build compiles or ships all of those regardless of
which machines are in the list. For that reason the closure starts
from the source files in src/emu, src/frontend and src/osd as well as
from the drivers: a device header that only the UI includes must still
be imported. src/emu/drivers/testcpu.cpp is left out, because the mame
target does not compile it.

scripts/target/mame/mame.lua is edited so that it links the "shared"
library only when src/mame/shared exists. That directory is pruned
like the rest of src/mame, so it is missing when no selected driver
uses it.

The six lua files are copied and then edited: the "--@" lines whose
header is not in the closure are removed. scripts/target/mame/mame.lua
turns on every flag those lines name, so after the edit the build
enables exactly the flags the imported files support and never asks
for a file that was pruned.

FILES NEXT TO THIS SCRIPT
-------------------------
machines.lst   input: the machine names to build, tracked by git
local.toml     input: the repository path and revision, ignored by git
source.lst     output: every path this run imported, one per line,
               sorted; ignored by git and only there for reference

MAME's own .gitignore, which the import copies, ignores every top
level directory it does not name, and that includes this one. Files
already tracked stay tracked. A new file added here needs "git add -f"
the first time. The script does not edit .gitignore.

WHAT GETS DELETED
-----------------
After the copy, every file that git tracks in the target and that is
not in the import list is deleted, and empty directories are removed.
Untracked files are never touched, whether git ignores them or not, so
build output, local settings and files you have not added to git all
survive. Files under this script's own directory are never deleted
either. The target must be a git repository, because the script asks
git which files it tracks. The deletions are left for you to stage.

Machine macro calls that are commented out or sit inside an "#if 0"
block are ignored, because MAME does not build those machines.
"""

import argparse
import os
import posixpath
import re
import subprocess
import sys
import tempfile
import threading
import tomllib
from pathlib import Path

SCRIPT_DIR = Path(__file__).resolve().parent
CONFIG_FILE = SCRIPT_DIR / "local.toml"
MACHINES_FILE = SCRIPT_DIR / "machines.lst"
MANIFEST_FILE = SCRIPT_DIR / "source.lst"
TARGET_DIR = SCRIPT_DIR.parent

# Number of arguments each machine macro takes. The machine name is the
# second argument and the parent the third in all of them.
MACHINE_MACROS = {"GAME": 11, "GAMEL": 12, "CONS": 11, "COMP": 11, "SYST": 11}

# Files under src/mame that are not needed to build the selected
# machines. The .flt and extra .lst files only matter for other build
# targets, and mame.lst is regenerated.
SKIPPED_MAME_ROOT_FILES = {"ci.flt", "nl.flt", "virtual.flt", "dummy.lst",
                           "tiny.lst", "mame.lst"}

SOURCE_SUFFIXES = (".cpp", ".h", ".ipp", ".hxx")

# The lua files whose "--@" lines and guarded blocks decide which
# device and format sources get compiled.
DEVICE_LUA_FILES = [f"scripts/src/{name}.lua" for name in
                    ("cpu", "sound", "video", "machine", "bus", "formats")]
FLAG_SETS = ("CPUS", "SOUNDS", "VIDEOS", "MACHINES", "BUSES", "FORMATS")

# Directories whose contents are chosen file by file. Everything
# outside them is imported whole.
PRUNED_DIRS = ("src/mame/", "src/devices/", "src/lib/formats/", "src/lib/netlist/")
# Directories whose source files are scanned for #include lines.
SCANNED_DIRS = ("src/mame/", "src/devices/", "src/lib/formats/")
# Directories that are imported whole and compiled in every build. Their
# includes into the scanned directories start the closure along with
# the drivers. src/frontend/mame/ui/filesel.cpp, for example, includes
# bus/midi/midiinport.h whatever machines are built.
ALWAYS_BUILT_DIRS = ("src/emu/", "src/frontend/", "src/osd/")
# Files in those directories that the mame target does not compile.
NOT_BUILT_FILES = {"src/emu/drivers/testcpu.cpp"}


# --- C++ source parsing ------------------------------------------------------

def strip_comments(text):
    """Replace C and C++ comments with spaces.

    String and character literals are left alone, so a "//" inside a
    quoted string does not start a comment. Newlines inside block
    comments are kept so line numbers stay the same.
    """
    out = []
    i = 0
    n = len(text)
    while i < n:
        c = text[i]
        nxt = text[i + 1] if i + 1 < n else ""
        if c == "/" and nxt == "*":
            end = text.find("*/", i + 2)
            if end == -1:
                end = n
            else:
                end += 2
            out.append("".join("\n" if ch == "\n" else " " for ch in text[i:end]))
            i = end
        elif c == "/" and nxt == "/":
            end = text.find("\n", i)
            if end == -1:
                end = n
            out.append(" " * (end - i))
            i = end
        elif c in "\"'":
            quote = c
            j = i + 1
            while j < n and text[j] != quote:
                if text[j] == "\\":
                    j += 1
                j += 1
            j = min(j + 1, n)
            out.append(text[i:j])
            i = j
        else:
            out.append(c)
            i += 1
    return "".join(out)


_IF_RE = re.compile(r"^\s*#\s*(if|ifdef|ifndef|else|elif|endif)\b\s*(.*)$")


def strip_if0_blocks(text):
    """Blank out the lines inside "#if 0" ... "#endif" (or "#else").

    Only the literal "#if 0" is treated as disabled. Every other
    conditional is kept, because the script cannot know how MAME's
    build would evaluate it. Nesting is tracked so an inner #endif
    does not end an outer "#if 0".
    """
    lines = text.split("\n")
    out = []
    # Each entry is True when that #if level is a disabled "#if 0".
    stack = []
    for line in lines:
        m = _IF_RE.match(line)
        disabled = any(stack)
        if m:
            directive, rest = m.group(1), m.group(2).strip()
            if directive in ("if", "ifdef", "ifndef"):
                stack.append(directive == "if" and rest == "0")
            elif directive in ("else", "elif"):
                # The branch after "#if 0" is live. After any other
                # conditional the code was already being kept.
                if stack:
                    stack[-1] = False
            elif directive == "endif":
                if stack:
                    stack.pop()
            out.append("")
            continue
        out.append("" if disabled else line)
    return "\n".join(out)


def strip_preprocessor_lines(text):
    """Blank out #define and every other preprocessor line.

    A line that ends with a backslash continues on the next line, and
    those lines are blanked too. Some drivers define helper macros
    whose body is a GAME() call with the macro's own parameters as
    arguments; without this step those would look like machines.
    Run strip_if0_blocks() first, because it needs the #if lines.
    """
    out = []
    continued = False
    for line in text.split("\n"):
        if continued or line.lstrip().startswith("#"):
            continued = line.rstrip().endswith("\\")
            out.append("")
        else:
            out.append(line)
    return "\n".join(out)


def split_args(arg_text):
    """Split macro arguments on the commas at nesting depth zero."""
    args = []
    depth = 0
    cur = []
    i = 0
    n = len(arg_text)
    while i < n:
        c = arg_text[i]
        if c in "\"'":
            quote = c
            j = i + 1
            while j < n and arg_text[j] != quote:
                if arg_text[j] == "\\":
                    j += 1
                j += 1
            j = min(j + 1, n)
            cur.append(arg_text[i:j])
            i = j
            continue
        if c in "([{":
            depth += 1
        elif c in ")]}":
            depth -= 1
        if c == "," and depth == 0:
            args.append("".join(cur).strip())
            cur = []
        else:
            cur.append(c)
        i += 1
    args.append("".join(cur).strip())
    return args


_MACRO_RE = re.compile(r"^[ \t]*(GAME|GAMEL|CONS|COMP|SYST)[ \t]*\(", re.MULTILINE)


def find_macro_calls(text):
    """Yield (macro, argument text) for each machine macro call.

    The file must already have its comments and "#if 0" blocks
    removed. A call may span several lines; the scan runs to the
    parenthesis that closes the one opened after the macro name.
    """
    for m in _MACRO_RE.finditer(text):
        start = m.end()
        depth = 1
        i = start
        n = len(text)
        while i < n and depth:
            c = text[i]
            if c in "\"'":
                quote = c
                i += 1
                while i < n and text[i] != quote:
                    if text[i] == "\\":
                        i += 1
                    i += 1
            elif c == "(":
                depth += 1
            elif c == ")":
                depth -= 1
            i += 1
        yield m.group(1), text[start:i - 1]


_PP_RE = re.compile(r'^[ \t]*#[ \t]*(\w+)[ \t]*(.*?)[ \t]*$')
_HAS_GUARD_RE = re.compile(r'^HAS_(CPUS|SOUNDS|VIDEOS|MACHINES|BUSES|FORMATS)_([A-Z0-9_]+)$')


def find_includes(text):
    """Return [(include, guards)] for the quoted #include lines.

    guards is the set of device flags the include depends on. It is
    filled from enclosing "#ifdef HAS_FORMATS_X" lines, which genie
    defines from the FORMATS flags. src/lib/formats/all.cpp wraps its
    include of every format header in one of those, so without the
    guard every format would look needed. Any other #if keeps the
    include, and so does the #else branch of a guard; that matches
    makedep.py, which follows includes inside "#if 0" too.
    """
    found = []
    stack = []
    for line in text.split("\n"):
        m = _PP_RE.match(line)
        if not m:
            continue
        directive, rest = m.group(1), m.group(2)
        if directive == "include":
            if len(rest) > 2 and rest[0] == '"' and rest[-1] == '"':
                guards = frozenset(g for g in stack if g)
                found.append((rest[1:-1], guards))
        elif directive == "ifdef":
            g = _HAS_GUARD_RE.match(rest)
            stack.append((g.group(1), g.group(2)) if g else None)
        elif directive in ("if", "ifndef"):
            stack.append(None)
        elif directive in ("else", "elif"):
            if stack:
                stack[-1] = None
        elif directive == "endif":
            if stack:
                stack.pop()
    return found


class Machine:
    __slots__ = ("name", "parent", "macro", "source")

    def __init__(self, name, parent, macro, source):
        self.name = name
        self.parent = "" if parent == "0" else parent
        self.macro = macro
        self.source = source


def parse_source(text, source, want_machines=True):
    """Return (machines, includes) for one source file.

    machines holds a Machine for every GAME/GAMEL/CONS/COMP/SYST call
    that is not commented out or inside "#if 0"; it is empty when
    want_machines is False. includes is what find_includes() returns.
    """
    text = strip_comments(text)
    includes = find_includes(text)
    machines = []
    if not want_machines:
        return machines, includes
    for macro, arg_text in find_macro_calls(
            strip_preprocessor_lines(strip_if0_blocks(text))):
        args = split_args(arg_text)
        if len(args) != MACHINE_MACROS[macro]:
            print(f"{source}: {macro}() has {len(args)} arguments, expected "
                  f"{MACHINE_MACROS[macro]}; skipped", file=sys.stderr)
            continue
        machines.append(Machine(args[1], args[2], macro, source))
    return machines, includes


# --- reading the MAME repository --------------------------------------------

def git(repo, *args, **kwargs):
    """Run a git command in repo and return the completed process."""
    return subprocess.run(["git", "-C", str(repo), *args],
                          capture_output=True, check=False, **kwargs)


def resolve_commit(repo, rev):
    """Turn a tag or hash into a full commit hash, or stop with a message."""
    if not (repo / ".git").exists():
        raise SystemExit(f"{repo}: not a git repository")
    result = git(repo, "rev-parse", "--verify", "--quiet", f"{rev}^{{commit}}")
    if result.returncode != 0:
        raise SystemExit(f"{repo}: {rev} is not a tag or commit in this repository")
    return result.stdout.decode().strip()


def list_tree(repo, commit):
    """Return every file path in the commit, as posix paths."""
    listing = git(repo, "ls-tree", "-r", "-z", "--name-only", commit)
    if listing.returncode != 0:
        raise SystemExit(listing.stderr.decode().strip())
    return [p for p in listing.stdout.decode().split("\0") if p]


def read_files(repo, commit, paths):
    """Yield (path, text) for each path at commit.

    One "git cat-file --batch" process streams every file. Reading them
    one "git show" at a time takes minutes on Windows; this takes
    seconds. The request list is fed from a thread so the output can
    be consumed while git is still working.
    """
    proc = subprocess.Popen(["git", "-C", str(repo), "cat-file", "--batch"],
                            stdin=subprocess.PIPE, stdout=subprocess.PIPE)

    def feed():
        for p in paths:
            proc.stdin.write(f"{commit}:{p}\n".encode())
        proc.stdin.close()

    threading.Thread(target=feed, daemon=True).start()
    out = proc.stdout
    for path in paths:
        header = out.readline().decode().rstrip("\n")
        if header.endswith(" missing"):
            raise SystemExit(f"{repo}: git could not read {path} at {commit}")
        size = int(header.rsplit(" ", 1)[1])
        body = out.read(size)
        out.read(1)  # the newline git writes after each object
        yield path, body.decode("utf-8", errors="replace")
    proc.wait()


def extract_files(repo, commit, paths, target):
    """Write the given paths from commit into target, overwriting.

    A temporary index holds the commit's tree so "git checkout-index"
    can write the files. The repository's own index is not touched.
    Line endings follow the repository's core.autocrlf setting, the
    same as a normal checkout.
    """
    fd, index_path = tempfile.mkstemp(prefix="import-mame-index-")
    os.close(fd)
    os.unlink(index_path)
    env = dict(os.environ, GIT_INDEX_FILE=index_path)
    try:
        result = git(repo, "read-tree", commit, env=env)
        if result.returncode != 0:
            raise SystemExit(result.stderr.decode().strip())
        prefix = target.resolve().as_posix().rstrip("/") + "/"
        query = "".join(p + "\0" for p in paths).encode()
        result = git(repo, "checkout-index", "-f", "-z", "--stdin",
                     f"--prefix={prefix}", env=env, input=query)
        if result.returncode != 0:
            raise SystemExit(result.stderr.decode().strip())
    finally:
        if os.path.exists(index_path):
            os.unlink(index_path)


# --- the include closure -----------------------------------------------------

def scan_sources(repo, commit, tree):
    """Parse every source file under the scanned and always-built
    directories at commit.

    Returns (machines, includes): a dict of machine name to Machine
    for the files under src/mame, and a dict of source path to its
    list of quoted includes for every parsed file.
    """
    sources = [p for p in tree
               if p.startswith(SCANNED_DIRS + ALWAYS_BUILT_DIRS)
               and p.endswith(SOURCE_SUFFIXES)]
    machines = {}
    includes = {}
    for path, text in read_files(repo, commit, sources):
        found, incs = parse_source(text, path, path.startswith("src/mame/"))
        includes[path] = incs
        for m in found:
            if m.name in machines:
                print(f"{path}: machine {m.name} is also defined in "
                      f"{machines[m.name].source}", file=sys.stderr)
            machines[m.name] = m
    return machines, includes


def always_built_sources(tree):
    """The source files under ALWAYS_BUILT_DIRS that the build compiles
    or includes, whatever machines are selected."""
    return {p for p in tree
            if p.startswith(ALWAYS_BUILT_DIRS) and p.endswith(SOURCE_SUFFIXES)
            and p not in NOT_BUILT_FILES}


def read_machine_list(path):
    """Return the machine names in machines.lst, in file order.

    Blank lines and lines starting with # are skipped. A missing or
    empty file stops the run, since importing nothing is never the
    intent.
    """
    if not path.is_file():
        raise SystemExit(f"{path} does not exist; list the machines to build in it, "
                         f"one per line")
    names = []
    for line in path.read_text(encoding="utf-8").splitlines():
        line = line.split("#", 1)[0].strip()
        if line:
            names.append(line)
    if not names:
        raise SystemExit(f"{path} names no machines")
    return names


def select_machines(machines, requested):
    """Return the requested machines plus the parents they need, by name.

    Every requested name must be defined by some driver; otherwise the
    run stops and names the missing ones.
    """
    missing = [name for name in requested if name not in machines]
    if missing:
        raise SystemExit("not defined by any driver at this revision: "
                         + ", ".join(missing))
    selected = set(requested)
    added_parents = set()
    pending = list(selected)
    while pending:
        parent = machines[pending.pop()].parent
        if parent and parent not in selected:
            if parent not in machines:
                print(f"{parent}: parent of a selected machine but not "
                      f"defined anywhere; skipped", file=sys.stderr)
                continue
            selected.add(parent)
            added_parents.add(parent)
            pending.append(parent)
    return selected, added_parents


def resolve_include(including, include, tree):
    """Find the file a quoted #include refers to, or None.

    The search order is the including file's own directory, then
    src/devices, src/mame/shared and src/lib, which are the include
    directories the MAME build gives the mame target.
    """
    candidates = [
        posixpath.normpath(posixpath.join(posixpath.dirname(including), include)),
        "src/devices/" + include,
        "src/mame/shared/" + include,
        "src/lib/" + include,
    ]
    for c in candidates:
        if c in tree:
            return c
    return None


def companion_files(header, tree):
    """The source files that go with a header.

    makedep.py treats "foo.h" as pulling in foo.cpp, foo.ipp and
    foo.hxx, and under src/mame also foo_a, foo_v and foo_m with those
    extensions. Only files that exist are returned.
    """
    base = header.rsplit(".", 1)[0]
    aspects = ("", "_a", "_v", "_m") if header.startswith("src/mame/") else ("",)
    found = []
    for aspect in aspects:
        for ext in (".cpp", ".ipp", ".hxx"):
            p = base + aspect + ext
            if p in tree:
                found.append(p)
    return found


def extend_closure(closure, layouts, start, includes, tree, flags=frozenset(), deferred=None):
    """Add start and everything it includes to closure, in place.

    Only files under the scanned directories are followed. Files that
    resolve elsewhere, such as src/emu, are imported whole anyway.
    Names of layouts included as .lh files go into layouts. An include
    whose guards are not all in flags is not followed; it is appended
    to deferred as (including file, include, guards) so a later pass
    with more flags can follow it.
    """
    pending = list(start)
    while pending:
        path = pending.pop()
        if path in closure:
            continue
        closure.add(path)
        for inc, guards in includes.get(path, ()):
            if guards and not guards <= flags:
                if deferred is not None:
                    deferred.append((path, inc, guards))
                continue
            if inc.endswith(".lh"):
                layouts.add(inc[:-3])
                continue
            resolved = resolve_include(path, inc, tree)
            if resolved is None or not resolved.startswith(SCANNED_DIRS):
                continue
            pending.append(resolved)
            if resolved.rsplit(".", 1)[-1].lower().startswith("h"):
                pending.extend(companion_files(resolved, tree))


# --- the device lua files ----------------------------------------------------

_DIRECTIVE_RE = re.compile(r'^--@([^,]+),\s*(CPUS|SOUNDS|VIDEOS|MACHINES|BUSES|FORMATS)\["([A-Z0-9_]+)"\]\s*=\s*true\s*$')
_FLAG_RE = re.compile(r'(CPUS|SOUNDS|VIDEOS|MACHINES|BUSES|FORMATS)\["([A-Z0-9_]+)"\](?:\s*~=\s*null)?')
_OPT_TOOL_RE = re.compile(r'opt_tool\((CPUS|SOUNDS|VIDEOS|MACHINES|BUSES|FORMATS),\s*"([A-Z0-9_]+)"\)')
_ASSIGN_RE = re.compile(r'^(CPUS|SOUNDS|VIDEOS|MACHINES|BUSES|FORMATS)\["([A-Z0-9_]+)"\]\s*=\s*true$')
_LOCAL_RE = re.compile(r'^local\s+(\w+)\s*=\s*(.+)$')
_DRC_RE = re.compile(r'^DRC_CPUS\s*=\s*\{(.*)\}')
_SRC_STRING_RE = re.compile(r'MAME_DIR\s*\.\.\s*"(src/[^"]+)"')


def parse_directives(text):
    """Return [(header path, (set, name))] for the "--@" lines."""
    found = []
    for line in text.splitlines():
        m = _DIRECTIVE_RE.match(line.strip())
        if m:
            found.append((m.group(1).strip(), (m.group(2), m.group(3))))
    return found


def lua_condition(cond, flags, variables, path):
    """Evaluate the condition of an "if ... then" line in a lua file.

    Flags such as CPUS["Z80"] and opt_tool(CPUS, "Z80") become True or
    False from flags. _OPTIONS values are False, since the build is
    not configured with tools. Local variables such as want_disasm_z80
    come from variables. Anything else is an error, because a wrong
    guess would silently drop or keep files.
    """
    expr = _OPT_TOOL_RE.sub(lambda m: str((m.group(1), m.group(2)) in flags), cond)
    expr = _FLAG_RE.sub(lambda m: str((m.group(1), m.group(2)) in flags), expr)
    expr = re.sub(r'CPUS\[v\]\s*~=\s*null', str(variables.get("CPU_INCLUDE_DRC", False)), expr)
    expr = re.sub(r'_OPTIONS\["[^"]+"\]', "False", expr)
    expr = re.sub(r'\b(\w+)\b',
                  lambda m: str(variables[m.group(1)]) if m.group(1) in variables else m.group(1),
                  expr)
    if not re.fullmatch(r'[\s()]*((True|False|and|or|not)[\s()]*)+', expr):
        raise SystemExit(f"{path}: cannot evaluate lua condition: {cond}")
    return bool(eval(expr, {"__builtins__": {}}, {}))  # only True/False/and/or/not remain


def lua_required_files(text, flags, path):
    """Walk one of the six lua files and return (paths, new flags).

    paths is every "src/..." file named inside a block whose guard is
    true with the given flags, including the unguarded parts of the
    file. new flags is the set of flags the file turns on itself, such
    as MACHINES["SCSI"] = true inside a bus block.
    """
    paths = set()
    new_flags = set()
    variables = {}
    stack = []
    for raw in text.splitlines():
        line = raw.strip()
        if not line or line.startswith("--"):
            continue
        active = all(stack)
        m = _DRC_RE.match(line)
        if m:
            drc = re.findall(r'"([A-Z0-9_]+)"', m.group(1))
            variables["CPU_INCLUDE_DRC"] = any(("CPUS", c) in flags for c in drc)
            # The native back end is built on x86 and arm64 hosts, which
            # is where this fork is built.
            variables["CPU_INCLUDE_DRC_NATIVE"] = variables["CPU_INCLUDE_DRC"]
            continue
        m = _LOCAL_RE.match(line)
        if m:
            variables[m.group(1)] = lua_condition(m.group(2), flags, variables, path)
            continue
        m = re.match(r'^if\s+(.*?)\s+then$', line)
        if m:
            stack.append(active and lua_condition(m.group(1), flags, variables, path))
            continue
        if line.startswith("elseif "):
            raise SystemExit(f"{path}: elseif is not supported: {line}")
        if line == "else":
            stack[-1] = all(stack[:-1]) and not stack[-1]
            continue
        if re.match(r'^(for\b.*\bdo|function\b.*)$', line):
            stack.append(active)
            continue
        if line == "end" or line.startswith("end "):
            stack.pop()
            continue
        if not active:
            continue
        m = _ASSIGN_RE.match(line)
        if m:
            new_flags.add((m.group(1), m.group(2)))
        paths.update(_SRC_STRING_RE.findall(line))
    if stack:
        raise SystemExit(f"{path}: unbalanced blocks")
    return paths, new_flags


def evaluate_device_lua(lua_texts, flags):
    """Evaluate all six lua files until the flags stop changing.

    Returns (paths, flags): the files the build would compile or use
    and the final set of flags, including those the files turn on
    themselves.
    """
    flags = set(flags)
    while True:
        paths = set()
        new_flags = set()
        for path, text in lua_texts.items():
            found, turned_on = lua_required_files(text, flags, path)
            paths |= found
            new_flags |= turned_on
        if new_flags <= flags:
            return paths, flags
        flags |= new_flags


def required_files(driver_sources, includes, tree, lua_texts):
    """Work out the include closure, flags, layouts and lua files.

    Starts from the drivers, turns on the flags whose headers are in
    the closure, adds the files the lua blocks name under those flags,
    follows their includes, and repeats until nothing changes.
    Returns (closure, layouts, flags, lua_paths).
    """
    tree = set(tree)
    directives = {}
    for text in lua_texts.values():
        for header, flag in parse_directives(text):
            directives.setdefault(header, set()).add(flag)
    closure = set()
    layouts = set()
    deferred = []
    extend_closure(closure, layouts, driver_sources, includes, tree, frozenset(), deferred)
    while True:
        flags = set()
        for path in closure:
            flags |= directives.get(path, set())
        lua_paths, flags = evaluate_device_lua(lua_texts, flags)
        new = {p for p in lua_paths if p in tree} - closure
        # Guarded includes whose flags are now on get followed as if
        # their including file were scanned again.
        ready = [d for d in deferred if d[2] <= flags]
        if not new and not ready:
            return closure, layouts, flags, lua_paths
        deferred = [d for d in deferred if not d[2] <= flags]
        for path, inc, _ in ready:
            closure.discard(path)
            extend_closure(closure, layouts, [path], {path: [(inc, frozenset())]}, tree)
        extend_closure(closure, layouts, new, includes, tree, flags, deferred)


def choose_files(tree, closure, lua_paths, layouts, netlist):
    """Return the sorted list of paths to import from the commit."""
    chosen = []
    for p in tree:
        if not p.startswith(PRUNED_DIRS):
            chosen.append(p)
        elif p in closure or p in lua_paths:
            chosen.append(p)
        elif p.startswith("src/lib/netlist/"):
            if netlist:
                chosen.append(p)
        elif p.startswith("src/mame/layout/"):
            if p.endswith(".lay") and p[len("src/mame/layout/"):-4] in layouts:
                chosen.append(p)
        elif p == "src/mame/mame.cpp":
            chosen.append(p)
        elif p.startswith("src/mame/") and "/" not in p[len("src/mame/"):]:
            if p[len("src/mame/"):] not in SKIPPED_MAME_ROOT_FILES:
                chosen.append(p)
    return sorted(chosen)


def filter_directives(text, closure):
    """Drop the "--@" lines whose header is not in the closure.

    mame.lua turns on every flag named by a "--@" line, so after this
    the full build enables only the flags the imported files support.
    """
    kept = []
    for line in text.splitlines(keepends=True):
        m = _DIRECTIVE_RE.match(line.strip())
        if m and m.group(1).strip() not in closure:
            continue
        kept.append(line)
    return "".join(kept)


def mame_lst_text(machines, selected, rev, commit):
    """The contents of src/mame/mame.lst for the selected machines."""
    by_source = {}
    for name in selected:
        by_source.setdefault(machines[name].source, []).append(name)
    lines = [
        "// license:BSD-3-Clause",
        f"// Written by import-mame/import_mame.py from MAME {rev} ({commit[:12]}).",
        "// Do not edit by hand; run the script again instead.",
    ]
    for source in sorted(by_source):
        lines.append("")
        lines.append("@source:" + source[len("src/mame/"):])
        lines.extend(sorted(by_source[source]))
    return "\n".join(lines) + "\n"


MAME_TARGET_LUA = "scripts/target/mame/mame.lua"
SHARED_LINK_LINE = 'table.insert(projects, "shared") -- must stay at the end'


def link_shared_only_if_present(text):
    """Make mame.lua link the "shared" library only when src/mame/shared
    exists.

    mame.lua always links "shared", but it creates that project only
    from the files in src/mame/shared. When no selected driver needs
    those files the import leaves the directory out, and the link then
    fails with "cannot find -lshared".
    """
    lines = text.splitlines(keepends=True)
    for i, line in enumerate(lines):
        if line.strip() == SHARED_LINK_LINE:
            indent = line[:len(line) - len(line.lstrip())]
            eol = line[len(line.rstrip("\r\n")):]
            lines[i] = (f'{indent}if os.isdir(path.join(MAME_DIR, "src", _target, "shared")) then{eol}'
                        f'{indent}\t{SHARED_LINK_LINE}{eol}'
                        f'{indent}end{eol}')
            return "".join(lines)
    raise SystemExit(f"{MAME_TARGET_LUA}: cannot find the line "
                     f"'{SHARED_LINK_LINE}' to edit")


# --- the target directory ----------------------------------------------------

def rewrite_in_place(path, transform):
    """Apply transform to a text file, keeping its line endings."""
    text = path.read_text(encoding="utf-8", newline="")
    path.write_text(transform(text), encoding="utf-8", newline="")


def tracked_files(target):
    """Every path git tracks in target, relative with forward slashes."""
    result = git(target, "ls-files", "-z")
    if result.returncode != 0:
        raise SystemExit(result.stderr.decode().strip())
    return [p for p in result.stdout.decode().split("\0") if p]


def delete_stale(target, imported):
    """Delete the tracked files under target that this run did not import.

    Untracked files are never touched, and neither is anything under
    the script's own directory. Empty directories left behind are
    removed too. Returns the number of files deleted.
    """
    own_dir = SCRIPT_DIR.name + "/"
    deleted = 0
    for rel in sorted(tracked_files(target), reverse=True):
        if rel in imported or rel.startswith(own_dir):
            continue
        path = target / rel
        if not path.is_file():
            continue
        path.unlink()
        deleted += 1
        parent = path.parent
        while parent != target and parent.is_dir() and not any(parent.iterdir()):
            parent.rmdir()
            parent = parent.parent
    return deleted


# --- settings and main -------------------------------------------------------

def read_config(config_path):
    """Return the settings in local.toml, or {} when the file is absent."""
    if not config_path.is_file():
        return {}
    with open(config_path, "rb") as f:
        return tomllib.load(f)


def resolve_settings(cli_dir, cli_rev, config_path=CONFIG_FILE):
    """Pick the repository path and revision.

    A command line value wins over local.toml. If either value is
    missing from both places the script stops with a message that
    says how to set it.
    """
    config = read_config(config_path)
    how = (f"Pass it on the command line, or copy local.example.toml to "
           f"{config_path.name} and set it there.")
    mame_dir = cli_dir or config.get("mame_dir")
    if not isinstance(mame_dir, str) or not mame_dir:
        raise SystemExit(f"no MAME repository given (mame_dir). {how}")
    rev = cli_rev or config.get("mame_rev")
    if not isinstance(rev, str) or not rev:
        raise SystemExit(f"no MAME tag or commit given (mame_rev). {how}")
    return Path(mame_dir), rev


def run_import(repo, rev, target, machines_file, manifest_file):
    """Do the whole import. Returns a dict of counts for the summary."""
    repo = Path(repo)
    if not (target / ".git").exists():
        raise SystemExit(f"{target}: not a git repository; the script needs "
                         f"git to know which files it tracks")
    commit = resolve_commit(repo, rev)
    tree = list_tree(repo, commit)
    tree_set = set(tree)

    requested = read_machine_list(machines_file)
    machines, includes = scan_sources(repo, commit, tree)
    selected, added_parents = select_machines(machines, requested)
    driver_sources = {machines[name].source for name in selected}

    lua_texts = dict(read_files(repo, commit, [p for p in DEVICE_LUA_FILES if p in tree_set]))
    closure, layouts, flags, lua_paths = required_files(
        driver_sources | always_built_sources(tree), includes, tree, lua_texts)
    netlist = ("MACHINES", "NETLIST") in flags
    paths = choose_files(tree, closure, lua_paths, layouts, netlist)

    extract_files(repo, commit, paths, target)
    for lua in lua_texts:
        rewrite_in_place(target / lua, lambda text: filter_directives(text, closure))
    if MAME_TARGET_LUA in tree_set:
        rewrite_in_place(target / MAME_TARGET_LUA, link_shared_only_if_present)
    lst_path = target / "src" / "mame" / "mame.lst"
    lst_path.write_text(mame_lst_text(machines, selected, rev, commit),
                        encoding="utf-8", newline="\n")
    imported = set(paths) | {"src/mame/mame.lst"}

    deleted = delete_stale(target, imported)

    manifest_file.write_text("".join(p + "\n" for p in sorted(imported)),
                             encoding="utf-8", newline="\n")

    def count(prefix):
        return sum(1 for p in imported if p.startswith(prefix))

    return {
        "commit": commit,
        "listed": len(requested),
        "parents": sorted(added_parents),
        "driver_files": len(driver_sources),
        "mame_files": count("src/mame/"),
        "layouts": count("src/mame/layout/"),
        "device_files": count("src/devices/"),
        "format_files": count("src/lib/formats/"),
        "netlist_files": count("src/lib/netlist/"),
        "flags": len(flags),
        "imported": len(imported),
        "deleted": deleted,
    }


def main(argv=None):
    parser = argparse.ArgumentParser(
        description="Import the MAME sources needed to build the machines "
                    "in machines.lst into the directory above this script.")
    parser.add_argument("mame_dir", nargs="?",
                        help="root of a MAME git repository "
                             "(default: mame_dir from local.toml)")
    parser.add_argument("--rev", metavar="TAG_OR_HASH",
                        help="tag or commit to read from "
                             "(default: mame_rev from local.toml)")
    args = parser.parse_args(argv)

    repo, rev = resolve_settings(args.mame_dir, args.rev)
    c = run_import(repo, rev, TARGET_DIR, MACHINES_FILE, MANIFEST_FILE)
    print(f"MAME {rev} ({c['commit'][:12]})")
    print(f"{c['listed']} machines listed in {MACHINES_FILE.name}"
          + (f", plus {len(c['parents'])} parents they need: "
             + ", ".join(c["parents"]) if c["parents"] else ""))
    print(f"{c['driver_files']} driver files; {c['mame_files']} files under src/mame "
          f"including {c['layouts']} layouts")
    print(f"{c['flags']} device flags on; {c['device_files']} files under src/devices, "
          f"{c['format_files']} under src/lib/formats, {c['netlist_files']} under src/lib/netlist")
    print(f"{c['imported']} files imported into {TARGET_DIR}, "
          f"{c['deleted']} stale files deleted")
    return 0


if __name__ == "__main__":
    sys.exit(main())
