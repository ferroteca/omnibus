## Changes from MAME

Omnibus is based on MAME 0.289. This file lists the changes Omnibus makes to MAME. Update it when a change is added, or when MAME merges one of these changes.

### Heath/Zenith

* H8: add the H-8-17 hard-sectored floppy controller. A setting models the HA-8-8 ROM disable modification. The H-8-17 and the H89 H-88-1 controller now share code in `h17_fdc_base`.
* H8: gate the H-8-5 console interrupt with /DTR.
* H8: the H-8-5 SER RX and SER TX jumpers set the console's receive and transmit rates, 110 to 9600 baud. Both default to 9600, which matches the H19's default. Before, the console ran at a fixed 600 baud, and the H19 showed nothing until its own switch was set to 600.
* H8/H88: the cassette interfaces run at 1200 baud, the rate Heath recorded its software tapes at. The H-88-5 has a fixed 1200 baud. The H-8-5's Tape RX and Tape TX jumpers pick 300 or 1200 baud, and both default to 1200. H8T images now play as 1200 baud audio. Before, both cards and the H8T format ran at 300 baud, so a load took four times as long as on the real machines.
* H8: add the H-8-4 four port serial card: four INS8250s, each with an RS-232 port, address jumpers (320, 330, 340 or 350 octal) and an interrupt jumper. Port 4 ships switched off, because HDOS moves its console to an 8250 it finds at 350 octal.
* H8: add the WH-8-37 soft-sectored floppy controller, with its port base and Z-37 jumpers. The Z-67 half of the card is not modelled. The WH-8-37 and the H89 Z-89-37 now share code in `z37_fdc_base`. The HA-8-6 gains an `h37` interrupt socket option for the WH-8-37 cable in U38, and maps the H-17 half of XCON8 at 1800-1FFF with PAM-37, which calls into it.
* H8: the machines start from the Heath catalog packages. `h8` is the H8 kit with one H-8-1 holding 4K, the least memory it runs with, and no I/O card or terminal. Before, it had a WH-8-64, an HA-8-8 and an H19. `h8_system2` (H8 System Two (24K)) is the System Two package with a third H-8-1: three H-8-1 boards with H-8-3 chip sets give 24K at 2000-7FFF, and an H9 is on the H-8-5 at 600 baud. `h8_hks85` (HKS-85) has 48K from 0000 on three WH-8-16 boards, an H-8-17 with two drives, an H19 on H-8-4 port 4, an HA-8-8 with XCON8 set to boot the disk, and the HA-8-8 ROM disable modification. It boots CP/M 2.2.04 and HDOS 1.0 to 2.0. `h8_wh837` is not a catalog package: an HA-8-6 Z80 CPU with PAM-37, 64K on a WH-8-64, a WH-8-37 with two drives, and an H19 on H-8-4 port 4. On every machine except `h8`, the terminal keyboard is enabled by default and the front panel keyboard is disabled.
* H8 and ET-3400: the system names drop the "Heathkit" prefix, because the manufacturer field already says Heath Company. The h8 is "H-8", like the H-19, H-88, H-89 and Z-90. The et3400 is "ET-3400 Microprocessor Trainer".
* H8/H89: the interrupt controller gives the highest pending level priority, as the 74LS148 on both CPU boards does. Before, the lowest level won, so the 2 mSec clock at level 1 came ahead of every card on the bus.
* H89 bus: a card that asks the I/O decoder for select lines its slot does not carry gets no ports. Before, it got the whole I/O space.
* H8/H89: the H-8-17 and H-88-1 controllers write to disk images.
* H8/H89: add the `h17_flop` software list of hard-sectored disks: CP/M 2.2.02, 2.2.03 and 2.2.04, and HDOS 1.0 (issues 50.00.00 and 50.03.00), 1.5, 1.6 and 2.0, and HUG Software Disk II: Games 1. The H-88-1 owns the list, and the H-8-17 lists it as compatible.
* H89: add the `h37_flop` software list of soft-sectored disks: the H-37 builds of CP/M 2.2.03 and 2.2.04. The Z-37 controller owns the list.
* H9: add the H9 video terminal, as the rs232 option `h9` and as the `h9` system. It has no CPU, so the device models what the TTL boards do. Its layout shows the hood, the tube and all 67 keys, and every key can be clicked. The keyboard is bit-paired, as Heath built it.
* H89: name the boot controller for the software menus. Under the MTR-90 SW501 labels, SW501:5 picks Disk I/O #1 (the Z-89-37) or Disk I/O #2 (the H-88-1). With both cards fitted, the launch menu offers only the list of the controller that boots.
* Formats: add the H8D disk image format. The H-8-17 and H-88-1 controllers can load H8D images.
* Formats: h17disk and H8D images can be saved. h17disk saves version 2 images, and identifies only version 2 files. A save keeps the file's other blocks, such as Labl, Comm and Date, and each sector's status byte.
* Formats: read h17disk version 1 images. Version 1 stores a dump of each sector from the disk, so the sector headers come from the image.
* Formats: a save of an h17disk version 1 image writes a version 2 file in its place, and `floptool flopconvert auto h17disk` converts version 1 files to version 2. The version 2 file keeps the label, comment, date, imager, program, parameters and each sector's status. It loses the bytes the imager read around each sector and the raw track data, because version 2 has no blocks for them.
* Formats: h17disk sets write protect from the R/O flag in the DskF block, or in the parameters block of a version 1 file. The file manager's "Write protect" option changes the flag, and a save writes it to the file.
* Formats: h17disk names the parameters block "Parm". MAME 0.289 defined the name as "Park", but did not use it.
* Formats: blank disks can be created with 10 hard sectors.

### Floppy core

* Give every 10, 16 and 32 hard-sector variant its index holes, fix the swapped SSSD and SSDD mapping for 16 hard sectors, and report SSQD10 media as single-sided ([mamedev/mame#16098](https://github.com/mamedev/mame/pull/16098), not yet merged).

* Separate 'nonpersistent' (full emulated read/write support, but changes are not written back to the disk image file) and 'write protected' (the emulated system sees write protected media) floppy modes.
* Floppy formats: add `save_format()`, which names the format that saves an image a format loaded. By default it is the format itself when it supports saving. A format for an old file version can return the format for the current version. The h17disk version 1 format uses it, so "Write to source image" saves a version 1 file as version 2, and h17disk1 does not appear in the lists of formats to save as.
* File manager: the access modes are "Write to source image", "Write to duplicate image" and "Write to memory only". A separate "Write protect" option sets the drive's write protect line. It starts from the image's own write protect flag, or from the "Image without save support" setting for a format without save(), and it sets the image's flag.

### Software lists and images

* A software list inside a card belongs to that card. A software item mounts on a drive of the card that holds its list, and the file manager offers each drive only its own card's lists. It skips the "Software Lists" menu when only one list is usable.
* A driver can name its boot device with `driver_device::boot_device()`. The launch software menu then offers only the lists under that device. The answer comes from the system's default switch settings and its saved cfg file.
* An image option that names a software list the machine does not have is cleared with a warning, and the drive starts empty. Before, MAME stopped with a fatal error. This happened when a slot change removed the card that held the list.

### Options

* `skip_gameinfo` defaults to on, so the system information screen does not show at startup. Use `-noskip_gameinfo` to show it.

### Sound

* Cassette volume starts at -15 dB on every machine. This is the cassette device's slider in Slider Controls, and a cfg file can still change it.
* A sound device can set the volume its slider starts at with `set_default_user_output_gain()`. A cfg file saves a device's volume only when it differs from that default. Before, a cfg file saved any volume other than 0 dB.

## Merged into MAME after 0.289

MAME has merged these changes, so they will leave this list when Omnibus moves to a later MAME release.

### Heath/Zenith

* H89: read-only support for h17disk hard-sector disk images ([mamedev/mame#16211](https://github.com/mamedev/mame/pull/16211)).
* H89 bus: pass the P506 select lines through to the I/O decoder ([mamedev/mame#16147](https://github.com/mamedev/mame/pull/16147)).
