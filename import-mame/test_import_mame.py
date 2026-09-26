"""Tests for import_mame.py.

The parser and selection tests work on strings and small dicts. The
import tests build a temporary git repository shaped like MAME and run
the whole import into a temporary target directory. Run with:

    python -m pytest import-mame
"""

import subprocess
from pathlib import Path

import pytest

import import_mame


def comp(name, parent="0", flags="MACHINE_SUPPORTS_SAVE"):
    return (f'COMP( 1979, {name}, {parent}, 0, {name}, {name}, {name}_state, '
            f'empty_init, "Maker", "{name}", {flags} )\n')


def cons(name, parent="0"):
    return (f'CONS( 1979, {name}, {parent}, 0, {name}, {name}, {name}_state, '
            f'empty_init, "Maker", "{name}", 0 )\n')


def game(name, parent="0"):
    return (f'GAME( 1980, {name}, {parent}, {name}, {name}, {name}_state, '
            f'empty_init, ROT0, "Maker", "{name}", 0 )\n')


def machines_in(text):
    machines, _ = import_mame.parse_source(text, "test.cpp")
    return [(m.macro, m.name, m.parent) for m in machines]


# --- parsing ---------------------------------------------------------------

def test_every_machine_macro_is_parsed_with_its_parent():
    text = (comp("h89") + comp("h88", "h89") + cons("nes") + game("pacman")
            + 'GAMEL( 1980, pacman2, pacman, pacman, pacman, pacman_state, empty_init, ROT90, "Namco", "Pac-Man 2", 0, layout_x )\n'
            + 'SYST( 1985, thing, 0, 0, thing, thing, thing_state, empty_init, "Someone", "Thing", 0 )\n')
    assert machines_in(text) == [
        ("COMP", "h89", ""), ("COMP", "h88", "h89"), ("CONS", "nes", ""),
        ("GAME", "pacman", ""), ("GAMEL", "pacman2", "pacman"), ("SYST", "thing", ""),
    ]


def test_multi_line_call_and_space_before_parenthesis():
    text = ('COMP( 1987, aa440,\n'
            '      aa310, 0, aa440, 0, aa310_state, init_hd,\n'
            '      "Acorn Computers",\n'
            '      "Archimedes 440", MACHINE_NOT_WORKING )\n'
            'COMP ( 1977, h8, 0, 0, h8, h8, h8_state, empty_init, "Heath Company", "H8", 0 )\n')
    assert machines_in(text) == [("COMP", "aa440", "aa310"), ("COMP", "h8", "")]


def test_macro_definitions_that_wrap_a_machine_macro_are_not_machines():
    text = ('#define GAME_CUSTOM(year, setname, parent, name) \\\n'
            '    GAME( year, setname, parent, mod4yam, mpu4, mpu4mod4yam_state, \\\n'
            '          init_m4, ROT0, "Barcrest", name, 0 )\n'
            '#define ONE_LINER(setname) COMP( 1979, setname, 0, 0, x, x, x_state, empty_init, "A", "B", 0 )\n'
            'GAME_CUSTOM( 1985, m4real, 0, "Real machine" )\n'
            + comp("h89"))
    assert machines_in(text) == [("COMP", "h89", "")]


def test_commented_out_and_if_0_calls_are_skipped():
    text = (comp("atom")
            + "//" + comp("prophet3", "atom")
            + "/*\n" + comp("prophet2", "atom") + "*/\n"
            + "#if 0\n" + comp("dead") + "#else\n" + comp("live") + "#endif\n")
    assert [m[1] for m in machines_in(text)] == ["atom", "live"]


def test_string_arguments_with_escapes_and_prefixes_do_not_confuse_the_parser():
    text = ('COMP( 1989, kccomp, cpc464, 0, kccomp, kccomp, amstrad_state, empty_init, '
            'u8"VEB Mikroelektronik \\"Wilhelm Pieck\\" Mühlhausen",\n'
            '      "KC compact", MACHINE_SUPPORTS_SAVE )\n'
            'COMP( 1990, joined, 0, 0, x, x, x_state, empty_init, '
            '"Acorn " "Computers", "Two, with comma", 0 )\n')
    assert [m[1] for m in machines_in(text)] == ["kccomp", "joined"]


def test_includes_are_collected_in_order_even_inside_if_0():
    text = ('#include "emu.h"\n'
            '#include "h89.h"\n'
            '#include "bus/rs232/rs232.h"\n'
            '// #include "notthis.h"\n'
            '#include <string>\n'
            '#if 0\n'
            '#include "maybe.h"\n'
            '#endif\n'
            '#include "h89.lh"\n')
    _, includes = import_mame.parse_source(text, "test.cpp")
    assert [inc for inc, _ in includes] == ["emu.h", "h89.h", "bus/rs232/rs232.h", "maybe.h", "h89.lh"]
    assert all(guards == frozenset() for _, guards in includes)


def test_includes_behind_has_formats_guards_carry_the_flag():
    text = ('#include "all.h"\n'
            '#ifdef HAS_FORMATS_H89_DSK\n'
            '#include "h89_dsk.h"\n'
            '#ifdef SOMETHING_ELSE\n'
            '#include "h89_extra.h"\n'
            '#endif\n'
            '#else\n'
            '#include "fallback.h"\n'
            '#endif\n'
            '#ifdef HAS_FORMATS_PC_DSK\n'
            '#include "pc_dsk.h"\n'
            '#endif\n')
    assert import_mame.find_includes(text) == [
        ("all.h", frozenset()),
        ("h89_dsk.h", frozenset({("FORMATS", "H89_DSK")})),
        ("h89_extra.h", frozenset({("FORMATS", "H89_DSK")})),
        ("fallback.h", frozenset()),
        ("pc_dsk.h", frozenset({("FORMATS", "PC_DSK")})),
    ]


# --- selecting machines and following includes -----------------------------

def machine_table(*rows):
    return {name: import_mame.Machine(name, parent, macro, source)
            for macro, name, parent, source in rows}


def unguarded(includes):
    """Turn {path: [include]} into the {path: [(include, guards)]} form."""
    return {path: [(inc, frozenset()) for inc in incs] for path, incs in includes.items()}


def test_machine_list_skips_comments_and_blank_lines(tmp_path):
    path = tmp_path / "machines.lst"
    path.write_text("# heath\n\nh8\nh89  # the main one\n  z90\n")
    assert import_mame.read_machine_list(path) == ["h8", "h89", "z90"]
    path.write_text("# nothing here\n")
    with pytest.raises(SystemExit, match="names no machines"):
        import_mame.read_machine_list(path)
    with pytest.raises(SystemExit, match="does not exist"):
        import_mame.read_machine_list(tmp_path / "missing.lst")


def test_listed_machines_are_selected_with_the_parents_they_need():
    machines = machine_table(
        ("COMP", "h89", "0", "src/mame/heathzenith/h89.cpp"),
        ("COMP", "h88", "h89", "src/mame/heathzenith/h89.cpp"),
        ("COMP", "intvkbd", "intv", "src/mame/mattel/intv.cpp"),
        ("CONS", "intv", "0", "src/mame/mattel/intv.cpp"),
        ("GAME", "pacman", "0", "src/mame/namco/pacman.cpp"),
    )
    selected, parents = import_mame.select_machines(machines, ["h88", "intvkbd"])
    assert selected == {"h88", "h89", "intvkbd", "intv"}
    assert parents == {"h89", "intv"}
    selected, parents = import_mame.select_machines(machines, ["h89", "h88"])
    assert selected == {"h88", "h89"} and parents == set()


def test_a_listed_machine_that_no_driver_defines_stops_the_run():
    machines = machine_table(("COMP", "h89", "0", "src/mame/heathzenith/h89.cpp"))
    with pytest.raises(SystemExit, match="not defined by any driver at this revision: h17, z100"):
        import_mame.select_machines(machines, ["h89", "h17", "z100"])


def test_closure_follows_includes_and_companions_through_scanned_dirs():
    tree = {
        "src/mame/heathzenith/h89.cpp",
        "src/mame/heathzenith/h89.h",
        "src/mame/heathzenith/tlb.h",
        "src/mame/heathzenith/tlb.cpp",
        "src/mame/heathzenith/tlb_v.cpp",
        "src/mame/heathzenith/unrelated.cpp",
        "src/mame/shared/sharedthing.h",
        "src/mame/shared/sharedthing.cpp",
        "src/mame/shared/deeper.h",
        "src/devices/bus/rs232/rs232.h",
        "src/devices/bus/rs232/rs232.cpp",
        "src/devices/bus/rs232/rs232_v.cpp",
        "src/devices/cpu/z80/z80.h",
        "src/devices/cpu/z80/z80.cpp",
        "src/lib/formats/h89_dsk.h",
        "src/lib/formats/h89_dsk.cpp",
        "src/emu/emu.h",
    }
    includes = unguarded({
        "src/mame/heathzenith/h89.cpp": ["emu.h", "h89.h", "bus/rs232/rs232.h", "h89.lh"],
        "src/mame/heathzenith/h89.h": ["tlb.h", "sharedthing.h"],
        "src/mame/heathzenith/tlb.cpp": ["tlb.h", "tlb.lh"],
        "src/mame/shared/sharedthing.cpp": ["deeper.h"],
        "src/devices/bus/rs232/rs232.cpp": ["cpu/z80/z80.h", "formats/h89_dsk.h"],
    })
    closure, layouts = set(), set()
    import_mame.extend_closure(closure, layouts, {"src/mame/heathzenith/h89.cpp"}, includes, tree)
    assert closure == tree - {
        "src/mame/heathzenith/unrelated.cpp",
        "src/devices/bus/rs232/rs232_v.cpp",  # _v companions are a src/mame rule
        "src/emu/emu.h",                      # outside the scanned directories
    }
    assert layouts == {"h89", "tlb"}


# --- the device lua files --------------------------------------------------

CPU_LUA = '''\
-- license:BSD-3-Clause
DRC_CPUS = { "MIPS3", "SH" }
CPU_INCLUDE_DRC = false
for i, v in ipairs(DRC_CPUS) do
	if (CPUS[v]~=null) then
		CPU_INCLUDE_DRC = true
		break
	end
end
CPU_INCLUDE_DRC_NATIVE = CPU_INCLUDE_DRC and (not _OPTIONS["FORCE_DRC_C_BACKEND"])

if CPU_INCLUDE_DRC then
	files {
		MAME_DIR .. "src/devices/cpu/drcbec.cpp",
	}
end

--@src/devices/cpu/z80/z80.h,CPUS["Z80"] = true
--@src/devices/cpu/z80/kc82.h,CPUS["KC80"] = true

if CPUS["Z80"] or CPUS["KC80"] then
	files {
		MAME_DIR .. "src/devices/cpu/z80/z80.cpp",
		MAME_DIR .. "src/devices/cpu/z80/z80.h",
		MAME_DIR .. "src/devices/cpu/z80/kc82.cpp",
	}
end

local want_disasm_z80 = opt_tool(CPUS, "Z80")
if want_disasm_z80 then
	table.insert(disasm_files, MAME_DIR .. "src/devices/cpu/z80/z80dasm.cpp")
end

--@src/devices/cpu/m6502/m6502.h,CPUS["M6502"] = true

if (CPUS["M6502"]~=null) then
	files {
		MAME_DIR .. "src/devices/cpu/m6502/m6502.cpp",
		MAME_DIR .. "src/devices/cpu/m6502/m6502.h",
	}
	custombuildtask {
		{ MAME_DIR .. "src/devices/cpu/m6502/om6502.lst", GEN_DIR .. "emu/cpu/m6502/m6502.hxx", { MAME_DIR .. "src/devices/cpu/m6502/m6502make.py" }, {"@echo ..." } },
	}
end
'''

BUS_LUA = '''\
--@src/devices/bus/nscsi/cd.h,BUSES["NSCSI"] = true
if (BUSES["NSCSI"]~=null) then
	MACHINES["NSCSI"] = true
	files {
		MAME_DIR .. "src/devices/bus/nscsi/cd.cpp",
	}
end
'''

MACHINE_LUA = '''\
--@src/devices/machine/nscsi_bus.h,MACHINES["NSCSI"] = true
if (MACHINES["NSCSI"]~=null) then
	files {
		MAME_DIR .. "src/devices/machine/nscsi_bus.cpp",
	}
end
'''


def test_directives_map_headers_to_flags():
    assert import_mame.parse_directives(CPU_LUA) == [
        ("src/devices/cpu/z80/z80.h", ("CPUS", "Z80")),
        ("src/devices/cpu/z80/kc82.h", ("CPUS", "KC80")),
        ("src/devices/cpu/m6502/m6502.h", ("CPUS", "M6502")),
    ]


def test_lua_blocks_are_evaluated_with_the_given_flags():
    paths, new_flags = import_mame.lua_required_files(CPU_LUA, {("CPUS", "Z80")}, "cpu.lua")
    assert paths == {
        "src/devices/cpu/z80/z80.cpp", "src/devices/cpu/z80/z80.h",
        "src/devices/cpu/z80/kc82.cpp", "src/devices/cpu/z80/z80dasm.cpp",
    }
    assert new_flags == set()
    paths, _ = import_mame.lua_required_files(CPU_LUA, {("CPUS", "M6502"), ("CPUS", "SH")}, "cpu.lua")
    assert paths == {
        "src/devices/cpu/drcbec.cpp",
        "src/devices/cpu/m6502/m6502.cpp", "src/devices/cpu/m6502/m6502.h",
        "src/devices/cpu/m6502/om6502.lst", "src/devices/cpu/m6502/m6502make.py",
    }


def test_flags_a_block_turns_on_reach_the_other_files():
    lua = {"bus.lua": BUS_LUA, "machine.lua": MACHINE_LUA}
    paths, flags = import_mame.evaluate_device_lua(lua, {("BUSES", "NSCSI")})
    assert flags == {("BUSES", "NSCSI"), ("MACHINES", "NSCSI")}
    assert paths == {"src/devices/bus/nscsi/cd.cpp", "src/devices/machine/nscsi_bus.cpp"}


def test_unknown_lua_condition_stops_instead_of_guessing():
    with pytest.raises(SystemExit, match="cannot evaluate"):
        import_mame.lua_required_files('if mystery_flag then\nend\n', set(), "x.lua")


def test_required_files_iterate_from_drivers_through_flags_to_includes():
    tree = {
        "src/mame/heathzenith/h89.cpp", "src/devices/cpu/z80/z80.h",
        "src/devices/cpu/z80/z80.cpp", "src/devices/cpu/z80/kc82.cpp",
        "src/devices/cpu/z80/kc82.h", "src/devices/cpu/z80/z80dasm.cpp",
        "src/devices/bus/nscsi/cd.h", "src/devices/bus/nscsi/cd.cpp",
        "src/devices/machine/nscsi_bus.h", "src/devices/machine/nscsi_bus.cpp",
        "src/devices/cpu/m6502/m6502.h", "src/devices/cpu/m6502/m6502.cpp",
    }
    includes = unguarded({
        "src/mame/heathzenith/h89.cpp": ["cpu/z80/z80.h"],
        # kc82.cpp is compiled only because it sits in the Z80 block; its
        # include must still be followed.
        "src/devices/cpu/z80/kc82.cpp": ["kc82.h", "bus/nscsi/cd.h"],
    })
    lua = {"cpu.lua": CPU_LUA, "bus.lua": BUS_LUA, "machine.lua": MACHINE_LUA}
    closure, layouts, flags, lua_paths = import_mame.required_files(
        {"src/mame/heathzenith/h89.cpp"}, includes, tree, lua)
    assert flags == {("CPUS", "Z80"), ("CPUS", "KC80"), ("BUSES", "NSCSI"), ("MACHINES", "NSCSI")}
    assert "src/devices/cpu/z80/kc82.h" in closure
    assert "src/devices/bus/nscsi/cd.cpp" in closure
    assert "src/devices/machine/nscsi_bus.cpp" in lua_paths
    assert not any("m6502" in p for p in closure | lua_paths)


def test_choose_files_prunes_only_the_pruned_directories():
    tree = [
        "makefile", "src/emu/emu.h", "src/lib/util/util.cpp", "src/tools/chdman.cpp",
        "src/mame/mame.cpp", "src/mame/mame.lst", "src/mame/ci.flt", "src/mame/tiny.lst",
        "src/mame/heathzenith/h89.cpp", "src/mame/namco/pacman.cpp",
        "src/mame/layout/h89.lay", "src/mame/layout/pacman.lay",
        "src/mame/shared/used.cpp", "src/mame/shared/unused.cpp",
        "src/devices/cpu/z80/z80.cpp", "src/devices/cpu/z80/z80dasm.cpp", "src/devices/cpu/m6502/m6502.cpp",
        "src/lib/formats/all.cpp", "src/lib/formats/x_dsk.cpp",
        "src/lib/netlist/nl_base.cpp",
    ]
    closure = {"src/mame/heathzenith/h89.cpp", "src/mame/shared/used.cpp", "src/devices/cpu/z80/z80.cpp"}
    lua_paths = {"src/devices/cpu/z80/z80dasm.cpp", "src/lib/formats/all.cpp"}
    assert import_mame.choose_files(tree, closure, lua_paths, {"h89"}, netlist=False) == [
        "makefile", "src/devices/cpu/z80/z80.cpp", "src/devices/cpu/z80/z80dasm.cpp",
        "src/emu/emu.h", "src/lib/formats/all.cpp", "src/lib/util/util.cpp",
        "src/mame/heathzenith/h89.cpp", "src/mame/layout/h89.lay",
        "src/mame/mame.cpp", "src/mame/shared/used.cpp", "src/tools/chdman.cpp",
    ]
    assert "src/lib/netlist/nl_base.cpp" in import_mame.choose_files(
        tree, closure, lua_paths, set(), netlist=True)


def test_directive_lines_outside_the_closure_are_removed():
    text = import_mame.filter_directives(CPU_LUA, {"src/devices/cpu/z80/z80.h"})
    assert '--@src/devices/cpu/z80/z80.h,CPUS["Z80"] = true\n' in text
    assert "kc82.h" not in text.split("if CPUS")[0]
    assert "--@src/devices/cpu/m6502" not in text
    assert 'if (CPUS["M6502"]~=null) then' in text


MAME_LUA_LINK = (
    'function linkProjects_mame_mame(_target, _subtarget)\n'
    '\ttable.sort(projects)\n'
    '\ttable.insert(projects, "shared") -- must stay at the end\n'
    '\tlinks(projects)\n'
    'end\n')


def test_mame_lua_links_shared_only_when_the_directory_exists():
    text = import_mame.link_shared_only_if_present(MAME_LUA_LINK.replace("\n", "\r\n"))
    assert text == (
        'function linkProjects_mame_mame(_target, _subtarget)\r\n'
        '\ttable.sort(projects)\r\n'
        '\tif os.isdir(path.join(MAME_DIR, "src", _target, "shared")) then\r\n'
        '\t\ttable.insert(projects, "shared") -- must stay at the end\r\n'
        '\tend\r\n'
        '\tlinks(projects)\r\n'
        'end\r\n')


def test_mame_lua_without_the_shared_link_line_stops_the_run():
    with pytest.raises(SystemExit):
        import_mame.link_shared_only_if_present("links(projects)\n")


def test_mame_lst_groups_names_by_source():
    machines = machine_table(
        ("COMP", "h89", "0", "src/mame/heathzenith/h89.cpp"),
        ("COMP", "h88", "h89", "src/mame/heathzenith/h89.cpp"),
        ("COMP", "aa310", "0", "src/mame/acorn/aa310.cpp"),
        ("GAME", "pacman", "0", "src/mame/namco/pacman.cpp"),
    )
    text = import_mame.mame_lst_text(machines, {"h89", "h88", "aa310"}, "mame0289", "f34f02505e32abcd")
    assert text.endswith(
        "\n@source:acorn/aa310.cpp\naa310\n"
        "\n@source:heathzenith/h89.cpp\nh88\nh89\n")
    assert "mame0289 (f34f02505e32)" in text
    assert "pacman" not in text


# --- the whole import against a git repository -----------------------------

def run_git(repo, *args):
    return subprocess.run(["git", "-C", str(repo), *args], check=True,
                          capture_output=True, text=True)


def write(root, rel, text):
    path = root / rel
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text(text, encoding="utf-8")


FORMATS_LUA = '''\
function formatsProject(_target, _subtarget)
project "formats"
	files {
		MAME_DIR .. "src/lib/formats/all.cpp",
	}

--@src/lib/formats/h89_dsk.h,FORMATS["H89_DSK"] = true
if opt_tool(FORMATS, "H89_DSK") then
	files {
		MAME_DIR .. "src/lib/formats/h89_dsk.cpp",
		MAME_DIR .. "src/lib/formats/h89_dsk.h",
	}
end

--@src/lib/formats/pc_dsk.h,FORMATS["PC_DSK"] = true
if opt_tool(FORMATS, "PC_DSK") then
	files {
		MAME_DIR .. "src/lib/formats/pc_dsk.cpp",
		MAME_DIR .. "src/lib/formats/pc_dsk.h",
	}
end
end
'''


ALL_CPP = '''\
#include "all.h"
#include "has_formats.h" // Generated by genie
#ifdef HAS_FORMATS_H89_DSK
#include "h89_dsk.h"
#include "h89_extra.h"
#endif
#ifdef HAS_FORMATS_PC_DSK
#include "pc_dsk.h"
#endif
'''


def test_guarded_includes_are_followed_once_their_flag_is_on():
    tree = {
        "src/mame/heathzenith/h89.cpp",
        "src/lib/formats/all.cpp", "src/lib/formats/all.h",
        "src/lib/formats/h89_dsk.h", "src/lib/formats/h89_dsk.cpp",
        "src/lib/formats/h89_extra.h", "src/lib/formats/h89_extra.cpp",
        "src/lib/formats/pc_dsk.h", "src/lib/formats/pc_dsk.cpp",
    }
    includes = unguarded({"src/mame/heathzenith/h89.cpp": ["formats/h89_dsk.h"]})
    includes["src/lib/formats/all.cpp"] = import_mame.find_includes(ALL_CPP)
    closure, _, flags, lua_paths = import_mame.required_files(
        {"src/mame/heathzenith/h89.cpp"}, includes, tree, {"formats.lua": FORMATS_LUA})
    assert flags == {("FORMATS", "H89_DSK")}
    # all.cpp comes from the unguarded part of formats.lua; its guarded
    # include of h89_extra.h is followed because H89_DSK is on.
    assert "src/lib/formats/h89_extra.cpp" in closure
    assert not any("pc_dsk" in p for p in closure | lua_paths)


@pytest.fixture
def mame_repo(tmp_path):
    """A git repository shaped like MAME with two tagged commits.

    Tag v1 has drivers h89 (Z80, an rs232 bus device, a shared file, a
    layout and a disk format), intvkbd (a computer whose parent intv is
    a console, on a 6502) and pacman (an arcade game that uses the
    netlist library). Tag v2 drops the intv driver file, so the 6502
    is no longer needed. The working tree also has an uncommitted
    driver that must never be imported.
    """
    repo = tmp_path / "mame"
    repo.mkdir()
    run_git(tmp_path, "init", "-q", "-b", "master", str(repo))
    run_git(repo, "config", "user.email", "test@example.com")
    run_git(repo, "config", "user.name", "Test")
    write(repo, ".gitignore", "build/\n")
    write(repo, "makefile", "all:\n")
    write(repo, "src/emu/emu.h", "// emu\n")
    write(repo, "src/lib/util/util.cpp", "// util\n")
    write(repo, "src/tools/chdman.cpp", "// tool\n")
    write(repo, "scripts/src/cpu.lua", CPU_LUA)
    write(repo, "scripts/src/bus.lua", BUS_LUA)
    write(repo, "scripts/src/machine.lua", MACHINE_LUA)
    write(repo, "scripts/src/formats.lua", FORMATS_LUA)
    write(repo, "scripts/src/sound.lua", "-- nothing\n")
    write(repo, "scripts/src/video.lua", "-- nothing\n")
    for name in ("z80.h", "z80.cpp", "z80dasm.cpp", "kc82.h", "kc82.cpp"):
        write(repo, f"src/devices/cpu/z80/{name}", "// z80\n")
    for name in ("m6502.h", "m6502.cpp", "om6502.lst", "m6502make.py"):
        write(repo, f"src/devices/cpu/m6502/{name}", "// 6502\n")
    write(repo, "src/devices/cpu/drcbec.cpp", "// drc\n")
    write(repo, "src/devices/bus/rs232/rs232.h", "// rs232\n")
    write(repo, "src/devices/bus/rs232/rs232.cpp", '#include "rs232.h"\n#include "machine/panel.h"\n')
    write(repo, "src/devices/machine/panel.h", "// panel\n")
    write(repo, "src/devices/machine/panel.cpp", '#include "panel.h"\n#include "panel.lh"\n')
    write(repo, "src/devices/machine/orphan.h", "// nobody includes this\n")
    write(repo, "src/devices/bus/nscsi/cd.h", "// cd\n")
    write(repo, "src/devices/bus/nscsi/cd.cpp", "// cd\n")
    write(repo, "src/devices/machine/nscsi_bus.h", "// nscsi\n")
    write(repo, "src/devices/machine/nscsi_bus.cpp", "// nscsi\n")
    write(repo, "src/lib/formats/all.cpp", ALL_CPP)
    for name in ("all.h", "h89_dsk.h", "h89_dsk.cpp", "h89_extra.h", "h89_extra.cpp",
                 "pc_dsk.h", "pc_dsk.cpp"):
        write(repo, f"src/lib/formats/{name}", "// format\n")
    write(repo, "src/lib/netlist/nl_base.cpp", "// netlist\n")
    write(repo, "src/frontend/mame/ui/filesel.cpp", '#include "emu.h"\n#include "bus/midi/midiin.h"\n')
    write(repo, "src/devices/bus/midi/midiin.h", "// midi in\n")
    write(repo, "src/devices/bus/midi/midiin.cpp", '#include "midiin.h"\n')
    write(repo, "src/emu/drivers/testcpu.cpp", '#include "cpu/ppc/ppc.h"\n')
    write(repo, "src/devices/cpu/ppc/ppc.h", "// ppc\n")
    write(repo, "src/devices/cpu/ppc/ppc.cpp", "// ppc\n")
    write(repo, "src/mame/mame.cpp", "// main\n")
    write(repo, "src/mame/mame.lst", "@source:namco/pacman.cpp\npacman\n")
    write(repo, "src/mame/ci.flt", "namco/pacman.cpp\n")
    write(repo, "src/mame/heathzenith/h89.cpp",
          '#include "emu.h"\n#include "h89.h"\n#include "bus/rs232/rs232.h"\n'
          '#include "formats/h89_dsk.h"\n#include "h89.lh"\n' + comp("h89") + comp("h88", "h89"))
    write(repo, "src/mame/heathzenith/h89.h", '#include "sharedthing.h"\n#include "cpu/z80/z80.h"\n')
    write(repo, "src/mame/heathzenith/h19.cpp",
          '#include "h19.lh"\n' + comp("h19", flags="MACHINE_NOT_WORKING | MACHINE_SUPPORTS_SAVE"))
    write(repo, "src/mame/shared/sharedthing.h", "// shared\n")
    write(repo, "src/mame/shared/sharedthing.cpp", '#include "sharedthing.h"\n')
    write(repo, "src/mame/shared/unused.cpp", "// nobody includes this\n")
    write(repo, "src/mame/mattel/intv.cpp",
          '#include "cpu/m6502/m6502.h"\n' + cons("intv") + comp("intvkbd", "intv"))
    write(repo, "src/mame/namco/pacman.cpp", '#include "machine/netlist.h"\n' + game("pacman"))
    for name in ("h89", "h19", "pacman", "panel"):
        write(repo, f"src/mame/layout/{name}.lay", f"<mamelayout>{name}</mamelayout>\n")
    run_git(repo, "add", ".")
    run_git(repo, "commit", "-q", "-m", "v1")
    run_git(repo, "tag", "v1")
    run_git(repo, "rm", "-q", "src/mame/mattel/intv.cpp")
    run_git(repo, "commit", "-q", "-m", "v2")
    run_git(repo, "tag", "v2")
    write(repo, "src/mame/misc/uncommitted.cpp", comp("uncommitted"))
    return repo


def do_import(repo, rev, target, names=("h89", "h88", "intvkbd")):
    """Import the named machines into target.

    target becomes a git repository on first use, and machines.lst is
    written with the names before every run.
    """
    own = target / "import-mame"
    own.mkdir(parents=True, exist_ok=True)
    if not (target / ".git").exists():
        run_git(target.parent, "init", "-q", str(target))
    (own / "machines.lst").write_text("".join(n + "\n" for n in names))
    return import_mame.run_import(repo, rev, target, own / "machines.lst", own / "source.lst")


def files_under(target):
    return {p.relative_to(target).as_posix() for p in target.rglob("*")
            if p.is_file() and ".git" not in p.parts}


def test_import_copies_the_needed_files_and_writes_the_lists(mame_repo, tmp_path):
    target = tmp_path / "target"
    counts = do_import(mame_repo, "v1", target)

    assert files_under(target) == {
        ".gitignore", "makefile", "src/emu/emu.h", "src/lib/util/util.cpp", "src/tools/chdman.cpp",
        # testcpu.cpp is copied with the rest of src/emu, but the build
        # does not compile it, so its PowerPC include is not followed.
        "src/emu/drivers/testcpu.cpp",
        # The UI includes a device header whatever machines are built.
        "src/frontend/mame/ui/filesel.cpp",
        "src/devices/bus/midi/midiin.h", "src/devices/bus/midi/midiin.cpp",
        "scripts/src/cpu.lua", "scripts/src/bus.lua", "scripts/src/machine.lua",
        "scripts/src/formats.lua", "scripts/src/sound.lua", "scripts/src/video.lua",
        # Z80 from h89.h, plus the whole Z80 block and its disassembler.
        "src/devices/cpu/z80/z80.h", "src/devices/cpu/z80/z80.cpp",
        "src/devices/cpu/z80/z80dasm.cpp", "src/devices/cpu/z80/kc82.cpp",
        # 6502 from intv.cpp, plus the generator script and table.
        "src/devices/cpu/m6502/m6502.h", "src/devices/cpu/m6502/m6502.cpp",
        "src/devices/cpu/m6502/om6502.lst", "src/devices/cpu/m6502/m6502make.py",
        # Reached by includes only; no lua block lists them.
        "src/devices/bus/rs232/rs232.h", "src/devices/bus/rs232/rs232.cpp",
        "src/devices/machine/panel.h", "src/devices/machine/panel.cpp",
        # Formats: the unguarded part, the one h89 uses, and the extra
        # header all.cpp includes behind that format's guard.
        "src/lib/formats/all.cpp", "src/lib/formats/all.h",
        "src/lib/formats/h89_dsk.h", "src/lib/formats/h89_dsk.cpp",
        "src/lib/formats/h89_extra.h", "src/lib/formats/h89_extra.cpp",
        "src/mame/mame.cpp", "src/mame/mame.lst",
        "src/mame/heathzenith/h89.cpp", "src/mame/heathzenith/h89.h",
        # h19.cpp is not here: its only machine is marked not working.
        "src/mame/shared/sharedthing.h", "src/mame/shared/sharedthing.cpp",
        "src/mame/mattel/intv.cpp",
        "src/mame/layout/h89.lay", "src/mame/layout/panel.lay",
        "import-mame/machines.lst", "import-mame/source.lst",
    }
    assert (target / "src" / "mame" / "mame.lst").read_text(encoding="utf-8").endswith(
        "\n@source:heathzenith/h89.cpp\nh88\nh89\n"
        "\n@source:mattel/intv.cpp\nintv\nintvkbd\n")
    manifest = (target / "import-mame" / "source.lst").read_text().split()
    assert "src/mame/mame.lst" in manifest and "src/mame/namco/pacman.cpp" not in manifest
    assert counts["listed"] == 3 and counts["parents"] == ["intv"]
    assert counts["deleted"] == 0


def test_imported_lua_files_keep_only_the_directives_in_use(mame_repo, tmp_path):
    target = tmp_path / "target"
    do_import(mame_repo, "v1", target)
    cpu = (target / "scripts" / "src" / "cpu.lua").read_text()
    assert "--@src/devices/cpu/z80/z80.h," in cpu
    assert "--@src/devices/cpu/m6502/m6502.h," in cpu
    assert "--@src/devices/cpu/z80/kc82.h," not in cpu
    formats = (target / "scripts" / "src" / "formats.lua").read_text()
    assert "--@src/lib/formats/h89_dsk.h," in formats
    assert "--@src/lib/formats/pc_dsk.h," not in formats
    assert 'if opt_tool(FORMATS, "PC_DSK") then' in formats


def test_second_import_overwrites_and_deletes_tracked_files_not_imported(mame_repo, tmp_path):
    target = tmp_path / "target"
    do_import(mame_repo, "v1", target)
    run_git(target, "add", "-A")
    write(target, "import-mame/tracked_note.txt", "tracked but under import\n")
    write(target, "tracked_note.txt", "tracked, not imported next time\n")
    run_git(target, "add", "import-mame/tracked_note.txt", "tracked_note.txt")
    (target / "makefile").write_text("edited locally\n")
    write(target, "src/mame/heathzenith/h17.cpp", "// my own driver, untracked\n")
    write(target, "notes.txt", "untracked, not ignored\n")
    write(target, "build/out.o", "untracked, ignored by .gitignore\n")

    counts = do_import(mame_repo, "v2", target, names=("h89", "h88"))

    assert (target / "makefile").read_text() == "all:\n"
    assert not (target / "src" / "mame" / "mattel").exists()
    assert not (target / "src" / "devices" / "cpu" / "m6502").exists()
    assert not (target / "tracked_note.txt").exists()
    assert (target / "import-mame" / "tracked_note.txt").is_file()
    assert (target / "src" / "mame" / "heathzenith" / "h17.cpp").is_file()
    assert (target / "notes.txt").is_file()
    assert (target / "build" / "out.o").is_file()
    # intv.cpp, the four 6502 files and tracked_note.txt
    assert counts["deleted"] == 6
    assert "intv" not in (target / "src" / "mame" / "mame.lst").read_text().split()
    assert "--@src/devices/cpu/m6502" not in (target / "scripts" / "src" / "cpu.lua").read_text()


def test_target_that_is_not_a_repository_stops_before_touching_anything(mame_repo, tmp_path):
    target = tmp_path / "target"
    write(target, "precious.txt", "keep\n")
    own = target / "import-mame"
    own.mkdir()
    with pytest.raises(SystemExit, match="not a git repository"):
        import_mame.run_import(mame_repo, "v1", target, own / "machines.lst", own / "source.lst")
    assert (target / "precious.txt").is_file()
    assert not (target / "makefile").exists()


def test_import_reads_the_commit_not_the_working_tree(mame_repo, tmp_path):
    target = tmp_path / "target"
    do_import(mame_repo, "v2", target, names=("h89",))
    assert not (target / "src" / "mame" / "misc").exists()
    assert "uncommitted" not in (target / "import-mame" / "source.lst").read_text()
    with pytest.raises(SystemExit, match="not defined by any driver"):
        do_import(mame_repo, "v2", target, names=("uncommitted",))


def test_unknown_revision_stops_with_a_message(mame_repo, tmp_path):
    with pytest.raises(SystemExit, match="nope is not a tag or commit"):
        do_import(mame_repo, "nope", tmp_path / "target")


def test_directory_that_is_not_a_repository_stops_with_a_message(tmp_path):
    with pytest.raises(SystemExit, match="not a git repository"):
        import_mame.resolve_commit(tmp_path, "v1")


# --- config ----------------------------------------------------------------

def test_command_line_values_win_over_config(tmp_path):
    config = tmp_path / "local.toml"
    config.write_text('mame_dir = "D:/from-config"\nmame_rev = "mame0001"\n')
    assert import_mame.resolve_settings("D:/from-cli", "abc123", config) == \
        (Path("D:/from-cli"), "abc123")


def test_config_supplies_missing_values(tmp_path):
    config = tmp_path / "local.toml"
    config.write_text('mame_dir = "D:/from-config"\nmame_rev = "mame0001"\n')
    assert import_mame.resolve_settings(None, None, config) == \
        (Path("D:/from-config"), "mame0001")
    assert import_mame.resolve_settings(None, "abc123", config) == \
        (Path("D:/from-config"), "abc123")


def test_missing_repository_stops_with_a_message(tmp_path):
    config = tmp_path / "local.toml"
    with pytest.raises(SystemExit, match="mame_dir"):
        import_mame.resolve_settings(None, "mame0001", config)


def test_missing_revision_stops_with_a_message(tmp_path):
    config = tmp_path / "local.toml"
    config.write_text('mame_dir = "D:/from-config"\n')
    with pytest.raises(SystemExit, match="mame_rev"):
        import_mame.resolve_settings(None, None, config)
