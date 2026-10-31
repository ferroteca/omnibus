// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic
/*********************************************************************

    formats/imd_dsk.cpp

    IMD disk images

    IMD.TXT, the ImageDisk manual quoted below, is in
    http://dunfield.classiccmp.org/img42841/imd120.zip

*********************************************************************/

#include "imd_dsk.h"
#include "flopimg_legacy.h"
#include "imageutl.h"

#include "ioprocs.h"

#include "osdcore.h" // osd_printf_*

#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>



struct imddsk_tag
{
	int heads;
	int tracks;
	int track_sectors[84*2]; /* number of sectors for each track */
	int sector_size;
	uint64_t track_offsets[84*2]; /* offset within data for each track */
};


static struct imddsk_tag *get_tag(floppy_image_legacy *floppy)
{
	struct imddsk_tag *tag;
	tag = (imddsk_tag *)floppy_tag(floppy);
	return tag;
}



FLOPPY_IDENTIFY( imd_dsk_identify )
{
	uint8_t header[3];

	floppy_image_read(floppy, header, 0, 3);
	if (header[0]=='I' && header[1]=='M' && header[2]=='D') {
		*vote = 100;
	} else {
		*vote = 0;
	}
	return FLOPPY_ERROR_SUCCESS;
}

static int imd_get_heads_per_disk(floppy_image_legacy *floppy)
{
	return get_tag(floppy)->heads;
}

static int imd_get_tracks_per_disk(floppy_image_legacy *floppy)
{
	return get_tag(floppy)->tracks;
}

static int imd_get_sectors_per_track(floppy_image_legacy *floppy, int head, int track)
{
	return get_tag(floppy)->track_sectors[(track<<1) + head];
}

static uint64_t imd_get_track_offset(floppy_image_legacy *floppy, int head, int track)
{
	return get_tag(floppy)->track_offsets[(track<<1) + head];
}

static floperr_t get_offset(floppy_image_legacy *floppy, int head, int track, int sector, bool sector_is_index, uint64_t *offset)
{
	uint64_t offs = 0;
	uint8_t header[5];
	uint8_t sector_num;
	int i;


	if ((head < 0) || (head >= get_tag(floppy)->heads) || (track < 0) || (track >= get_tag(floppy)->tracks)
			|| (sector < 0) )
		return FLOPPY_ERROR_SEEKERROR;

	offs = imd_get_track_offset(floppy,head,track);
	floppy_image_read(floppy, header, offs, 5);

	sector_num = header[3];
	offs += 5 + sector_num; // skip header and sector numbering map
	if(header[2] & 0x80) offs += sector_num; // skip cylinder numbering map
	if(header[2] & 0x40) offs += sector_num; // skip head numbering map
	get_tag(floppy)->sector_size = 1 << (header[4] + 7);
	for(i=0;i<sector;i++) {
		floppy_image_read(floppy, header, offs, 1); // take sector data type
		switch(header[0]) {
			case 0: offs++; break;
			case 1:
			case 3:
			case 5:
			case 7: offs += get_tag(floppy)->sector_size + 1; break;
			default: offs += 2;
		}
	}
	if (offset)
		*offset = offs;
	return FLOPPY_ERROR_SUCCESS;
}



static floperr_t internal_imd_read_sector(floppy_image_legacy *floppy, int head, int track, int sector, bool sector_is_index, void *buffer, size_t buflen)
{
	uint64_t offset;
	floperr_t err;
	uint8_t header[1];

	// take sector offset
	err = get_offset(floppy, head, track, sector, sector_is_index, &offset);
	if (err)
		return err;

	floppy_image_read(floppy, header, offset, 1);
	switch(header[0]) {
		case 0: break;
		case 1:
		case 3:
		case 5:
		case 7:
				floppy_image_read(floppy, buffer, offset+1, buflen);
				break;

		default: // all data same
				floppy_image_read(floppy, header, offset+1, 1);
				memset(buffer,header[0],buflen);
				break;
	}

	return FLOPPY_ERROR_SUCCESS;
}


static floperr_t imd_read_sector(floppy_image_legacy *floppy, int head, int track, int sector, void *buffer, size_t buflen)
{
	return internal_imd_read_sector(floppy, head, track, sector, false, buffer, buflen);
}

static floperr_t imd_read_indexed_sector(floppy_image_legacy *floppy, int head, int track, int sector, void *buffer, size_t buflen)
{
	return internal_imd_read_sector(floppy, head, track, sector, true, buffer, buflen);
}

static floperr_t imd_expand_file(floppy_image_legacy *floppy , uint64_t offset , size_t amount)
{
		if (amount == 0) {
				return FLOPPY_ERROR_SUCCESS;
		}

		uint64_t file_size = floppy_image_size(floppy);

		if (offset > file_size) {
				return FLOPPY_ERROR_INTERNAL;
		}

	uint64_t size_after_off = file_size - offset;

	if (size_after_off == 0) {
		return FLOPPY_ERROR_SUCCESS;
	}

	auto buffer = std::make_unique<uint8_t []>(size_after_off);

	// Read the part of file after offset
	floppy_image_read(floppy, buffer.get(), offset, size_after_off);

	// Add zeroes
	floppy_image_write_filler(floppy, 0, offset, amount);

	// Write back the part of file after offset
	floppy_image_write(floppy, buffer.get(), offset + amount, size_after_off);

	buffer.reset();

	// Update track offsets
	struct imddsk_tag *tag = get_tag(floppy);
	for (int track = 0; track < tag->tracks; track++) {
		for (int head = 0; head < tag->heads; head++) {
			uint64_t *track_off = &(tag->track_offsets[ (track << 1) + head ]);
			if (*track_off >= offset) {
				*track_off += amount;
			}
		}
	}

	return FLOPPY_ERROR_SUCCESS;
}

static floperr_t imd_write_indexed_sector(floppy_image_legacy *floppy, int head, int track, int sector_index, const void *buffer, size_t buflen, int ddam)
{
	uint64_t offset;
	floperr_t err;
	uint8_t header[1];

	// take sector offset
	err = get_offset(floppy, head, track, sector_index, true, &offset);
	if (err)
		return err;

	floppy_image_read(floppy, header, offset, 1);

	switch (header[ 0 ]) {
	case 0:
		return FLOPPY_ERROR_SEEKERROR;

	default:
		// Expand image file (from 1 byte to a whole sector)
		err = imd_expand_file(floppy , offset , buflen - 1);
		if (err) {
			return err;
		}
		[[fallthrough]];

	case 1:
	case 3:
	case 5:
	case 7:
		// Turn every kind of sector into type 1 (normal data)
		header[ 0 ] = 1;
		floppy_image_write(floppy, header, offset, 1);
		// Write sector
		floppy_image_write(floppy, buffer, offset + 1, buflen);
		break;
	}

	return FLOPPY_ERROR_SUCCESS;
}

static floperr_t imd_get_sector_length(floppy_image_legacy *floppy, int head, int track, int sector, uint32_t *sector_length)
{
	floperr_t err;
	err = get_offset(floppy, head, track, sector, false, nullptr);
	if (err)
		return err;

	if (sector_length) {
		*sector_length = get_tag(floppy)->sector_size;
	}
	return FLOPPY_ERROR_SUCCESS;
}

static floperr_t imd_get_indexed_sector_info(floppy_image_legacy *floppy, int head, int track, int sector_index, int *cylinder, int *side, int *sector, uint32_t *sector_length, unsigned long *flags)
{
	uint64_t offset;
	uint8_t header[5];
	uint8_t hd;
	uint8_t tr;
	uint32_t sector_size;
	uint8_t sector_num;

	offset = imd_get_track_offset(floppy,head,track);
	floppy_image_read(floppy, header, offset, 5);
	tr = header[1];
	hd = header[2];
	sector_num = header[3];
	sector_size = 1 << (header[4] + 7);
	if (sector_index >= sector_num) return FLOPPY_ERROR_SEEKERROR;
	if (cylinder) {
		if (head & 0x80) {
			floppy_image_read(floppy, header, offset + 5 + sector_num+ sector_index, 1);
			*cylinder = header[0];
		} else {
			*cylinder = tr;
		}
	}
	if (side) {
		if (head & 0x40) {
			if (head & 0x80) {
				floppy_image_read(floppy, header, offset + 5 + 2 * sector_num+sector_index, 1);
			} else {
				floppy_image_read(floppy, header, offset + 5 + sector_num+sector_index, 1);
			}
			*side = header[0];
		} else {
			*side = hd & 1;
		}
	}
	if (sector) {
		floppy_image_read(floppy, header, offset + 5 + sector_index, 1);
		*sector = header[0];
	}
	if (sector_length) {
		*sector_length = sector_size;
	}
	if (flags) {
		uint8_t skip;
		if (head & 0x40) {
			if (head & 0x80) {
				skip = 3;
			} else {
				skip = 2;
			}
		} else {
			skip = 1;
		}
		floppy_image_read(floppy, header, offset + 5 + skip * sector_num, 1);
		*flags = 0;
		if ((header[0]-1) & 0x02) *flags |= ID_FLAG_DELETED_DATA;
		if ((header[0]-1) & 0x04) *flags |= ID_FLAG_CRC_ERROR_IN_DATA_FIELD;
	}
	return FLOPPY_ERROR_SUCCESS;
}


FLOPPY_CONSTRUCT( imd_dsk_construct )
{
	struct FloppyCallbacks *callbacks;
	struct imddsk_tag *tag;
	uint8_t header[0x100];
	uint64_t pos = 0;
	int sector_size = 0;
	int sector_num;
	int i;
	if(params)
	{
		// create
		return FLOPPY_ERROR_UNSUPPORTED;
	}

	tag = (struct imddsk_tag *) floppy_create_tag(floppy, sizeof(struct imddsk_tag));
	if (!tag)
		return FLOPPY_ERROR_OUTOFMEMORY;

	floppy_image_read(floppy, header, pos, 1);
	while(header[0]!=0x1a) {
		pos++;
		floppy_image_read(floppy, header, pos, 1);
	}
	pos++;
	tag->tracks = 0;
	tag->heads = 1;
	do {
		floppy_image_read(floppy, header, pos, 5);
		sector_num = header[3];
		int track = (header[1]<<1) + (header[2] & 1);
		if ((header[2] & 1)==1) tag->heads = 2;
		tag->track_offsets[track] = pos;
		tag->track_sectors[track] = sector_num;
		pos += 5 + sector_num; // skip header and sector numbering map
		if(header[2] & 0x80) pos += sector_num; // skip cylinder numbering map
		if(header[2] & 0x40) pos += sector_num; // skip head numbering map
		sector_size = 1 << (header[4] + 7);
		for(i=0;i<sector_num;i++) {
			floppy_image_read(floppy, header, pos, 1); // take sector data type
			switch(header[0]) {
				case 0: pos++; break;
				case 1:
				case 3:
				case 5:
				case 7: pos += sector_size + 1; break;
				default: pos += 2;break;
			}
		}
		tag->tracks += 1;
	} while(pos < floppy_image_size(floppy));
	if (tag->heads==2) {
		tag->tracks = tag->tracks / 2;
	}
	callbacks = floppy_callbacks(floppy);
	callbacks->read_sector = imd_read_sector;
	callbacks->read_indexed_sector = imd_read_indexed_sector;
	callbacks->write_indexed_sector = imd_write_indexed_sector;
	callbacks->get_sector_length = imd_get_sector_length;
	callbacks->get_heads_per_disk = imd_get_heads_per_disk;
	callbacks->get_tracks_per_disk = imd_get_tracks_per_disk;
	callbacks->get_indexed_sector_info = imd_get_indexed_sector_info;
	callbacks->get_sectors_per_track = imd_get_sectors_per_track;

	return FLOPPY_ERROR_SUCCESS;
}


// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
/*********************************************************************

    formats/imd_dsk.cpp

    IMD disk images

*********************************************************************/

imd_format::imd_format()
{
}

const char *imd_format::name() const noexcept
{
	return "imd";
}

const char *imd_format::description() const noexcept
{
	return "IMD disk image";
}

const char *imd_format::extensions() const noexcept
{
	return "imd";
}

int imd_format::identify(util::random_read &io, uint32_t form_factor, const std::vector<uint32_t> &variants) const
{
	char h[4];
	auto const [err, actual] = read_at(io, 0, h, 4);
	if(err || (4 != actual))
		return 0;

	if(!memcmp(h, "IMD ", 4))
		return FIFID_SIGN;

	return 0;
}

bool imd_format::load(util::random_read &io, uint32_t form_factor, const std::vector<uint32_t> &variants, floppy_image &image) const
{
	std::vector<uint8_t> comment;
	std::vector<std::vector<uint8_t> > snum;
	std::vector<std::vector<uint8_t> > tnum;
	std::vector<std::vector<uint8_t> > hnum;

	std::vector<uint8_t> mode;
	std::vector<uint8_t> track;
	std::vector<uint8_t> head;
	std::vector<uint8_t> sector_count;
	std::vector<uint8_t> ssize;

	int trackmult;
	uint64_t size;
	if(io.length(size))
		return false;

	auto const [err, img, actual] = read_at(io, 0, size);
	if(err || (actual != size))
		return false;

	uint64_t pos, savepos;
	for(pos=0; pos < size && img[pos] != 0x1a; pos++) { }
	if(pos >= size)
		return false;    // no comment terminator, so not an IMD file
	pos++;

	comment.resize(pos);
	memcpy(&comment[0], &img[0], pos);

	// A file that ends after the comment has no track records, so every
	// cylinder is unformatted.  save() writes this for a blank disk.  Load it
	// as a blank disk with the drive's form factor and no variant.  The next
	// save after the machine formats the disk takes the variant from the
	// tracks.
	if(pos >= size) {
		image.set_form_variant(form_factor, 0);
		return true;
	}

	int tracks, heads;
	image.get_maximal_geometry(tracks, heads);

	mode.clear();
	track.clear();
	head.clear();
	sector_count.clear();
	ssize.clear();
	trackmult = 1;

	// we have to walk the whole file to find out the number of tracks
	//
	// maxtrack is the highest cylinder number in the record headers.  It
	// decides where the tracks go in the drive.  maxidtrack is the highest
	// cylinder number in the sector headers, taken from the cylinder map when
	// a record has one.  It gives the number of cylinders on the disk itself.
	// Records with no sectors do not count towards maxidtrack.
	//
	// The two differ for a 48 tpi disk saved from a 96 tpi drive by older
	// code.  That file has records on cylinders 0, 2, 4 ... 78, and each one
	// has a cylinder map that numbers its sectors 0 to 39.  maxtrack is 78,
	// but the disk has 40 cylinders, and maxidtrack is 39.
	savepos = pos;
	uint8_t maxtrack = 0;
	uint8_t maxidtrack = 0;
	while(pos < size)
	{
		pos++;   // skip mode
		uint8_t track = img[pos++];
		uint8_t head = img[pos++];
		uint8_t sector_count = img[pos++];
		uint8_t sector_size = img[pos++];
		int actual_size = sector_size < 7 ? 128 << sector_size : 8192;

		if (track > maxtrack)
		{
			maxtrack = track;
		}

		pos += sector_count;
		if (head & 0x80)
		{
			for (int i = 0; i < sector_count; i++)
			{
				if (img[pos + i] > maxidtrack)
				{
					maxidtrack = img[pos + i];
				}
			}
			pos += sector_count;
		}
		else if (sector_count && (track > maxidtrack))
		{
			maxidtrack = track;
		}
		if (head & 0x40)
		{
			pos += sector_count;
		}

		for (int i = 0; i < sector_count; i++)
		{
			uint8_t stype = img[pos++];
			if (stype == 0 || stype > 8)
			{
			}
			else
			{
				if (stype == 2 || stype == 4 || stype == 6 || stype == 8)
				{
					pos++;
				}
				else
				{
					pos += actual_size;
				}
			}
		}
	}

	if(form_factor == floppy_image::FF_525)
	{
		// On 5.25, check if the drive is QD or HD but we're a 40 track
		// image.  If so, put the image on even tracks.
		if ((has_variant(variants, floppy_image::SSQD)) ||
			(has_variant(variants, floppy_image::DSQD)) ||
			(has_variant(variants, floppy_image::DSHD)))
		{
			if (maxtrack <= 39)
				trackmult = 2;
		}
		else
		{
			if (maxtrack > 42)
				return false;
		}
	}

	pos = savepos;
	while(pos < size) {
		mode.push_back(img[pos++]);
		track.push_back(img[pos++]);
		head.push_back(img[pos++]);
		sector_count.push_back(img[pos++]);
		ssize.push_back(img[pos++]);

		if(track.back() >= tracks)
		{
			osd_printf_error("imd_format: Track %d exceeds maximum of %d\n", track.back(), tracks);
			return false;
		}

		if((head.back() & 0x3f) >= heads)
		{
			osd_printf_error("imd_format: Head %d exceeds maximum of %d\n", head.back() & 0x3f, heads);
			return false;
		}

		if(ssize.back() == 0xff)
		{
			osd_printf_error("imd_format: Unsupported variable sector size on track %d head %d", track.back(), head.back() & 0x3f);
			return false;
		}

		uint32_t actual_size = ssize.back() < 7 ? 128 << ssize.back() : 8192;

		static const int rates[3] = { 500000, 300000, 250000 };
		bool fm = mode.back() < 3;
		int rate = rates[mode.back() % 3];
		int rpm = form_factor == floppy_image::FF_8 || (form_factor == floppy_image::FF_525 && rate >= 300000) ? 360 : 300;
		int cell_count = (fm ? 1 : 2)*rate*60/rpm;

		//const uint8_t *snum = &img[pos];
		snum.push_back(std::vector<uint8_t>(sector_count.back()));
		memcpy(&snum.back()[0], &img[pos], sector_count.back());
		pos += sector_count.back();

		//const uint8_t *tnum = head & 0x80 ? &img[pos] : nullptr;
		if (head.back() & 0x80)
		{
			tnum.push_back(std::vector<uint8_t>(sector_count.back()));
			memcpy(&tnum.back()[0], &img[pos], sector_count.back());
			pos += sector_count.back();
		}
		else
		{
			tnum.push_back(std::vector<uint8_t>(0));
		}

		//const uint8_t *hnum = head & 0x40 ? &img[pos] : nullptr;
		if (head.back() & 0x40)
		{
			hnum.push_back(std::vector<uint8_t>(sector_count.back()));
			memcpy(&hnum.back()[0], &img[pos], sector_count.back());
			pos += sector_count.back();
		}
		else
		{
			hnum.push_back(std::vector<uint8_t>(0));
		}

		uint8_t chead = head.back() & 0x3f;

		int gap_3 = calc_default_pc_gap3_size(form_factor, actual_size);

		desc_pc_sector sects[256];

		for(int i=0; i<sector_count.back(); i++) {
			uint8_t stype        = img[pos++];
			sects[i].track       = tnum.back().size() ? tnum.back()[i] : track.back();
			// Use chead, not head.back().  The head byte in the file also holds
			// the 0x80 and 0x40 flags that mark a cylinder map and a head map,
			// and the sector header must not get those bits.
			sects[i].head        = hnum.back().size() ? hnum.back()[i] : chead;
			sects[i].sector      = snum.back()[i];
			sects[i].size        = ssize.back();
			sects[i].actual_size = actual_size;

			if(stype == 0 || stype > 8) {
				sects[i].data = nullptr;
			} else {
				sects[i].deleted = stype == 3 || stype == 4 || stype == 7 || stype == 8;
				sects[i].bad_data_crc = stype == 5 || stype == 6 || stype == 7 || stype == 8;
				sects[i].bad_addr_crc = false;

				if(stype == 2 || stype == 4 || stype == 6 || stype == 8) {
					sects[i].data = new uint8_t [actual_size];
					memset(sects[i].data, img[pos++], actual_size);
				} else {
					sects[i].data = &img[pos];
					pos += actual_size;
				}
			}
		}

		if(sector_count.back()) {
			if(fm) {
				build_pc_track_fm(track.back()*trackmult, chead, image, cell_count, sector_count.back(), sects, gap_3);
			} else {
				build_pc_track_mfm(track.back()*trackmult, chead, image, cell_count, sector_count.back(), sects, gap_3);
			}
		}

		for(int i=0; i< sector_count.back(); i++)
			if(sects[i].data && (sects[i].data < &img[0] || sects[i].data >= (&img[0] + size)))
				delete [] sects[i].data;
	}

	// Tag the image with form-factor + variant.  Without this the floppy_image
	// keeps FF_UNKNOWN/variant=0, which propagates to any subsequent save
	// (e.g. MFI) and to MAME runtime code that uses the form factor to pick
	// rotation rate / cell timing.  When the caller passed FF_UNKNOWN (e.g.
	// floptool's auto-detect) we infer everything from the parsed geometry;
	// when the caller passed a real form factor we keep theirs and only deduce
	// the variant.
	// Use maxidtrack, not maxtrack, here.  The form factor and variant
	// describe the disk, not where its tracks sit in the drive.
	uint32_t img_form = form_factor;
	if (img_form == floppy_image::FF_UNKNOWN) {
		if (maxidtrack >= 75 && maxidtrack <= 78)
			img_form = floppy_image::FF_8;       // 77-track 8"
		else if (maxidtrack <= 42)
			img_form = floppy_image::FF_525;     // 40-track 5.25"
		else
			img_form = floppy_image::FF_525;     // 80-track: ambiguous 5.25" vs 3.5"; prefer 525 (IMD-era PC default)
	}

	// Skip records with no sectors.  Older saves wrote one for each
	// unformatted cylinder, and their mode byte is always 3 (500 kbps MFM),
	// because detect_track() found nothing to measure.  A 48 tpi disk saved
	// from a 96 tpi drive is half such records.  Counting them made a 100K
	// single density disk load as DSHD.
	bool any_mfm = false;
	bool any_500kbps = false;
	uint8_t maxhead = 0;
	for (size_t i = 0; i < mode.size(); i++) {
		if (!sector_count[i])
			continue;
		if (mode[i] >= 3) any_mfm = true;
		if (mode[i] == 0 || mode[i] == 3) any_500kbps = true;
		if ((head[i] & 0x3f) > maxhead) maxhead = head[i] & 0x3f;
	}
	const bool ds = (maxhead >= 1);

	uint32_t img_variant;
	if (img_form == floppy_image::FF_8) {
		img_variant = any_mfm ? (ds ? floppy_image::DSDD : floppy_image::SSDD)
							  : (ds ? floppy_image::DSSD : floppy_image::SSSD);
	} else if (img_form == floppy_image::FF_525 || img_form == floppy_image::FF_35) {
		if (any_500kbps && any_mfm)
			img_variant = floppy_image::DSHD;
		else if (maxidtrack > 42)
			// More than 42 cylinders means 96 tpi media, so the variant is
			// SSQD or DSQD whatever the encoding.  save() reads the variant
			// to decide whether load() doubled the tracks.  If a single-sided
			// 80-track disk loaded as SSDD, save() would take it for 48 tpi
			// media and drop every other track.  An 80-track FM disk also
			// gets a QD variant: the density is wrong for FM, but the track
			// pitch is right.
			img_variant = ds ? floppy_image::DSQD : floppy_image::SSQD;
		else if (any_mfm)
			img_variant = ds ? floppy_image::DSDD : floppy_image::SSDD;
		else
			img_variant = ds ? floppy_image::DSSD : floppy_image::SSSD;
	} else {
		img_variant = any_mfm ? (ds ? floppy_image::DSDD : floppy_image::SSDD)
							  : (ds ? floppy_image::DSSD : floppy_image::SSSD);
	}
	image.set_form_variant(img_form, img_variant);

	return true;
}


/*********************************************************************

    Save side: flux-level floppy_image -> IMD container.

    Approach:
      1. For each (cyl, head), probe ten (encoding, cell_size)
         combinations covering 250/300/500 kbps MFM/FM at 300 and 360
         RPM.  The combination that yields the most cleanly-decoded
         sectors wins.
      2. Walk the resulting per-track sector list (in physical order)
         and emit IMD's per-track record: 5-byte header, sector
         numbering map, optional cyl/head maps, then per-sector type
         byte + data (compressed when the sector is all-fill).
      3. A fresh "Created by MAME flopconvert" header with localtime is
         emitted at the start of every saved IMD.

    In step 1, several combinations can decode the same number of
    sectors.  The one chosen becomes the IMD mode byte, and load()
    rebuilds the track at that rate.  If it is the wrong rate, the saved
    disk cannot be read: every sector is in the file, but the track is
    too fast or too slow for the drive.  Nothing reports an error.

    This happened to a CP/M distribution disk.  The PLL locks over
    about a 2:1 range of cell sizes, so four probes read all ten
    sectors of each track, and the first one in the table won.  Ties
    are now settled by measure_cell_size(), which measures the cell size
    from the flux.

*********************************************************************/

bool imd_format::supports_save() const noexcept
{
	return true;
}

// Single-bit reader with wrap-around.  The private sbit_rp method in
// floppy_image_format_t is not visible from this translation unit, so
// keep an identical local copy.
static inline bool sbit_local(const std::vector<bool> &bs, uint32_t &pos)
{
	bool b = bs[pos++];
	if (pos == bs.size()) pos = 0;
	return b;
}

void imd_format::extract_track_rich(const std::vector<bool> &bs, bool is_mfm,
									std::vector<extracted_sector> &out) const
{
	out.clear();
	if (bs.size() < 100)
		return;

	// Pass 1: locate every IDAM and every DAM in the bitstream, recording
	// both the position immediately after the type byte and the actual
	// type byte (needed later for CRC verification).
	struct mark { uint32_t pos; uint8_t type_byte; };
	std::vector<mark> idam_marks, dam_marks;

	uint16_t shift_reg = 0;
	// Precharge for wrap-around-the-index detection.
	for (uint32_t i = 0; i < 16; i++)
		if (bs[bs.size() - 16 + i])
			shift_reg |= 0x8000 >> i;

	if (is_mfm) {
		for (uint32_t i = 0; i < bs.size(); i++) {
			shift_reg = uint16_t((shift_reg << 1) | bs[i]);
			if (shift_reg == 0x4489) {        // MFM 0xA1 sync (missing clock)
				uint16_t header_word;
				uint32_t pos = i + 1;
				do {
					header_word = 0;
					for (int j = 0; j < 16; j++)
						if (sbit_local(bs, pos))
							header_word |= 0x8000 >> j;
				} while (header_word == 0x4489 && pos > i);

				// 0x5554 = MFM(0xFE), 0x5555 = MFM(0xFF)
				if (header_word == 0x5554 || header_word == 0x5555) {
					idam_marks.push_back({pos, uint8_t(header_word == 0x5554 ? 0xfe : 0xff)});
					i = pos - 1;
				}
				// 0x554a = MFM(0xF8) deleted DAM
				else if (header_word == 0x554a) {
					dam_marks.push_back({pos, 0xf8});
					i = pos - 1;
				}
				// 0x5549 = MFM(0xF9), 0x5544 = MFM(0xFA), 0x5545 = MFM(0xFB)
				else if (header_word == 0x5549) {
					dam_marks.push_back({pos, 0xf9});
					i = pos - 1;
				} else if (header_word == 0x5544) {
					dam_marks.push_back({pos, 0xfa});
					i = pos - 1;
				} else if (header_word == 0x5545) {
					dam_marks.push_back({pos, 0xfb});
					i = pos - 1;
				}
			}
		}
	} else {
		// FM: address marks have unique clock patterns; no separate sync
		// byte.  See IBM 3740 SD format.
		for (uint32_t i = 0; i < bs.size(); i++) {
			shift_reg = uint16_t((shift_reg << 1) | bs[i]);
			if (shift_reg == 0xf57e) {                                   // 0xFE
				idam_marks.push_back({i + 1, 0xfe});
			} else if (shift_reg == 0xf56a) {                            // 0xF8 deleted
				dam_marks.push_back({i + 1, 0xf8});
			} else if (shift_reg == 0xf56b) {                            // 0xF9
				dam_marks.push_back({i + 1, 0xf9});
			} else if (shift_reg == 0xf56e) {                            // 0xFA
				dam_marks.push_back({i + 1, 0xfa});
			} else if (shift_reg == 0xf56f) {                            // 0xFB
				dam_marks.push_back({i + 1, 0xfb});
			}
		}
	}

	// Pass 2: walk IDAMs in physical order, read the 4-byte sector header
	// plus CRC, find the matching DAM, read data + CRC, validate both.
	for (size_t i = 0; i < idam_marks.size(); i++) {
		uint32_t ipos = idam_marks[i].pos;

		extracted_sector sec;
		sec.idam_track  = sbyte_mfm_r(bs, ipos);
		sec.idam_head   = sbyte_mfm_r(bs, ipos);
		sec.idam_sector = sbyte_mfm_r(bs, ipos);
		sec.idam_size   = sbyte_mfm_r(bs, ipos);
		uint8_t crc_hi  = sbyte_mfm_r(bs, ipos);
		uint8_t crc_lo  = sbyte_mfm_r(bs, ipos);
		uint16_t idam_stored_crc = uint16_t(uint16_t(crc_hi) << 8 | crc_lo);

		uint8_t idam_buf[8];
		size_t idam_buf_len;
		if (is_mfm) {
			idam_buf[0] = 0xa1; idam_buf[1] = 0xa1; idam_buf[2] = 0xa1;
			idam_buf[3] = idam_marks[i].type_byte;
			idam_buf[4] = sec.idam_track;
			idam_buf[5] = sec.idam_head;
			idam_buf[6] = sec.idam_sector;
			idam_buf[7] = sec.idam_size;
			idam_buf_len = 8;
		} else {
			idam_buf[0] = idam_marks[i].type_byte;
			idam_buf[1] = sec.idam_track;
			idam_buf[2] = sec.idam_head;
			idam_buf[3] = sec.idam_sector;
			idam_buf[4] = sec.idam_size;
			idam_buf_len = 5;
		}
		sec.addr_crc_ok = (ccitt_crc16(0xffff, idam_buf, idam_buf_len) == idam_stored_crc);

		if (sec.idam_size >= 8) {
			// Idam size >= 8 means the size code byte fell outside the
			// spec range 0..6.  Almost always flux noise dressed up as a
			// false sync.  Drop entirely.
			continue;
		}
		int ssize = 128 << sec.idam_size;

		// Find first DAM after this IDAM within the IBM-spec tolerance.
		// MFM: 704 cells nominal, allow 704-128 .. 1008+128.
		// FM:  384 cells nominal, allow 384-128 .. 384+128.
		int dam_idx = -1;
		for (size_t d = 0; d < dam_marks.size(); d++) {
			int delta = int(dam_marks[d].pos) - int(idam_marks[i].pos);
			if (is_mfm) {
				if (delta >= 704 - 128 && delta <= 1008 + 128) { dam_idx = int(d); break; }
			} else {
				if (delta >= 384 - 128 && delta <= 384 + 128) { dam_idx = int(d); break; }
			}
		}
		if (dam_idx < 0) {
			// No matching DAM after this IDAM.  Real sectors usually
			// have a DAM; a missing-data sector is rare but legal in
			// IMD (sector type 0).  We only emit one if the IDAM CRC
			// validated — otherwise it's more likely a phantom sync.
			if (sec.addr_crc_ok) out.push_back(std::move(sec));
			continue;
		}

		sec.deleted_dam = (dam_marks[dam_idx].type_byte == 0xf8);

		uint32_t dpos = dam_marks[dam_idx].pos;
		sec.data.resize(ssize);
		for (int j = 0; j < ssize; j++)
			sec.data[j] = sbyte_mfm_r(bs, dpos);
		uint8_t dcrc_hi = sbyte_mfm_r(bs, dpos);
		uint8_t dcrc_lo = sbyte_mfm_r(bs, dpos);
		uint16_t dam_stored_crc = uint16_t(uint16_t(dcrc_hi) << 8 | dcrc_lo);

		std::vector<uint8_t> dam_buf;
		if (is_mfm) {
			dam_buf = { 0xa1, 0xa1, 0xa1, dam_marks[dam_idx].type_byte };
		} else {
			dam_buf = { dam_marks[dam_idx].type_byte };
		}
		dam_buf.insert(dam_buf.end(), sec.data.begin(), sec.data.end());
		sec.data_crc_ok = (ccitt_crc16(0xffff, dam_buf.data(), size_t(dam_buf.size())) == dam_stored_crc);
		sec.has_data    = true;

		if (sec.addr_crc_ok || sec.data_crc_ok)
			out.push_back(std::move(sec));
	}

	// Deduplicate by sector ID.  Phantom IDAMs (random sync-pattern hits in
	// noisy flux) can produce extra entries with the same idam_sector as a
	// real sector but no matching DAM.  When two candidates share the same
	// sector ID, prefer the one with valid data over the one without; if
	// both have data, prefer good data CRC; if both lack data, prefer the
	// good IDAM CRC.  Stable: preserves the physical order of the kept
	// entries.
	auto rank = [](const extracted_sector &s) -> int {
		// Higher rank = more trustworthy
		if (s.has_data && s.data_crc_ok && s.addr_crc_ok) return 4;
		if (s.has_data && s.addr_crc_ok)                   return 3;
		if (s.has_data)                                    return 2;
		if (s.addr_crc_ok)                                 return 1;
		return 0;
	};
	std::vector<extracted_sector> dedup;
	dedup.reserve(out.size());
	for (auto &candidate : out) {
		bool replaced = false;
		for (auto &kept : dedup) {
			if (kept.idam_sector == candidate.idam_sector) {
				if (rank(candidate) > rank(kept))
					kept = std::move(candidate);
				replaced = true;
				break;
			}
		}
		if (!replaced)
			dedup.push_back(std::move(candidate));
	}
	out = std::move(dedup);
}

// Measure the cell size of a track from its flux.  detect_track() uses it
// when two probes decode the same number of sectors.  Without it, the probe
// listed first wins, and its rate goes into the saved file.  A wrong rate
// makes the saved disk unreadable, with no error reported.
//
// Probes with the same encoding often tie.  The PLL locks on cell sizes up to
// about twice or half the real one, so on a 3333 ns FM track the 2400, 3333,
// 4000 and 4800 ns FM probes all read every sector.  The measurement picks
// 3333.
//
// The measurement cannot tell FM from MFM.  The shortest gap between
// transitions is one cell for FM and two cells for MFM, so FM 500 kbps (2000
// ns cells) and MFM 500 kbps (1000 ns cells) both measure 2000.  This does not
// matter, because only the right encoding decodes any sectors, so the score
// already separates them.  A caller must double an MFM probe's cell size
// before comparing it with this measurement.
//
// The value is the gap at the 5th percentile, not the shortest gap, so one
// write splice or weak bit cannot decide it.  These values came from saving a
// small IMD in each mode and measuring its tracks:
//
//   mode 0  FM  500   cell 2000   measured 2000      mode 3  MFM 500   cell 1000   measured 2000
//   mode 1  FM  300   cell 3333   measured 3333      mode 4  MFM 300   cell 1666   measured 3333
//   mode 2  FM  250   cell 4000   measured 4000      mode 5  MFM 250   cell 2000   measured 4000
//
// It returns 0 when a track has fewer than 64 transitions.  detect_track()
// then settles ties by table order.
//
// This runs only on save.  Loading an image and emulating from it never call
// detect_track().  The whole of detect_track() takes about 6 ms a track on a
// 40 track disk, and nearly all of that time is its ten PLL passes.
static int measure_cell_size(const floppy_image &image, int cyl, int head)
{
	const std::vector<uint32_t> &buf = image.get_buffer(cyl, head);

	if (buf.size() < 64)
		return 0;

	std::vector<uint32_t> gaps;
	gaps.reserve(buf.size());

	for (size_t i = 1; i < buf.size(); i++) {
		uint32_t const prev = buf[i - 1] & floppy_image::TIME_MASK;
		uint32_t const here = buf[i] & floppy_image::TIME_MASK;

		if (here > prev)
			gaps.push_back(here - prev);
	}

	if (gaps.size() < 64)
		return 0;

	// only the one value is wanted, so there is no reason to sort the rest
	auto const nth = gaps.begin() + gaps.size() / 20;
	std::nth_element(gaps.begin(), nth, gaps.end());

	return int(*nth);
}

bool imd_format::detect_track(const floppy_image &image, int cyl, int head,
							  track_info &out) const
{
	// (encoding, cell_size_ns, IMD mode byte)
	// cell_size is in MAME's internal flux representation: 200,000,000 ns
	// canonical per revolution / cells_per_revolution.  So a given media rate
	// needs *different* probe cell_sizes for 300 RPM vs 360 RPM disks:
	//   300 RPM, MFM 500 kbps: 200M / 200000 = 1000 ns/cell  (5.25" HD, 3.5" HD)
	//   360 RPM, MFM 500 kbps: 200M / 166666 = 1200 ns/cell  (8" DSDD/SSDD)
	//   300 RPM, MFM 250 kbps: 200M / 100000 = 2000 ns/cell  (5.25" DD, 3.5" DD)
	//   360 RPM, MFM 250 kbps: 200M /  83333 = 2400 ns/cell  (8" rare)
	//   300 RPM, FM  500 kbps: 200M / 100000 = 2000 ns/cell
	//   360 RPM, FM  500 kbps: 200M /  83333 = 2400 ns/cell  (8" SSSD)
	//   300 RPM, FM  250 kbps: 200M /  50000 = 4000 ns/cell  (5.25" SD; T-200/250)
	//   360 RPM, FM  250 kbps: 200M /  41666 = 4800 ns/cell  (8" rare)
	// Without the 360 RPM variants, MAME-loaded 8" MFM disks save as empty IMD
	// records because the PLL's ~25% lock tolerance is overrun by the 20%
	// cell-size mismatch (1200 vs 1000) -- the 5.25" probe scores zero sectors
	// and detect_track returns "mode 3 with no sectors".
	//
	// IMD.TXT section 6.1 (the file header gives its URL) defines the mode
	// byte: 0-2 are FM at 500/300/250 kbps and 3-5 the same for MFM, where
	// "kbps indicates transfer rate, not the data rate, which is 1/2 for FM
	// encoding".  That is why load() uses (fm ? 1 : 2).
	//
	// The two 300 kbps modes are probed at 300 RPM only:
	//   300 RPM, MFM 300 kbps: 200M / 120000 = 1666 ns/cell
	//   300 RPM, FM  300 kbps: 200M /  60000 = 3333 ns/cell
	// A 5.25" HD drive spins at 360 RPM, so it reads DD and QD media at 300
	// kbps.  IMD.TXT 3.3 says this "results in the same bit density as 250kbps
	// on the slower (300 RPM) drives".  At 360 RPM the two 300 kbps modes come
	// to 2000 and 4000 ns, the same cell sizes as the MFM 250 and FM 250
	// probes.  Probes for them would tie with those probes, and the mode byte
	// would depend on table order.
	//
	// Changing a cell size here, or adding a probe, can make a mode that saved
	// correctly save as a different mode.  To test a change, write one small
	// IMD in each mode, with a few tracks of ordinary sectors.  Convert each
	// one with "floptool flopconvert imd mfi" and back to IMD.  Every file must
	// come back with its own mode.  Before measure_cell_size() settled ties,
	// modes 1 and 4 came back as modes 0 and 5.
	struct probe { bool is_mfm; int cell_size; uint8_t mode; };
	static const probe probes[] = {
		{ true,  1000, 3 },   // MFM 500 kbps @ 300 RPM (5.25" HD, 3.5" HD)
		{ true,  1200, 3 },   // MFM 500 kbps @ 360 RPM (8" DSDD/SSDD)
		{ true,  1666, 4 },   // MFM 300 kbps @ 300 RPM
		{ true,  2000, 5 },   // MFM 250 kbps @ 300 RPM (5.25" DD, 3.5" DD)
		{ true,  2400, 5 },   // MFM 250 kbps @ 360 RPM (8")
		{ false, 2000, 0 },   // FM  500 kbps @ 300 RPM
		{ false, 2400, 0 },   // FM  500 kbps @ 360 RPM (8" SSSD)
		{ false, 3333, 1 },   // FM  300 kbps @ 300 RPM
		{ false, 4000, 2 },   // FM  250 kbps @ 300 RPM (5.25" SD; T-200/250 boot)
		{ false, 4800, 2 },   // FM  250 kbps @ 360 RPM (8")
	};
	int const measured = measure_cell_size(image, cyl, head);

	int best_score = -1;
	int best_delta = 0;
	track_info best;

	for (const auto &p : probes) {
		auto bs = generate_bitstream_from_track(cyl, head, p.cell_size, image);
		if (bs.empty())
			continue;

		std::vector<extracted_sector> secs;
		extract_track_rich(bs, p.is_mfm, secs);

		// Score by sectors with a good IDAM CRC.  Bad data CRC is not
		// penalised — the saved IMD will honestly mark such sectors
		// type 5/6/7/8 — but a bad IDAM CRC means we likely guessed
		// the wrong encoding.
		int score = 0;
		for (const auto &s : secs)
			if (s.addr_crc_ok) score++;

		// A higher score always wins, and that picks the encoding.  On a
		// tie, the probe nearest the measured cell size wins.  An MFM
		// probe's cell size is doubled first; see measure_cell_size().
		int const shortest = p.is_mfm ? 2 * p.cell_size : p.cell_size;
		int const delta = measured ? std::abs(shortest - measured) : 0;

		if ((score > best_score)
				|| ((score == best_score) && measured && (delta < best_delta))) {
			best_score      = score;
			best_delta      = delta;
			best.is_mfm     = p.is_mfm;
			best.cell_size  = p.cell_size;
			best.mode_byte  = p.mode;
			best.sectors    = std::move(secs);
		}
	}

	out = std::move(best);
	return best_score > 0;
}

bool imd_format::save(util::random_read_write &io, const std::vector<uint32_t> &variants,
					  const floppy_image &image) const
{
	// A disk with no formatted tracks is saved as the header with no records,
	// and load() reads that back as a blank disk.  Do not refuse to save it.
	// The drive truncates the file before it calls save() and ignores the
	// result, so a refusal leaves a 0 byte file.  No format identifies a 0
	// byte file, and a machine started with it mounted stops with a fatal
	// error.
	int tracks = 0, heads = 0;
	image.get_actual_geometry(tracks, heads);

	int max_tracks = 0, max_heads = 0;
	image.get_maximal_geometry(max_tracks, max_heads);

	// True when the drive is 96 tpi.  load() makes the same test to decide
	// whether to double the tracks.
	bool const drive_96tpi = (image.get_form_factor() == floppy_image::FF_525)
			&& (has_variant(variants, floppy_image::SSQD)
			 || has_variant(variants, floppy_image::DSQD)
			 || has_variant(variants, floppy_image::DSHD));

	// -- ASCII header.  Always fresh; the format manager is const, so
	//    we deliberately do not cache anything from a prior load().
	std::string header;
	{
		std::time_t const t = std::time(nullptr);
		std::tm const lt = *std::localtime(&t);
		char buf[64];
		std::snprintf(buf, sizeof(buf), "IMD 1.20: %02d/%02d/%04d %02d:%02d:%02d\r\n",
					  lt.tm_mday, lt.tm_mon + 1, lt.tm_year + 1900,
					  lt.tm_hour, lt.tm_min, lt.tm_sec);
		header  = buf;
		header += "Created by MAME flopconvert\r\n";
		header += '\x1a';
	}

	if (auto pr = write_at(io, 0, header.data(), header.size()); pr.first || pr.second != header.size())
		return false;
	uint64_t out_pos = header.size();

	// -- Undo the doubling load() applies to 48 tpi media in a 96 tpi drive.
	//
	// load() puts a 40-track image on the even cylinders of an 80-track drive,
	// because a 96 tpi head finds a 48 tpi disk's tracks there.  If save()
	// wrote those cylinder numbers out, the file would describe an 80-cylinder
	// disk with every other cylinder blank.  It would still load into a 96 tpi
	// drive, but a 48 tpi drive would reject it, because load() refuses a 5.25"
	// image with cylinders past 42.  So save() writes every second cylinder,
	// numbered 0 to 39.
	//
	// The image's variant shows when load() doubled the tracks.  load() sets
	// the variant from the cylinder numbers in the sector headers, so a 48 tpi
	// variant on an image with more than 42 cylinders means load() doubled it.
	//
	// A variant of 0 means no load() set one.  That only happens to an image
	// made by call_create(), which starts blank, so it is never doubled.  IMD
	// is the only format in MAME that doubles tracks.
	int trackmult = 1;
	if ((tracks > 42) && (image.get_form_factor() == floppy_image::FF_525)) {
		switch (image.get_variant()) {
		case floppy_image::SSSD: case floppy_image::DSSD:
		case floppy_image::SSDD: case floppy_image::DSDD:
			trackmult = 2;
			break;
		default:
			break;
		}
	}

	// -- How many cylinders to write.
	//
	// Normally the file stops at the last formatted cylinder.  One disk needs
	// more: 40 tracks on cylinders 0 to 39 of a 96 tpi drive, written at the
	// drive's own track pitch.  CP/M writes this disk when it is set up for 48
	// tpi drives but the drives are 96 tpi.
	//
	// load() doubles a 5.25" image whose cylinders stop at 39 when the drive
	// is 96 tpi (IMD.TXT section 2.1.4).  For this disk that is wrong.  Its
	// tracks are already in the right place, and doubling moves track 1 to
	// cylinder 2, so the boot loader steps to cylinder 1 and finds it empty.
	// trackmult == 2 above is a different disk: one that load() doubled
	// correctly.
	//
	// For this disk, save() writes empty records out to the drive's last
	// cylinder.  load() then sees cylinders past 39 and does not double the
	// tracks.
	//
	// The test asks for exactly 40 formatted cylinders, starting at 0, with no
	// gaps.  Any other disk, such as a partly formatted one, is left alone,
	// even though it may have the same problem.  IMD is a format for nearly
	// every floppy driver in MAME through add_fm_containers(), and more than a
	// hundred of those drivers have 5.25" QD or HD drives.  Change the test only
	// to fix a real disk that saves wrongly.
	bool describe_extent = drive_96tpi && (trackmult == 1)
						&& (tracks == 40) && (max_tracks > 42);
	for (int cyl = 0; describe_extent && (cyl < tracks); cyl++) {
		bool any = false;
		for (int hd = 0; hd < heads; hd++) {
			if (image.track_is_formatted(cyl, hd)) {
				any = true;
				break;
			}
		}
		if (!any)
			describe_extent = false;
	}
	int const out_tracks = describe_extent ? max_tracks : tracks;

	// The mode of the first track that decodes.  The empty records written
	// past the data use it.  detect_track() returns mode 3 (500 kbps MFM) for
	// a track with no sectors, and that mode is wrong for most disks.
	uint8_t disk_mode = 0xff;

	// -- Walk tracks in IMD's canonical order: cyl outer, head inner.
	for (int cyl = 0; cyl < out_tracks; cyl += trackmult) {
		uint8_t const out_cyl = uint8_t(cyl / trackmult);
		for (int hd = 0; hd < heads; hd++) {
			track_info ti;
			bool ok = detect_track(image, cyl, hd, ti);
			(void)ok;

			// Write no record for an unformatted cylinder.  Each record holds
			// its cylinder number, and load() places it by that number, so a
			// missing cylinder loads as unformatted.
			//
			// An empty record would need a mode byte, and detect_track()
			// returns mode 3 (500 kbps MFM) for a track with no sectors.  Other
			// programs that read the file would take that as high density.
			// Older saves wrote these records, and a 100K single density disk
			// loaded as DSHD.  load() now ignores them.
			uint8_t sec_count = uint8_t(ti.sectors.size());
			if (!sec_count && !describe_extent)
				continue;

			uint8_t mode      = sec_count ? ti.mode_byte
										  : (disk_mode != 0xff ? disk_mode : ti.mode_byte);
			uint8_t size_code = 0;

			if (sec_count) {
				if (disk_mode == 0xff)
					disk_mode = mode;
				size_code = ti.sectors[0].idam_size;
				for (size_t i = 1; i < ti.sectors.size(); i++) {
					if (ti.sectors[i].idam_size != size_code) {
						osd_printf_error("imd_format: cyl %d head %d has mixed sector "
										 "sizes (code %d vs %d); IMD cannot represent this.\n",
										 cyl, hd, size_code, ti.sectors[i].idam_size);
						return false;
					}
				}
				if (size_code >= 8) {
					osd_printf_error("imd_format: cyl %d head %d has out-of-spec sector "
									 "size code %d.\n", cyl, hd, size_code);
				return false;
				}
			}
			int sec_size = 128 << size_code;

			bool need_cmap = false, need_hmap = false;
			for (const auto &s : ti.sectors) {
				if (s.idam_track != out_cyl)      need_cmap = true;
				if (s.idam_head  != uint8_t(hd))  need_hmap = true;
			}
			uint8_t head_byte = uint8_t(uint8_t(hd) & 0x3f)
							  | uint8_t(need_cmap ? 0x80 : 0)
							  | uint8_t(need_hmap ? 0x40 : 0);

			uint8_t recbuf[5] = { mode, out_cyl, head_byte, sec_count, size_code };
			if (auto pr = write_at(io, out_pos, recbuf, 5); pr.first || pr.second != 5) return false;
			out_pos += 5;

			// An empty record has only its 5-byte header.
			if (!sec_count)
				continue;

			std::vector<uint8_t> snum(sec_count);
			for (int i = 0; i < sec_count; i++) snum[i] = ti.sectors[i].idam_sector;
			if (auto pr = write_at(io, out_pos, snum.data(), sec_count); pr.first || pr.second != sec_count) return false;
			out_pos += sec_count;

			if (need_cmap) {
				std::vector<uint8_t> cmap(sec_count);
				for (int i = 0; i < sec_count; i++) cmap[i] = ti.sectors[i].idam_track;
				if (auto pr = write_at(io, out_pos, cmap.data(), sec_count); pr.first || pr.second != sec_count) return false;
				out_pos += sec_count;
			}
			if (need_hmap) {
				std::vector<uint8_t> hmap(sec_count);
				for (int i = 0; i < sec_count; i++) hmap[i] = ti.sectors[i].idam_head;
				if (auto pr = write_at(io, out_pos, hmap.data(), sec_count); pr.first || pr.second != sec_count) return false;
				out_pos += sec_count;
			}

			for (const auto &s : ti.sectors) {
				if (!s.has_data) {
					uint8_t zero = 0;
					if (auto pr = write_at(io, out_pos, &zero, 1); pr.first || pr.second != 1) return false;
					out_pos += 1;
					continue;
				}

				// Compress if every byte is the same value.
				bool compressed = true;
				uint8_t fill = s.data[0];
				for (int i = 1; i < sec_size; i++) {
					if (s.data[i] != fill) { compressed = false; break; }
				}

				// Type byte: 1=normal/good/normal-DAM, +1 if compressed,
				// +2 if deleted DAM, +4 if bad data CRC OR bad addr CRC.
				bool bad = !s.data_crc_ok || !s.addr_crc_ok;
				int t = 1;
				if (compressed)    t += 1;
				if (s.deleted_dam) t += 2;
				if (bad)           t += 4;
				uint8_t type = uint8_t(t);
				if (auto pr = write_at(io, out_pos, &type, 1); pr.first || pr.second != 1) return false;
				out_pos += 1;

				if (compressed) {
					if (auto pr = write_at(io, out_pos, &fill, 1); pr.first || pr.second != 1) return false;
					out_pos += 1;
				} else {
					if (auto pr = write_at(io, out_pos, s.data.data(), sec_size); pr.first || pr.second != size_t(sec_size)) return false;
					out_pos += sec_size;
				}
			}
		}
	}

	return true;
}


const imd_format FLOPPY_IMD_FORMAT;
