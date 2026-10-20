// license:BSD-3-Clause
// copyright-holders:Mark Garlanger
/*********************************************************************

Heath h17disk  disk image format

The Heath hard-sectored disk format for the H8 and H89 systems with the
H17 controller on the H8 and the H-88-1 controller on the H89.

*********************************************************************/
#ifndef MAME_FORMATS_H17DISK_H
#define MAME_FORMATS_H17DISK_H

#pragma once

#include "h17_common.h"

// Loads version 1 files.  save_format() returns the version 2 format, so a
// save writes a version 2 file over the version 1 file.  Version 2 has no
// place for the bytes the imager read around each sector or for the raw
// track data, so the save drops them.  It keeps the label, comment, date,
// imager, program, parameters and each sector's status, which load() stores
// in the image's format data.
class heath_h17d_v1_format : public heath_h17_format
{
public:
	heath_h17d_v1_format();

	int identify(util::random_read &io, uint32_t form_factor, const std::vector<uint32_t> &variants) const override;
	bool load(util::random_read &io, uint32_t form_factor, const std::vector<uint32_t> &variants, floppy_image &image) const override;

	const char *name() const noexcept override { return "h17disk1"; }
	const char *description() const noexcept override { return "Heath H17D disk image (version 1)"; }
	const char *extensions() const noexcept override { return "h17,h17d,h17disk"; }
	const floppy_image_format_t *save_format() const noexcept override;
};

class heath_h17d_v2_format : public heath_h17_format
{
public:
	heath_h17d_v2_format();

	int identify(util::random_read &io, uint32_t form_factor, const std::vector<uint32_t> &variants) const override;
	bool load(util::random_read &io, uint32_t form_factor, const std::vector<uint32_t> &variants, floppy_image &image) const override;
	bool save(util::random_read_write &io, const std::vector<uint32_t> &variants, const floppy_image &image) const override;

	const char *name() const noexcept override { return "h17disk"; }
	const char *description() const noexcept override { return "Heath H17D disk image"; }
	const char *extensions() const noexcept override { return "h17,h17d,h17disk"; }
	bool supports_save() const noexcept override { return true; }
};

extern const heath_h17d_v1_format FLOPPY_H17D_V1_FORMAT;
extern const heath_h17d_v2_format FLOPPY_H17D_V2_FORMAT;

#endif // MAME_FORMATS_H17DISK_H
