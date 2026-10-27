## Changes from MAME

Omnibus is based on MAME 0.289. This file lists the changes Omnibus makes to MAME. Update it when a change is added, or when MAME merges one of these changes.

### Heath/Zenith

* H8: add the H-8-17 hard-sectored floppy controller. A setting models the HA-8-8 ROM disable modification. The H-8-17 and the H89 H-88-1 controller now share code in `h17_fdc_base`.
* H8: gate the H-8-5 console interrupt with /DTR.
* H8/H89: the interrupt controller gives the highest pending level priority, as the 74LS148 on both CPU boards does. Before, the lowest level won, so the 2 mSec clock at level 1 came ahead of every card on the bus.
* H8/H89: the H-8-17 and H-88-1 controllers write to disk images.
* H8/H89: add the `h17_flop` software list of hard-sectored disks: CP/M 2.2.02, 2.2.03 and 2.2.04, and HDOS 1.0 (issues 50.00.00 and 50.03.00), 1.5, 1.6 and 2.0. The H-88-1 owns the list, and the H-8-17 lists it as compatible.
* H89: add the `h37_flop` software list of soft-sectored disks: the H-37 builds of CP/M 2.2.03 and 2.2.04. The Z-37 controller owns the list.
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

## Merged into MAME after 0.289

MAME has merged these changes, so they will leave this list when Omnibus moves to a later MAME release.

### Heath/Zenith

* H89: read-only support for h17disk hard-sector disk images ([mamedev/mame#16211](https://github.com/mamedev/mame/pull/16211)).
* H89 bus: pass the P506 select lines through to the I/O decoder ([mamedev/mame#16147](https://github.com/mamedev/mame/pull/16147)).
