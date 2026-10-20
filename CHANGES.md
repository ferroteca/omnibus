## Changes from MAME

Omnibus is based on MAME 0.289. This file lists the changes Omnibus makes to MAME. Update it when a change is added, or when MAME merges one of these changes.

### Heath/Zenith

* H8: add the H-8-17 hard-sectored floppy controller. A setting models the HA-8-8 ROM disable modification. The H-8-17 and the H89 H-88-1 controller now share code in `h17_fdc_base`.
* H8: gate the H-8-5 console interrupt with /DTR.
* Formats: add the H8D disk image format, read only. The H-8-17 and H-88-1 controllers can load H8D images.
* Formats: h17disk sets write protect from the R/O flag in the DskF block.

### Floppy core

* Give every 10, 16 and 32 hard-sector variant its index holes, fix the swapped SSSD and SSDD mapping for 16 hard sectors, and report SSQD10 media as single-sided ([mamedev/mame#16098](https://github.com/mamedev/mame/pull/16098), not yet merged).

* Separate 'nonpersistent' (full emulated read/write support, but changes are not written back to the disk image file) and 'write protected' (the emulated system sees write protected media) floppy modes.
* File manager: the access modes are "Write to source image", "Write to duplicate image" and "Write to memory only". A separate "Write protect" option sets the drive's write protect line. It starts from the image's own write protect flag, or from the "Image without save support" setting for a format without save(), and it sets the image's flag.

## Merged into MAME after 0.289

MAME has merged these changes, so they will leave this list when Omnibus moves to a later MAME release.

### Heath/Zenith

* H89: read-only support for h17disk hard-sector disk images ([mamedev/mame#16211](https://github.com/mamedev/mame/pull/16211)).
* H89 bus: pass the P506 select lines through to the I/O decoder ([mamedev/mame#16147](https://github.com/mamedev/mame/pull/16147)).
