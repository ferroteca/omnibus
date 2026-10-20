# Omnibus Emulator

## What is Omnibus?

Omnibus Emulator is a fork of [MAME](https://www.mamedev.org/) that builds a small set of vintage computer systems. The current set is listed in `src/mame/omnibus.flt`.

Omnibus keeps the full MAME source tree and history. The repository tracks `mamedev/mame` as its upstream, so MAME changes can be merged in with `git merge`. The default build target is `omnibus`, which produces an `omnibus` binary instead of `mame`.

MAME is a multi-purpose emulation framework. Its purpose is to preserve decades of software history by documenting the hardware and how it functions. The MAME source code serves as that documentation. Omnibus exists to carry the same work for a focused list of machines.

## Where can I find out more?

* [Omnibus on GitHub](https://github.com/ferroteca/omnibus)
* [Changes from MAME](CHANGES.md)
* [Official MAME Development Team Site](https://www.mamedev.org/) (binary downloads, wiki, forums, and more)
* [MAME documentation](https://docs.mamedev.org/)

## Development

### How to compile?

On a UNIX-like system (including Linux and macOS), type

```
make
```

to build the `omnibus` binary with the systems listed in `src/mame/omnibus.flt`.

See the [Compiling MAME](http://docs.mamedev.org/initialsetup/compilingmame.html) page on the MAME documentation site for prerequisites for macOS and popular Linux distributions.

For recent versions of macOS you need to install [Xcode](https://developer.apple.com/xcode/) including command-line tools and [SDL 2.0](https://github.com/libsdl-org/SDL/releases/latest).

On Windows, the project builds in an [MSYS2](https://www.msys2.org/) UCRT64 shell. MAME's ready-made [build environment](http://www.mamedev.org/tools/) based on MinGW-w64 also works.

Visual Studio builds are also possible, but you still need the [build environment](http://www.mamedev.org/tools/) based on MinGW-w64.
To generate solution and project files, run:

```
make vs2022
```

or use this command to build directly with msbuild:

```
make vs2022 MSBUILD=1
```

### Adding systems

Each line in `src/mame/omnibus.flt` names a driver source file under `src/mame`. Add a line for each driver you want in the `omnibus` build and run `make` again.

### Coding standard

Omnibus follows the MAME coding standard. Source code should be viewed and edited with your editor set to use four spaces per tab. Tabs are used for initial indentation of lines, with one tab used per indentation level. Spaces are used for other alignment within a line.

Some parts of the code follow [Allman style](https://en.wikipedia.org/wiki/Indent_style#Allman_style); some parts of the code follow [K&R style](https://en.wikipedia.org/wiki/Indent_style#K.26R_style), depending on who wrote the original version. **Above all else, be consistent with what you modify, and keep whitespace changes to a minimum when modifying existing source.** For new code, prefer Allman style.

New files need a standard header with license info. Code is made available under one of the following licenses: the [BSD-3-Clause](http://opensource.org/licenses/BSD-3-Clause) license, the [LGPL-2.1](http://opensource.org/licenses/LGPL-2.1), or the [GPL-2.0](http://opensource.org/licenses/GPL-2.0).

See the [C++ Coding Guidelines](https://docs.mamedev.org/contributing/cxx.html) on the MAME documentation site.

## License

Omnibus Emulator, like the MAME project it is based on, is made available under the terms of the
[GNU General Public License, version 2](http://opensource.org/licenses/GPL-2.0)
or later (GPL-2.0+), since it contains code made available under multiple
GPL-compatible licenses.  A great majority of the source files (over 90%
including core files) are made available under the terms of the
[3-clause BSD License](http://opensource.org/licenses/BSD-3-Clause), and we
would encourage new contributors to make their contributions available under the
terms of this license.

Please note that MAME is a registered trademark of Gregory Ember, and permission
is required to use the "MAME" name, logo, or wordmark. Omnibus Emulator is not
affiliated with or endorsed by the MAME project.

<a href="http://opensource.org/licenses/GPL-2.0" target="_blank">
<img align="right" width="100" src="https://opensource.org/wp-content/uploads/2009/06/OSIApproved.svg">
</a>

    Copyright (c) 1997-2026  MAMEdev and contributors

    This program is free software; you can redistribute it and/or modify it
    under the terms of the GNU General Public License version 2, as provided in
    docs/legal/GPL-2.0.

    This program is distributed in the hope that it will be useful, but WITHOUT
    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
    FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
    more details.

Please see [COPYING](COPYING) for more details.
