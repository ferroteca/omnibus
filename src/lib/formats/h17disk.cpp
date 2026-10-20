// license:BSD-3-Clause
// copyright-holders:Mark Garlanger, Paul Galbraith
/*********************************************************************

Heath H17D disk image format (version 2.0.0)

   Format for Heath hard-sectored 5.25" disk images.

   See https://heathkit.garlanger.com/diskformats/ for more information

*********************************************************************/

#include "h17disk.h"

#include "imageutl.h"

#include "ioprocs.h"

#include <algorithm>
#include <array>
#include <cstring>
#include <vector>


namespace {

using heath_h17::H17_SYNC_BYTE;
using heath_h17::SECTORS_PER_TRACK;
using heath_h17::SECTOR_DATA_SIZE;
using heath_h17::format;
using heath_h17::format_size;
using heath_h17::formats;
using heath_h17::sector_index;

uint32_t get_be32(uint8_t const *data)
{
	return (uint32_t(data[0]) << 24) | (uint32_t(data[1]) << 16) | (uint32_t(data[2]) << 8) | data[3];
}

// version 1 block identifiers
enum {
	V1_DISK_FORMAT = 0x00,
	V1_PARAMETERS  = 0x01,
	V1_LABEL       = 0x02,
	V1_COMMENT     = 0x03,
	V1_DATE        = 0x04,
	V1_IMAGER      = 0x05,
	V1_PROGRAM     = 0x06,
	V1_DATA        = 0x10,
	V1_RAW_DATA    = 0x30,
};

constexpr uint8_t V1_TRACK_SUB_BLOCK  = 0x11;
constexpr uint8_t V1_SECTOR_SUB_BLOCK = 0x12;

// block flag: a reader must understand the block
constexpr uint8_t V1_MANDATORY = 0x80;

// Block header: id, flags, 4 byte length.
// Track sub-block header: id, head, track, 2 byte length.
// Sector sub-block header: id, sector, status, 2 byte length.
constexpr int V1_BLOCK_HEADER_SIZE  = 6;
constexpr int V1_TRACK_HEADER_SIZE  = 5;
constexpr int V1_SECTOR_HEADER_SIZE = 5;

constexpr int V1_FIRST_BLOCK = 7;

// A version 1 block that version 2 also has, so that a save can write it in
// the version 2 file.
struct v1_block
{
	uint8_t id;
	std::vector<uint8_t> data;
};

// Returns the position of the sector header in a sector dump, or end.  The
// dump includes bytes read either side of the sector, so the header is the
// first sync byte whose checksum matches.
size_t find_v1_header(std::vector<uint8_t> const &block, size_t pos, size_t end)
{
	for (; (pos + 5) <= end; pos++)
	{
		if ((block[pos] == H17_SYNC_BYTE) &&
				(heath_h17::h17_checksum(block[pos + 1], block[pos + 2], block[pos + 3]) == block[pos + 4]))
		{
			return pos;
		}
	}

	return end;
}

// Reads the sector dumps in a version 1 data block into img and sectors, and
// the status the imager recorded for each sector into status.  A sector with
// no valid header keeps a zero header, so the controller cannot find it.
bool unpack_v1_sectors(std::vector<uint8_t> const &block, format const &fmt,
		std::vector<uint8_t> &img, std::vector<heath_h17::sector_fields> &sectors, std::vector<uint8_t> &status)
{
	img.assign(format_size(fmt), 0);
	sectors.assign(img.size() / SECTOR_DATA_SIZE, {});
	status.assign(sectors.size(), 0);

	for (size_t i = 0; i < sectors.size(); i++)
	{
		sectors[i].data_sync = 0;
		sectors[i].data      = &img[i * SECTOR_DATA_SIZE];
	}

	size_t pos = 0;

	while ((pos + V1_TRACK_HEADER_SIZE) <= block.size())
	{
		if (block[pos] != V1_TRACK_SUB_BLOCK)
		{
			LOG_FORMATS("H17D v1: expected a track sub-block at %d\n", int(pos));

			return false;
		}

		int const head = block[pos + 1];
		int const track = block[pos + 2];
		size_t const track_length = (size_t(block[pos + 3]) << 8) | block[pos + 4];
		size_t sector_pos = pos + V1_TRACK_HEADER_SIZE;
		size_t const track_end = sector_pos + track_length;

		if ((head >= fmt.head_count) || (track >= fmt.track_count) || (track_end > block.size()))
		{
			LOG_FORMATS("H17D v1: bad track sub-block %d/%d at %d\n", head, track, int(pos));

			return false;
		}

		while ((sector_pos + V1_SECTOR_HEADER_SIZE) <= track_end)
		{
			if (block[sector_pos] != V1_SECTOR_SUB_BLOCK)
			{
				LOG_FORMATS("H17D v1: expected a sector sub-block at %d\n", int(sector_pos));

				return false;
			}

			size_t const dump_length = (size_t(block[sector_pos + 3]) << 8) | block[sector_pos + 4];
			size_t const dump = sector_pos + V1_SECTOR_HEADER_SIZE;
			size_t const dump_end = dump + dump_length;

			if (dump_end > track_end)
			{
				LOG_FORMATS("H17D v1: sector sub-block runs past its track at %d\n", int(sector_pos));

				return false;
			}

			sector_pos = dump_end;

			size_t const header = find_v1_header(block, dump, dump_end);

			if (header == dump_end)
			{
				LOG_FORMATS("H17D v1: no sector header in %d/%d sub-block %d\n", head, track, block[dump - 4]);

				continue;
			}

			int const sector = block[header + 3];

			if (sector >= SECTORS_PER_TRACK)
			{
				LOG_FORMATS("H17D v1: sector %d out of range in %d/%d\n", sector, head, track);

				continue;
			}

			size_t data_sync = header + 5;

			while ((data_sync < dump_end) && (block[data_sync] != H17_SYNC_BYTE))
			{
				data_sync++;
			}

			if ((data_sync + 1 + SECTOR_DATA_SIZE + 1) > dump_end)
			{
				LOG_FORMATS("H17D v1: no sector data in %d/%d/%d\n", head, track, sector);

				continue;
			}

			int const index = sector_index(fmt, head, track, sector);
			heath_h17::sector_fields &fields = sectors[index];
			uint8_t *const data = &img[size_t(index) * SECTOR_DATA_SIZE];

			std::copy(&block[header], &block[header + 5], fields.header.begin());
			std::copy(&block[data_sync + 1], &block[data_sync + 1 + SECTOR_DATA_SIZE], data);
			fields.data_sync     = H17_SYNC_BYTE;
			fields.data_checksum = block[data_sync + 1 + SECTOR_DATA_SIZE];
			status[index]        = block[dump - 3];
		}

		pos = track_end;
	}

	return pos == block.size();
}

// Reads a version 1 file.  blocks gets the blocks that version 2 also has,
// in file order, and status the status byte of each sector.
bool load_v1(util::random_read &io, format &fmt, std::vector<uint8_t> &img, std::vector<heath_h17::sector_fields> &sectors,
		std::vector<v1_block> &blocks, std::vector<uint8_t> &status)
{
	uint64_t file_size;

	if (io.length(file_size))
	{
		return false;
	}

	int head_count = 1;
	int track_count = 40;
	std::vector<uint8_t> data_block;
	bool have_data = false;
	uint64_t pos = V1_FIRST_BLOCK;

	while ((pos + V1_BLOCK_HEADER_SIZE) <= file_size)
	{
		uint8_t header[V1_BLOCK_HEADER_SIZE];
		auto const [err, actual] = read_at(io, pos, header, sizeof(header));

		if (err || (actual != sizeof(header)))
		{
			return false;
		}

		uint8_t const id = header[0];
		uint8_t const flags = header[1];
		uint32_t const length = get_be32(&header[2]);
		uint64_t const data_pos = pos + V1_BLOCK_HEADER_SIZE;
		uint64_t const next_pos = data_pos + length;

		if (next_pos > file_size)
		{
			LOG_FORMATS("H17D v1 block 0x%02x runs past the end of the file\n", id);

			return false;
		}

		switch (id)
		{
		case V1_DISK_FORMAT:
			{
				uint8_t buf[2];

				if (length < sizeof(buf))
				{
					LOG_FORMATS("H17D v1 disk format block too short %d\n", length);

					return false;
				}

				auto const [derr, dactual] = read_at(io, data_pos, buf, sizeof(buf));

				if (derr || (dactual != sizeof(buf)))
				{
					return false;
				}

				head_count = buf[0];
				track_count = buf[1];
			}
			break;

		case V1_DATA:
			{
				data_block.resize(length);
				auto const [derr, dactual] = read_at(io, data_pos, data_block.data(), data_block.size());

				if (derr || (dactual != data_block.size()))
				{
					LOG_FORMATS("unable to read H17D v1 data block\n");

					return false;
				}

				have_data = true;
			}
			break;

		case V1_PARAMETERS:
		case V1_LABEL:
		case V1_COMMENT:
		case V1_DATE:
		case V1_IMAGER:
		case V1_PROGRAM:
			{
				v1_block &kept = blocks.emplace_back(v1_block{ id, std::vector<uint8_t>(length) });
				auto const [derr, dactual] = read_at(io, data_pos, kept.data.data(), kept.data.size());

				if (derr || (dactual != kept.data.size()))
				{
					return false;
				}
			}
			break;

		case V1_RAW_DATA:
			// not needed to build the tracks, and version 2 has no block for it
			break;

		default:
			if (flags & V1_MANDATORY)
			{
				LOG_FORMATS("unknown mandatory H17D v1 block 0x%02x\n", id);

				return false;
			}
			break;
		}

		pos = next_pos;
	}

	if ((pos != file_size) || !have_data)
	{
		LOG_FORMATS("invalid H17D v1 block structure\n");

		return false;
	}

	for (int i = 0; formats[i].head_count; i++)
	{
		if ((formats[i].head_count == head_count) && (formats[i].track_count == track_count))
		{
			fmt = formats[i];

			return unpack_v1_sectors(data_block, fmt, img, sectors, status);
		}
	}

	LOG_FORMATS("invalid H17D v1 disk format - heads: %d, tracks: %d\n", head_count, track_count);

	return false;
}

constexpr int SECTOR_METADATA_SIZE = 16;

struct block_header {
	uint32_t block_name;
	uint32_t length;
};

enum {
	DskF  = 0x466b7344, //!< "DskF", Disk Format
	Parm  = 0x6d726150, //!< "Parm", Parameters
	Date  = 0x65746144, //!< "Date", Date
	Imgr  = 0x72676d49, //!< "Imgr", Imager

	Prog  = 0x676f7250, //!< "Prog", Program (creation)
	Padd  = 0x64646150, //!< "Padd", Padding
	H8DB  = 0x42443848, //!< "H8DB", H8D data block
	SecM  = 0x4d636553, //!< "SecM", Sector Metadata
	Labl  = 0x6c62614c, //!< "Labl", Label
	Comm  = 0x6d6d6f43, //!< "Comm", Comment
};

std::pair<std::uint64_t, std::size_t> find_block(util::random_read &io, uint32_t block_id)
{
	LOG_FORMATS("find_block: 0x%x\n", block_id);

	// start of file
	int          pos    = 0;
	block_header header = { 0, 0 };

	do
	{
		pos += header.length + 8;
		auto const [err, actual] = read_at(io, pos, (void *) &header, 8);
		if (err || actual !=8)
		{
			return std::make_pair(0, 0);
		}
		header.length = swapendian_int32(header.length);
	}
	while (header.block_name != block_id);

	// update position to point to data portion of the block
	return std::make_pair(pos + 8, header.length);
}

// Returns the layout from the DskF block.  The block's optional third byte
// is the R/O flag: 1 means the disk is write protected.
format find_format(util::random_read &io, bool &write_protected)
{
	write_protected = false;

	auto const [pos, length] = find_block(io, DskF);
	if ((pos == 0) || (length < 2) || (length > 3))
	{
		LOG_FORMATS("Can't find valid DskF block %d/%d\n", pos, length);

		return {};
	}

	uint8_t buf[3];

	auto const [err, actual] = read_at(io, pos, buf, length);
	if (err || (actual != length))
	{
		LOG_FORMATS("read error\n");

		return {};
	}

	int head_count  = buf[0];
	int track_count = buf[1];

	write_protected = (length == 3) && (buf[2] == 1);

	for (int i = 0; formats[i].head_count; i++)
	{
		if ((formats[i].head_count == head_count) && (formats[i].track_count == track_count))
		{
			LOG_FORMATS("find_format format found: %d - variant: 0x%x\n", i, formats[i].variant);

			return formats[i];
		}
	}

	LOG_FORMATS("Invalid disk format - heads: %d, tracks: %d\n", head_count, track_count);
	return {};
}

uint32_t get_block_name(uint8_t const *data)
{
	return (uint32_t(data[3]) << 24) | (uint32_t(data[2]) << 16) | (uint32_t(data[1]) << 8) | data[0];
}

void put_be32(uint8_t *data, uint32_t val)
{
	data[0] = val >> 24;
	data[1] = val >> 16;
	data[2] = val >> 8;
	data[3] = val;
}

// Returns the blocks that save() does not build from the tracks, so that it
// can write them back.  They are the file's blocks in file order, header and
// data, except DskF and Padd.  H8DB is kept as a header with length 0, to
// mark which blocks come before the sector data and which come after.  SecM
// is kept whole, so that save() can keep each sector's status byte.
std::vector<uint8_t> keep_blocks(util::random_read &io)
{
	std::vector<uint8_t> kept;
	uint64_t file_size;

	if (io.length(file_size))
	{
		return kept;
	}

	uint64_t pos = 8;

	while ((pos + 8) <= file_size)
	{
		uint8_t header[8];
		auto const [err, actual] = read_at(io, pos, header, sizeof(header));

		if (err || (actual != sizeof(header)))
		{
			break;
		}

		uint32_t const block_name = get_block_name(header);
		uint32_t const length = get_be32(&header[4]);

		if ((pos + 8 + length) > file_size)
		{
			break;
		}

		if (block_name == H8DB)
		{
			put_be32(&header[4], 0);
			kept.insert(kept.end(), std::begin(header), std::end(header));
		}
		else if ((block_name != DskF) && (block_name != Padd))
		{
			size_t const start = kept.size();
			kept.resize(start + sizeof(header) + length);
			std::copy(std::begin(header), std::end(header), &kept[start]);

			auto const [derr, dactual] = read_at(io, pos + 8, &kept[start + sizeof(header)], length);

			if (derr || (dactual != length))
			{
				kept.resize(start);
				break;
			}
		}

		pos += 8 + length;
	}

	return kept;
}

void put_block_name(uint8_t *data, uint32_t val)
{
	data[0] = val;
	data[1] = val >> 8;
	data[2] = val >> 16;
	data[3] = val >> 24;
}

bool write_exact(util::random_read_write &io, uint64_t offset, void const *data, size_t length)
{
	auto const [err, actual] = write_at(io, offset, data, length);

	return !err && (actual == length);
}

bool write_block(util::random_read_write &io, uint64_t &offset, uint32_t block_name, void const *data, uint32_t length)
{
	uint8_t header[8];
	put_block_name(header, block_name);
	put_be32(&header[4], length);

	if (!write_exact(io, offset, header, sizeof(header)))
	{
		return false;
	}

	offset += sizeof(header);

	if (length && !write_exact(io, offset, data, length))
	{
		return false;
	}

	offset += length;
	return true;
}

// Returns the SecM status byte for a version 1 sector status.  Version 1 bits
// 2-7 are version 2 bits 0-5, and the version 1 read error (bit 0) is the
// version 2 "sector was unreadable" (bit 6).  Version 2 has no bit for the
// version 1 "invalid clock bits" (bit 1), so it is dropped.
uint8_t v2_sector_status(uint8_t v1_status)
{
	return (v1_status >> 2) | ((v1_status & 0x01) ? 0x40 : 0x00);
}

void append_block(std::vector<uint8_t> &kept, uint32_t block_name, std::vector<uint8_t> const &data)
{
	size_t const start = kept.size();
	kept.resize(start + 8 + data.size());
	put_block_name(&kept[start], block_name);
	put_be32(&kept[start + 4], uint32_t(data.size()));
	std::copy(data.begin(), data.end(), kept.begin() + start + 8);
}

// Converts the blocks of a version 1 file into the format data that
// heath_h17d_v2_format::load() keeps, so that a save writes them in the
// version 2 file.  Parm, Date, Imgr and Prog go before H8DB, as Applesauce
// writes them.  Labl and Comm go after SecM, because they are often too long
// to fit before H8DB.  Sets write_protected from the R/O flag, which version
// 1 keeps in the parameters block and version 2 in DskF.
std::vector<uint8_t> convert_v1_blocks(std::vector<v1_block> const &blocks,
		std::vector<heath_h17::sector_fields> const &sectors, std::vector<uint8_t> const &status, bool &write_protected)
{
	std::vector<uint8_t> before;
	std::vector<uint8_t> after;

	write_protected = false;

	for (v1_block const &block : blocks)
	{
		std::vector<uint8_t> data = block.data;

		if (block.id == V1_PARAMETERS)
		{
			// Version 1 has R/O, distribution disk and header source.  Bit 7
			// of each is a mandatory flag.  Version 2 has the last two.
			write_protected = !data.empty() && (data[0] & 0x01);
			data.erase(data.begin(), data.begin() + std::min<size_t>(data.size(), 1));
			data.resize(std::min<size_t>(data.size(), 2));

			for (uint8_t &value : data)
			{
				value &= 0x7f;
			}

			append_block(before, Parm, data);
			continue;
		}

		// Version 1 text ends with a zero byte, and version 2 text has no
		// terminator.  The version 1 date is text such as "Sat 01 Aug 2015
		// 04:23:05 PM GMT".  Version 2 asks for ISO 8601, but the date is
		// kept as it is.
		while (!data.empty() && !data.back())
		{
			data.pop_back();
		}

		switch (block.id)
		{
		case V1_DATE:    append_block(before, Date, data); break;
		case V1_IMAGER:  append_block(before, Imgr, data); break;
		case V1_PROGRAM: append_block(before, Prog, data); break;
		case V1_LABEL:   append_block(after,  Labl, data); break;
		case V1_COMMENT: append_block(after,  Comm, data); break;
		}
	}

	// heath_h17d_v2_format::save() keeps the status byte of a sector whose
	// header and checksums match its SecM entry.  A sector that was not found
	// gets the entry that save() would write for it.
	std::vector<uint8_t> secm(sectors.size() * SECTOR_METADATA_SIZE, 0);

	for (size_t index = 0; index < sectors.size(); index++)
	{
		heath_h17::sector_fields const &fields = sectors[index];
		uint8_t *const entry = &secm[index * SECTOR_METADATA_SIZE];

		if (fields.data_sync != H17_SYNC_BYTE)
		{
			entry[4] = 0x40; // sector was unreadable
			continue;
		}

		entry[4] = v2_sector_status(status[index]);
		std::copy(fields.header.begin(), fields.header.end(), &entry[5]);
		entry[10] = fields.data_sync;
		entry[11] = fields.data_checksum;
		entry[12] = 1; // 256 valid bytes
	}

	std::vector<uint8_t> kept = std::move(before);
	uint8_t h8db[8];
	put_block_name(h8db, H8DB);
	put_be32(&h8db[4], 0);
	kept.insert(kept.end(), std::begin(h8db), std::end(h8db));
	append_block(kept, SecM, secm);
	kept.insert(kept.end(), after.begin(), after.end());

	return kept;
}

} // anonymous namespace


heath_h17d_v1_format::heath_h17d_v1_format() : heath_h17_format()
{
}

int heath_h17d_v1_format::identify(util::random_read &io, uint32_t form_factor, const std::vector<uint32_t> &variants) const
{
	uint8_t h[5];
	auto const [err, actual] = read_at(io, 0, h, sizeof(h));

	if (err || (actual != sizeof(h)))
	{
		return 0;
	}

	// version 1 stores the version in binary, not ASCII
	if ((h[0] == 'H') && (h[1] == '1') && (h[2] == '7') && (h[3] == 'D') && (h[4] == 1))
	{
		return FIFID_SIGN;
	}

	return 0;
}

bool heath_h17d_v1_format::load(util::random_read &io, uint32_t form_factor, const std::vector<uint32_t> &variants, floppy_image &image) const
{
	format fmt{};
	std::vector<uint8_t> img;
	std::vector<heath_h17::sector_fields> sectors;
	std::vector<v1_block> blocks;
	std::vector<uint8_t> status;

	if (!load_v1(io, fmt, img, sectors, blocks, status))
	{
		return false;
	}

	bool write_protected;
	image.set_format_data(convert_v1_blocks(blocks, sectors, status, write_protected));
	image.set_write_protected(write_protected);

	return generate_tracks(image, fmt, sectors);
}

const floppy_image_format_t *heath_h17d_v1_format::save_format() const noexcept
{
	// load() stored the version 1 blocks in the version 2 layout
	return &FLOPPY_H17D_V2_FORMAT;
}


heath_h17d_v2_format::heath_h17d_v2_format() : heath_h17_format()
{
}

int heath_h17d_v2_format::identify(util::random_read &io, uint32_t form_factor, const std::vector<uint32_t> &variants) const
{
	uint8_t h[8];
	auto const [err, actual] = read_at(io, 0, h, sizeof(h));

	if (err || (actual != sizeof(h)))
	{
		return 0;
	}

	// Verify the "H17D" signature and a version 2 header: the version as
	// ASCII digits, then 0xff.  heath_h17d_v1_format reads version 1 images.
	if ((h[0] == 'H') && (h[1] == '1') && (h[2] == '7') && (h[3] == 'D') &&
			(h[4] == '2') && (h[5] >= '0') && (h[5] <= '9') && (h[6] >= '0') && (h[6] <= '9') && (h[7] == 0xff))
	{
		return FIFID_SIGN;
	}

	return 0;
}

bool heath_h17d_v2_format::load(util::random_read &io, uint32_t form_factor, const std::vector<uint32_t> &variants, floppy_image &image) const
{
	bool write_protected;
	const format fmt = find_format(io, write_protected);

	if (!fmt.head_count)
	{
		LOG_FORMATS("invalid format\n");

		return false;
	}

	image.set_write_protected(write_protected);
	image.set_format_data(keep_blocks(io));

	auto const [secm_pos, secm_length] = find_block(io, SecM);

	// the sector data, in sector_index() order; sectors points into it
	std::vector<uint8_t> img(format_size(fmt));
	std::vector<heath_h17::sector_fields> sectors(img.size() / SECTOR_DATA_SIZE);

	uint8_t sector_meta_data[SECTOR_METADATA_SIZE];

	for (int head = 0; head < fmt.head_count; head++)
	{
		for (int track = 0; track < fmt.track_count; track++)
		{
			for (int sector = 0; sector < SECTORS_PER_TRACK; sector++)
			{
				int const index = sector_index(fmt, head, track, sector);
				int sect_meta_pos = index * SECTOR_METADATA_SIZE + secm_pos;

				auto const [err, actual] = read_at(io, sect_meta_pos, sector_meta_data, SECTOR_METADATA_SIZE);

				if (err || (actual != SECTOR_METADATA_SIZE))
				{
					LOG_FORMATS("unable to read sect meta data %d/%d/%d\n", head, track, sector);

					return false;
				}
				int data_offset = sector_meta_data[0] << 24 | sector_meta_data[1] << 16 | sector_meta_data[2] << 8 | sector_meta_data[3];

				uint8_t *const sector_data = &img[size_t(index) * SECTOR_DATA_SIZE];
				auto const [err2, actual2] = read_at(io, data_offset, sector_data, SECTOR_DATA_SIZE);

				if (err2 || (actual2 != SECTOR_DATA_SIZE))
				{
					LOG_FORMATS("unable to read sect data %d/%d/%d\n", head, track, sector);

					return false;
				}

				// SecM bytes 5-9 are the header (sync, volume, track, sector,
				// checksum), byte 10 the data sync and byte 11 the data checksum
				heath_h17::sector_fields &fields = sectors[index];

				std::copy(&sector_meta_data[5], &sector_meta_data[10], fields.header.begin());
				fields.data_sync     = sector_meta_data[10];
				fields.data          = sector_data;
				fields.data_checksum = sector_meta_data[11];
			}
		}
	}

	return generate_tracks(image, fmt, sectors);
}

bool heath_h17d_v2_format::save(util::random_read_write &io, const std::vector<uint32_t> &variants, const floppy_image &image) const
{
	format const fmt = heath_h17::find_format(image);

	if (!fmt.head_count)
	{
		LOG_FORMATS("no H17D layout holds this image\n");

		return false;
	}

	// Sort the blocks load() kept into those before H8DB and those after it,
	// and find the old SecM data.  A new image has no kept blocks.
	std::vector<uint8_t> const &kept = image.get_format_data();
	std::vector<uint8_t> before;
	std::vector<uint8_t> after;
	uint8_t const *old_secm = nullptr;
	size_t old_secm_length = 0;
	bool seen_h8db = false;

	for (size_t pos = 0; (pos + 8) <= kept.size(); )
	{
		uint32_t const block_name = get_block_name(&kept[pos]);
		size_t const length = get_be32(&kept[pos + 4]);
		size_t const next = pos + 8 + length;

		if (block_name == H8DB)
		{
			seen_h8db = true;
		}
		else if (block_name == SecM)
		{
			old_secm = &kept[pos + 8];
			old_secm_length = length;
		}
		else
		{
			std::vector<uint8_t> &dest = seen_h8db ? after : before;
			dest.insert(dest.end(), kept.begin() + pos, kept.begin() + next);
		}

		pos = next;
	}

	size_t const sector_count = size_t(fmt.head_count) * fmt.track_count * SECTORS_PER_TRACK;
	std::vector<uint8_t> h8db(sector_count * SECTOR_DATA_SIZE);
	std::vector<uint8_t> secm(sector_count * SECTOR_METADATA_SIZE);

	std::vector<heath_h17::sector_read> const sectors = read_sectors(image, fmt);

	for (size_t index = 0; index < sector_count; index++)
	{
		heath_h17::sector_read const &found = sectors[index];
		std::array<uint8_t, SECTOR_METADATA_SIZE> metadata;

		metadata.fill(0);

		uint32_t const data_offset = 256 + uint32_t(index * SECTOR_DATA_SIZE);
		put_be32(metadata.data(), data_offset);

		// A sector that would not decode is saved as zeros, rather than
		// failing the save.  The file is already truncated when save() runs,
		// so a failed save would leave it empty.
		if (found.found)
		{
			metadata[4]  = found.data_valid ? 0x00 : 0x20; // invalid data checksum
			metadata[5]  = H17_SYNC_BYTE;
			metadata[6]  = found.volume;
			metadata[7]  = found.track;
			metadata[8]  = found.sector;
			metadata[9]  = found.header_checksum;
			metadata[10] = H17_SYNC_BYTE;
			metadata[11] = found.data_checksum;
			metadata[12] = 1;

			std::copy(found.data.begin(), found.data.end(), h8db.begin() + (index * SECTOR_DATA_SIZE));
		}
		else
		{
			metadata[4] = 0x40; // sector was unreadable
		}

		// Byte 4 is the status the imager recorded, and bytes 12-15 the valid
		// byte count and a reserved field.  Keep them when the sector still
		// has the header and checksums it had when it was loaded.
		size_t const old_pos = index * SECTOR_METADATA_SIZE;

		if (old_secm && ((old_pos + SECTOR_METADATA_SIZE) <= old_secm_length)
				&& std::equal(&metadata[5], &metadata[12], &old_secm[old_pos + 5]))
		{
			metadata[4] = old_secm[old_pos + 4];
			std::copy(&old_secm[old_pos + 12], &old_secm[old_pos + 16], &metadata[12]);
		}

		std::copy(metadata.begin(), metadata.end(), secm.begin() + (index * SECTOR_METADATA_SIZE));
	}

	uint8_t const header[8] = { 'H', '1', '7', 'D', '2', '0', '0', 0xff };

	if (!write_exact(io, 0, header, sizeof(header)))
	{
		return false;
	}

	// keep the R/O flag that load() read from the DskF block
	uint64_t offset = sizeof(header);
	uint8_t const dskf[3] = { uint8_t(fmt.head_count), uint8_t(fmt.track_count), uint8_t(image.is_write_protected() ? 1 : 0) };

	if (!write_block(io, offset, DskF, dskf, sizeof(dskf)))
	{
		return false;
	}

	// The H8DB block must start at offset 248, so that its data starts at
	// 256.  The blocks that came before it go back there if they fit with
	// room for a Padd block, or fill the gap exactly.  Otherwise they move
	// after SecM.
	uint64_t const h8db_pos = 256 - 8;
	uint64_t const before_end = offset + before.size();

	if ((before_end != h8db_pos) && ((before_end + 8) > h8db_pos))
	{
		before.insert(before.end(), after.begin(), after.end());
		after = std::move(before);
		before.clear();
	}

	if (!before.empty())
	{
		if (!write_exact(io, offset, before.data(), before.size()))
		{
			return false;
		}

		offset += before.size();
	}

	if (offset != h8db_pos)
	{
		std::vector<uint8_t> padding(h8db_pos - offset - 8, 0);

		if (!write_block(io, offset, Padd, padding.data(), uint32_t(padding.size())))
		{
			return false;
		}
	}

	if (!write_block(io, offset, H8DB, h8db.data(), uint32_t(h8db.size())))
	{
		return false;
	}

	if (!write_block(io, offset, SecM, secm.data(), uint32_t(secm.size())))
	{
		return false;
	}

	return after.empty() || write_exact(io, offset, after.data(), after.size());
}

const heath_h17d_v1_format FLOPPY_H17D_V1_FORMAT;
const heath_h17d_v2_format FLOPPY_H17D_V2_FORMAT;
