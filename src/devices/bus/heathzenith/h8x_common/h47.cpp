// license:BSD-3-Clause
// copyright-holders:Paul Galbraith
/***************************************************************************

  Heath H-47 Dual 8-inch Floppy Disk System

    This device is the H-47 cabinet, not the interface card in the computer.
    The cabinet holds two 8" Remex drives and a controller board.  The head
    of H47DVD in the HDOS 3.02 sources says "DDDVD is the device driver for
    the DD: device, the initial incarnation of which is the Remex H-47".

    The cabinet has its own power supply.  It connects to the computer with
    a 40-wire cable that carries 8 data lines, used in both directions, and
    eight handshake lines.  The computer sends a command byte, then the
    command's parameters, then any data.  It never reads or writes a disk
    controller register.

    On the H-17 and the Z-37, the computer drives the floppy controller chip
    directly, so MAME emulates the chips.  The H-47's controller runs its
    own firmware instead, from a ROM that has not been dumped.  Its part
    number is not known either.  So this file models the command set, not
    the board.  It is written from these sources:

      the WH8-47 card's Operation manual (595-2469, in
      [https://sebhc.github.io/sebhc/documentation/hardware/H8/H8-47_Op.zip])
      for the cable, the handshake and the order of each I/O transaction;

      the H47DEF and H47PAR decks and the H47LIB library of the HDOS 3.02
      sources for the command numbers, the status and auxiliary status bytes,
      the side/unit/sector byte and the disk layout;

      the Heath CP/M 2.2.03 BIOS, which uses the same two ports with its own
      names for the constants, and has the same values for all of them;

      the MTR-90 source for the boot sequence.  The monitor sends a master
      reset, sends DD.RRDY until the drive it wants is ready, reads the
      auxiliary status, loads a sector count of ten, and reads the ten
      sectors with DD.REAB;

      the Heathkit catalogs for the drives and the disks.

    THE CONTROLLER BOARD
    --------------------
    The parts on the controller board are not known.  The 1981 catalog says
    only that "the controller board includes Read-Only Memory (ROM) that
    operates the system and Random Access Memory (RAM) that serves as a
    buffer memory".  No document we have names its processor, its floppy
    controller chip or its parallel interface chip.  This model uses a 179x
    floppy controller because MAME's floppy code reads and writes IBM format
    tracks through one.  The computer cannot see which chip the model uses.

    WHAT THE COMPUTER SEES
    ----------------------
    The card shows three of the cable lines in its status register:

      /BUSY   high when the controller is idle and will take a command.
              The card calls this bit DONE.
      /DTR    high when the controller wants a byte, or has a byte ready.
              The controller sets the direction with DDOUT, which drives the
              card's buffers.  The computer cannot read DDOUT.
      /ERROR  high when the last command failed.  It is valid only after
              the command has finished.  DD.RST returns the cause.

    Page 21 of the manual gives the order of a transaction.  When the
    controller is idle, the first byte written to it is a command.  The
    controller sets BUSY low and asks for each parameter the command takes,
    one byte at a time, with DTR.  Data bytes follow in the same way.  When
    the command finishes, BUSY goes high and the error flag is valid.

    THE DRIVES AND THE DISKS
    ------------------------
    The H-47 specifications in the Fall 1981 catalog give: 8" soft-sectored
    disks in standard IBM 3740 format, FM or MFM, one or two sides, 77
    tracks a side at 48 tpi, 8 to 26 sectors a track with 26 standard, 360
    rpm, 3 ms track to track and 78 ms average seek.  "Disk Capacity:
    Dual-density/dual-sided diskette, up to 1,261,568 bytes per disk
    (depending upon format used); Single-density/single-sided diskette, up to
    256,000 bytes per diskette."

    H47PAR gives HDOS 13 sectors a track in single density and 26 in double
    density.  These are HDOS logical sectors of 256 bytes.  The disk itself
    has 26 sectors of 128 bytes in single density, and 26 sectors of 256
    bytes in double density.  H47LIB converts between the two: its LSC
    routine doubles the sector count in single density "for 128 byte sect.",
    and its SUS routine maps logical sector n to physical sector 2n-1.

    CP/M uses a third layout: eight 1024-byte sectors a track on each of two
    sides.  That layout gives the catalog's largest capacity.  H47DEF sets
    the largest sector size to 1024 bytes for this reason, and has two bits
    in the auxiliary status for the sector size.

    The cabinet has a write-protect switch on the front panel for each drive.
    Its drives also read the write-protect notch the opposite way from a
    5.25" drive.  The HDOS manual warns that "if the write-protect notch is
    covered, the disk is WRITE-ENABLED".  This model ignores both.  A disk is
    write protected when its image is read only.

    WHAT IS NOT MODELLED
    --------------------
    H47DEF defines command 0x04, read address of last sector accessed.  No
    known software sends it.  This model answers it with the first four
    bytes of the last ID field the controller read, because the name of the
    command suggests that.  Command 0x00 (boot), the debug commands at 0x10-0x15 and the
    Heath extensions at 0x80-0x88 have no known caller and no documented
    parameters.  They return an illegal command error.  DD.FRM0 and DD.FRM2
    are the "IBM" format commands.  No document we have says how they differ
    from DD.FRM1 and DD.FRM3, so this model treats them the same way.

****************************************************************************/

#include "emu.h"

#include "h47.h"

#include "formats/flopimg.h"

#define LOG_CMD   (1U << 1)   // commands and their parameters
#define LOG_XFER  (1U << 2)   // data transfers
#define LOG_DRIVE (1U << 3)   // drive and media selection
#define LOG_ERR   (1U << 4)   // errors

#define VERBOSE (0)

#include "logmacro.h"

#define LOGCMD(...)    LOGMASKED(LOG_CMD, __VA_ARGS__)
#define LOGXFER(...)   LOGMASKED(LOG_XFER, __VA_ARGS__)
#define LOGDRIVE(...)  LOGMASKED(LOG_DRIVE, __VA_ARGS__)
#define LOGERR(...)    LOGMASKED(LOG_ERR, __VA_ARGS__)


heath_h47_device::heath_h47_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, HEATH_H47, tag, owner, 0)
	, m_fdc(*this, "fdc")
	, m_floppies(*this, "fdc:%u", 0U)
	, m_done_cb(*this)
	, m_dtr_cb(*this)
	, m_error_cb(*this)
{
}

static void h47_floppies(device_slot_interface &device)
{
	device.option_add("8_ss_sd", FLOPPY_8_SSSD);
	device.option_add("8_ds_sd", FLOPPY_8_DSSD);
	device.option_add("8_ss_dd", FLOPPY_8_SSDD);
	device.option_add("8_ds_dd", FLOPPY_8_DSDD);
}

void heath_h47_device::device_add_mconfig(machine_config &config)
{
	// The real board's floppy controller chip is not known.  The FD1793 has a
	// true data bus.  The FD1791 inverts every register, and the model would
	// only have to invert the values back.
	FD1793(config, m_fdc, FDC_CLOCK);
	m_fdc->intrq_wr_callback().set(FUNC(heath_h47_device::fdc_intrq_w));
	m_fdc->drq_wr_callback().set(FUNC(heath_h47_device::fdc_drq_w));

	// The cabinet holds two drives.  The commands have two bits for the unit
	// number, and a comment in MTR-90 says "4 on H37/47/67".  Units 2 and 3
	// are in a second cabinet, which is chained to the first and has no
	// controller board of its own.  An H-47 was sold with two drives, so
	// units 0 and 1 have drives by default and units 2 and 3 are empty.
	for (int i = 0; i < 4; i++)
	{
		FLOPPY_CONNECTOR(config, m_floppies[i], h47_floppies,
			(i < 2) ? "8_ds_dd" : nullptr,
			floppy_image_device::default_mfm_floppy_formats);
		m_floppies[i]->enable_sound(true);
	}
}

void heath_h47_device::device_start()
{
	std::fill(std::begin(m_buffer), std::end(m_buffer), 0);

	// set_done(), set_dtr() and set_error() call out only when the line
	// changes, so they need a starting value
	m_done  = false;
	m_dtr   = false;
	m_error = false;

	save_item(NAME(m_phase));
	save_item(NAME(m_done));
	save_item(NAME(m_dtr));
	save_item(NAME(m_error));
	save_item(NAME(m_cmd));
	save_item(NAME(m_params));
	save_item(NAME(m_param_count));
	save_item(NAME(m_param_wanted));
	save_item(NAME(m_buffer));
	save_item(NAME(m_buf_pos));
	save_item(NAME(m_buf_len));
	save_item(NAME(m_gap_fill));
	save_item(NAME(m_status));
	save_item(NAME(m_sector_count));
	save_item(NAME(m_unit));
	save_item(NAME(m_side));
	save_item(NAME(m_track));
	save_item(NAME(m_sector));
	save_item(NAME(m_phys_track));
	save_item(NAME(m_last_id));
	save_item(NAME(m_double_density));
	save_item(NAME(m_size_code));
	save_item(NAME(m_step));
	save_item(NAME(m_after_seek));
	save_item(NAME(m_after_id));
	save_item(NAME(m_id_retried));
	save_item(NAME(m_sectors_left));
	save_item(NAME(m_deleted));
	save_item(NAME(m_deleted_seen));
	save_item(NAME(m_format_sectors));
	save_item(NAME(m_copying));
	save_item(NAME(m_src_track));
	save_item(NAME(m_src_sus));
	save_item(NAME(m_dst_track));
	save_item(NAME(m_dst_sus));
}

void heath_h47_device::device_reset()
{
	// 8" drives spin all the time.  The drive interface has no motor line,
	// so start the motors here and never stop them.
	for (auto &connector : m_floppies)
	{
		floppy_image_device *floppy = connector->get_device();

		if (floppy)
		{
			floppy->mon_w(0);
		}
	}

	std::fill(std::begin(m_phys_track), std::end(m_phys_track), 0);
	std::fill(std::begin(m_last_id), std::end(m_last_id), 0);

	// Send all three cable lines to the card, even if they have not changed,
	// so that the card's copies match these.  abort() below sends only the
	// lines that change.
	m_done_cb(0);
	m_dtr_cb(0);
	m_error_cb(0);

	m_done  = false;
	m_dtr   = false;
	m_error = false;

	m_status        = 0;
	m_sector_count  = 1;
	m_unit          = 0;
	m_side          = 0;
	m_track         = 0;
	m_sector        = 1;
	m_double_density = true;
	m_size_code     = 1;

	abort();
}

//-------------------------------------------------
//  the three cable lines
//-------------------------------------------------

void heath_h47_device::set_done(int state)
{
	if (m_done != bool(state))
	{
		m_done = bool(state);
		m_done_cb(state);
	}
}

void heath_h47_device::set_dtr(int state)
{
	if (m_dtr != bool(state))
	{
		m_dtr = bool(state);
		m_dtr_cb(state);
	}
}

void heath_h47_device::set_error(int state)
{
	if (m_error != bool(state))
	{
		m_error = bool(state);
		m_error_cb(state);
	}
}

//-------------------------------------------------
//  master reset - "stop any operation in process,
//  clear all error conditions, raise all master and
//  slave heads, and reset the disk system to an idle
//  state"
//-------------------------------------------------

void heath_h47_device::mrst_w(int state)
{
	if (state)
	{
		LOGCMD("%s: master reset\n", machine().describe_context());

		// Send a force interrupt only when a command is running.  A 179x
		// takes a few clocks to latch a command.  If a force interrupt is
		// still waiting in the latch, the chip drops the next command.  A
		// computer that resets the controller and then sends a command at
		// once would lose that command.
		if (m_step != ST_NONE)
		{
			m_fdc->cmd_w(FDC_FORCE);
		}

		abort();
	}
}

void heath_h47_device::abort()
{
	m_phase        = PH_IDLE;
	m_step         = ST_NONE;
	m_after_seek   = ST_NONE;
	m_after_id     = ST_NONE;
	m_id_retried   = false;
	m_cmd          = 0;
	m_param_count  = 0;
	m_param_wanted = 0;
	m_buf_pos      = 0;
	m_buf_len      = 0;
	m_gap_fill     = 0xff;
	m_sectors_left = 0;
	m_deleted      = false;
	m_deleted_seen = false;
	m_format_sectors = 0;
	m_copying      = false;
	m_src_track    = 0;
	m_src_sus      = 0;
	m_dst_track    = 0;
	m_dst_sus      = 0;

	set_dtr(0);
	set_error(0);
	set_done(1);
}

//-------------------------------------------------
//  the data port
//-------------------------------------------------

u8 heath_h47_device::data_r()
{
	if (m_phase != PH_DATA_OUT)
	{
		// the controller has no byte ready, so the bus floats
		return 0xff;
	}

	u8 const data = m_buffer[m_buf_pos++];

	if (m_buf_pos >= m_buf_len)
	{
		set_dtr(0);

		if (m_sectors_left)
		{
			// read the next sector, and seek first if it is on the next
			// track
			seek_then(ST_READ);
		}
		else
		{
			finish(m_deleted_seen ? SB_DLD : 0);
		}
	}

	return data;
}

void heath_h47_device::data_w(u8 data)
{
	switch (m_phase)
	{
	case PH_IDLE:
		// "The first byte transferred to the subsystem while it is in a
		// system ready state ... is interpreted as a command."
		m_cmd         = data;
		m_param_count = 0;
		set_error(0);
		set_done(0);
		start_command();
		break;

	case PH_PARAMS:
		if (m_param_count < std::size(m_params))
		{
			m_params[m_param_count] = data;
		}
		if (++m_param_count >= m_param_wanted)
		{
			set_dtr(0);
			start_command();
		}
		break;

	case PH_DATA_IN:
		m_buffer[m_buf_pos++] = data;
		if (m_buf_pos >= m_buf_len)
		{
			set_dtr(0);
			start_write();
		}
		break;

	default:
		// the controller did not ask for a byte, so drop it
		LOGERR("%s: data written in phase %d\n", machine().describe_context(), m_phase);
		break;
	}
}

//-------------------------------------------------
//  command dispatch
//-------------------------------------------------

void heath_h47_device::want_params(u8 count)
{
	if (m_param_count >= count)
	{
		return;
	}

	m_param_wanted = count;
	m_phase        = PH_PARAMS;
	set_dtr(1);
}

void heath_h47_device::send_bytes(u32 count)
{
	LOGXFER("%s: %d byte(s) for the host\n", machine().describe_context(), count);

	m_buf_pos = 0;
	m_buf_len = count;
	m_phase   = PH_DATA_OUT;
	set_dtr(1);
}

void heath_h47_device::take_bytes(u32 count)
{
	LOGXFER("%s: %d byte(s) wanted from the host\n", machine().describe_context(), count);

	m_buf_pos = 0;
	m_buf_len = count;
	m_phase   = PH_DATA_IN;
	set_dtr(1);
}

void heath_h47_device::finish(u8 status)
{
	m_status       = status;
	m_step         = ST_NONE;
	m_phase        = PH_IDLE;
	// A read that fails part way leaves sectors uncounted.  Clear the count,
	// or data_r() starts another read after the next command's reply.
	m_sectors_left = 0;
	m_deleted_seen = false;
	m_copying      = false;

	if (status)
	{
		LOGERR("%s: command 0x%02x failed, status 0x%02x\n",
			machine().describe_context(), m_cmd, status);
	}

	set_dtr(0);
	set_error(status ? 1 : 0);
	set_done(1);
}

void heath_h47_device::start_command()
{
	switch (m_cmd)
	{
	case CMD_RST:
		// the status of the command before this one
		m_buffer[0] = m_status;
		LOGCMD("%s: read status -> 0x%02x\n", machine().describe_context(), m_status);
		send_bytes(1);
		m_status = 0;
		break;

	case CMD_RRDY:
		{
			// "Bits 'ON' indicate unit not ready", per MTR-90's RRDY
			u8 not_ready = 0;

			for (int i = 0; i < 4; i++)
			{
				floppy_image_device *floppy = m_floppies[i]->get_device();

				if (!floppy || !floppy->exists() || floppy->ready_r())
				{
					not_ready |= 1 << i;
				}
			}

			LOGCMD("%s: read ready -> 0x%02x\n", machine().describe_context(), not_ready);
			m_buffer[0] = not_ready;
			send_bytes(1);
		}
		break;

	case CMD_LSC:
		// two bytes, high order first
		want_params(2);
		if (m_param_count >= 2)
		{
			m_sector_count = (m_params[0] << 8) | m_params[1];
			LOGCMD("%s: load sector count %d\n", machine().describe_context(), m_sector_count);
			finish(0);
		}
		break;

	case CMD_RAS:
		// one side/unit/sector byte, of which only the unit matters
		want_params(1);
		if (m_param_count >= 1)
		{
			if (!select(m_params[0], false))
			{
				return;
			}

			m_track    = 0;
			m_side     = 0;
			m_after_id = ST_NONE;
			seek_then(ST_READ_ID);
		}
		break;

	case CMD_RAD:
		// No known software sends this command, so its reply is a guess.
		// Return the track, side, sector and length code from the last ID
		// field the FDC read.
		std::copy(std::begin(m_last_id), std::end(m_last_id), std::begin(m_buffer));
		send_bytes(4);
		break;

	case CMD_REA:
	case CMD_REAB:
		want_params(2);
		if (m_param_count >= 2)
		{
			begin_transfer(false, false);
		}
		break;

	case CMD_WRI:
	case CMD_WRIB:
		want_params(2);
		if (m_param_count >= 2)
		{
			begin_transfer(true, false);
		}
		break;

	case CMD_WRD:
	case CMD_WRBD:
		want_params(2);
		if (m_param_count >= 2)
		{
			begin_transfer(true, true);
		}
		break;

	case CMD_FRM0:
	case CMD_FRM1:
	case CMD_FRM2:
	case CMD_FRM3:
		// one byte: side, unit and the sectors wanted on each track
		want_params(1);
		if (m_param_count >= 1)
		{
			begin_format((m_cmd == CMD_FRM2) || (m_cmd == CMD_FRM3), m_params[0] & 0x1f);
		}
		break;

	case CMD_CPY:
		// track, side/unit/sector, track, side/unit/sector - source then
		// destination
		want_params(4);
		if (m_param_count >= 4)
		{
			begin_copy();
		}
		break;

	default:
		LOGERR("%s: unknown command 0x%02x\n", machine().describe_context(), m_cmd);
		finish(SB_ILC);
		break;
	}
}

//-------------------------------------------------
//  drive and media
//-------------------------------------------------

floppy_image_device *heath_h47_device::unit()
{
	return m_floppies[m_unit & 3]->get_device();
}

// Decode a side/unit/sector byte and select that drive on the FDC.  If the
// drive is missing, or is write protected when for_write is set, finish the
// command with an error and return false.
bool heath_h47_device::select(u8 sus, bool for_write)
{
	m_unit   = (sus >> 5) & 3;
	m_side   = BIT(sus, 7);
	m_sector = sus & 0x1f;

	floppy_image_device *floppy = unit();

	LOGDRIVE("%s: unit %d side %d sector %d\n", machine().describe_context(),
		m_unit, m_side, m_sector);

	if (!floppy || !floppy->exists())
	{
		finish(SB_UNR);
		return false;
	}

	if (for_write && floppy->wpt_r())
	{
		finish(SB_WPD);
		return false;
	}

	m_fdc->set_floppy(floppy);
	floppy->ss_w(m_side);

	// Guess the density from the image's variant, so that the first read of
	// an ID field usually works.  The guess can be wrong: a disk formatted
	// again at the other density keeps the variant it was created with.  If
	// the read fails, fdc_intrq_w() tries the other density.
	switch (floppy->get_variant())
	{
	case floppy_image::SSSD:
	case floppy_image::DSSD:
		m_double_density = false;
		break;
	default:
		m_double_density = true;
		break;
	}

	m_id_retried = false;

	return true;
}

// The standard 8" sector counts for each sector size.  The controller's own
// formats and the IBM formats all use these counts.
u8 heath_h47_device::sectors_per_track() const
{
	if (m_double_density)
	{
		static const u8 counts[4] = { 26, 26, 15, 8 };
		return counts[m_size_code & 3];
	}

	static const u8 counts[4] = { 26, 15, 8, 4 };
	return counts[m_size_code & 3];
}

//-------------------------------------------------
//  read and write
//-------------------------------------------------

void heath_h47_device::begin_transfer(bool write, bool deleted)
{
	if (!select(m_params[1], write))
	{
		return;
	}

	m_track        = m_params[0];
	m_deleted      = deleted;
	m_deleted_seen = false;
	m_sectors_left = m_sector_count;

	LOGCMD("%s: %s %d sector(s) from track %d sector %d\n", machine().describe_context(),
		write ? "write" : "read", m_sectors_left, m_track, m_sector);

	if (m_track >= TRACKS)
	{
		finish(SB_BTO);
		return;
	}

	if (!m_sectors_left)
	{
		finish(0);
		return;
	}

	m_after_id = write ? ST_WRITE : ST_READ;
	seek_then(ST_READ_ID);
}

// The copy command copies sectors from one place on the drives to another.
// The data does not pass through the computer.  HDOS also uses it to test
// for write protection: DDWRI1 in H47DVD copies one sector onto itself and
// checks only whether the command failed.  So this function checks the
// destination drive before it copies anything.  A sector count of zero does
// only that check.
void heath_h47_device::begin_copy()
{
	if (!select(m_params[3], true))
	{
		return;
	}

	m_src_track = m_params[0];
	m_src_sus   = m_params[1];
	m_dst_track = m_params[2];
	m_dst_sus   = m_params[3];

	LOGCMD("%s: copy %d sector(s) from track %d sus 0x%02x to track %d sus 0x%02x\n",
		machine().describe_context(), m_sector_count,
		m_src_track, m_src_sus, m_dst_track, m_dst_sus);

	if ((m_src_track >= TRACKS) || (m_dst_track >= TRACKS))
	{
		finish(SB_BTO);
		return;
	}

	m_sectors_left = m_sector_count;

	if (!m_sectors_left)
	{
		finish(0);
		return;
	}

	m_copying = true;
	copy_read();
}

void heath_h47_device::copy_read()
{
	if (!select(m_src_sus, false))
	{
		return;
	}

	m_track    = m_src_track;
	m_after_id = ST_READ;
	seek_then(ST_READ_ID);
}

// Move a side/unit/sector byte to the next sector, going back to sector 1
// after the last one.  The caller moves to the next track when that happens.
u8 heath_h47_device::next_sector(u8 sus, u8 per_track)
{
	u8 sector = sus & 0x1f;

	if (++sector > per_track)
	{
		sector = 1;
	}

	return (sus & 0xe0) | sector;
}

void heath_h47_device::seek_then(u8 next)
{
	m_after_seek = next;

	// /DDEN, so the line is high for single density
	m_fdc->dden_w(!m_double_density);

	floppy_image_device *floppy = unit();

	if (floppy)
	{
		floppy->ss_w(m_side);
	}

	if (m_phys_track[m_unit] == m_track)
	{
		// the head is already on this track, but the FDC's track register
		// still has to be set for this drive
		m_fdc->track_w(m_track);

		switch (next)
		{
		case ST_READ_ID:
			m_step    = ST_READ_ID;
			m_buf_pos = 0;
			m_phase   = PH_EXEC;
			m_fdc->cmd_w(FDC_READ_ID);
			break;

		case ST_READ:
			start_read();
			break;

		case ST_WRITE:
			m_step = ST_NONE;
			take_bytes(sector_size());
			break;

		case ST_FORMAT:
			m_step = ST_FORMAT;
			build_track(m_double_density, m_format_sectors);
			start_write();
			break;

		default:
			m_step = ST_NONE;
			finish(0);
			break;
		}
		return;
	}

	m_fdc->track_w(m_phys_track[m_unit]);
	m_fdc->data_w(m_track);
	m_step  = ST_SEEK;
	m_phase = PH_EXEC;
	m_fdc->cmd_w(FDC_SEEK);
}

void heath_h47_device::start_read()
{
	m_step    = ST_READ;
	m_phase   = PH_EXEC;
	m_buf_pos = 0;
	m_buf_len = sector_size();

	m_fdc->sector_w(m_sector);
	m_fdc->cmd_w(FDC_READ);
}

void heath_h47_device::start_write()
{
	m_phase = PH_EXEC;

	if (m_step == ST_FORMAT)
	{
		m_fdc->cmd_w(FDC_WR_TRK);
		return;
	}

	m_step    = ST_WRITE;
	m_buf_pos = 0;

	m_fdc->sector_w(m_sector);
	m_fdc->cmd_w(m_deleted ? FDC_WRITE_D : FDC_WRITE);
}

u8 heath_h47_device::translate_status(u8 fdc_status) const
{
	if (fdc_status & FS_NOT_RDY)
	{
		return SB_UNR;
	}
	if (fdc_status & FS_PROTECT)
	{
		return SB_WPD;
	}
	if (fdc_status & FS_NOT_FND)
	{
		return SB_NRF;
	}
	if (fdc_status & FS_CRC_ERR)
	{
		return SB_CRC;
	}
	if (fdc_status & FS_LOST)
	{
		return SB_LTD;
	}

	return 0;
}

//-------------------------------------------------
//  formatting
//-------------------------------------------------

void heath_h47_device::begin_format(bool double_density, u8 sectors)
{
	if (!select(m_params[0] & 0xe0, true))
	{
		return;
	}

	if (!sectors)
	{
		finish(SB_ILC);
		return;
	}

	// The format command sets the density.  Sectors are 128 bytes in single
	// density and 256 bytes in double density.  H47INIT uses these sizes.
	m_double_density = double_density;
	m_size_code      = double_density ? 1 : 0;
	m_format_sectors = sectors;
	m_track          = 0;

	LOGCMD("%s: format unit %d side %d, %d %s sectors a track\n",
		machine().describe_context(), m_unit, m_side, sectors,
		double_density ? "double density" : "single density");

	m_after_id = ST_NONE;
	m_step     = ST_FORMAT;
	seek_then(ST_FORMAT);
}

// Lay out one IBM 3740 (FM) or System 34 (MFM) track for the FDC's write
// track command.  0xf5 and 0xf6 become the A1 and C2 marks with their
// missing clocks, and 0xf7 becomes the two CRC bytes.
void heath_h47_device::build_track(bool double_density, u8 sectors)
{
	u8 *p = m_buffer;

	auto fill = [&p] (int count, u8 value)
	{
		for (int i = 0; i < count; i++)
		{
			*p++ = value;
		}
	};

	u16 const size = double_density ? 256 : 128;
	u8 const  code = double_density ? 1 : 0;

	m_gap_fill = double_density ? 0x4e : 0xff;

	// The sector count is five bits of the command's parameter byte, so it
	// can ask for more sectors than fit on a track.  Limit it to what fits,
	// so that the track stays inside m_buffer.  The FDC stops writing at the
	// index hole in either case.
	u16 const per_sector = double_density ? (114 + size) : (58 + size);
	u16 const preamble   = double_density ? 146 : 73;

	sectors = u8(std::min<u32>(sectors, (MAX_TRACK - preamble) / per_sector));

	if (double_density)
	{
		fill(80, 0x4e);                 // gap 4a
		fill(12, 0x00);
		fill(3, 0xf6);
		*p++ = 0xfc;                    // index mark
		fill(50, 0x4e);                 // gap 1
	}
	else
	{
		fill(40, 0xff);                 // gap 4a
		fill(6, 0x00);
		*p++ = 0xfc;                    // index mark
		fill(26, 0xff);                 // gap 1
	}

	for (u8 sector = 1; sector <= sectors; sector++)
	{
		if (double_density)
		{
			fill(12, 0x00);
			fill(3, 0xf5);
			*p++ = 0xfe;                // ID address mark
			*p++ = m_track;
			*p++ = m_side;
			*p++ = sector;
			*p++ = code;
			*p++ = 0xf7;                // ID CRC
			fill(22, 0x4e);             // gap 2
			fill(12, 0x00);
			fill(3, 0xf5);
			*p++ = 0xfb;                // data address mark
			fill(size, 0xe5);
			*p++ = 0xf7;                // data CRC
			fill(54, 0x4e);             // gap 3
		}
		else
		{
			fill(6, 0x00);
			*p++ = 0xfe;                // ID address mark
			*p++ = m_track;
			*p++ = m_side;
			*p++ = sector;
			*p++ = code;
			*p++ = 0xf7;                // ID CRC
			fill(11, 0xff);             // gap 2
			fill(6, 0x00);
			*p++ = 0xfb;                // data address mark
			fill(size, 0xe5);
			*p++ = 0xf7;                // data CRC
			fill(27, 0xff);             // gap 3
		}
	}

	m_buf_pos = 0;
	m_buf_len = p - m_buffer;

	// The rest of the track is gap 4b.  The FDC stops taking bytes at the
	// index hole, not at the end of this buffer, so fdc_drq_w() sends
	// m_gap_fill until then.
}

//-------------------------------------------------
//  the floppy sequencer
//-------------------------------------------------

void heath_h47_device::fdc_drq_w(int state)
{
	if (!state)
	{
		return;
	}

	switch (m_step)
	{
	case ST_READ_ID:
		{
			u8 const data = m_fdc->data_r();

			// track, side, sector, length code, then the two CRC bytes
			if (m_buf_pos < std::size(m_last_id))
			{
				m_last_id[m_buf_pos] = data;
			}
			m_buf_pos++;
		}
		break;

	case ST_READ:
		if (m_buf_pos < m_buf_len)
		{
			m_buffer[m_buf_pos++] = m_fdc->data_r();
		}
		else
		{
			(void)m_fdc->data_r();
		}
		break;

	case ST_WRITE:
	case ST_FORMAT:
		m_fdc->data_w((m_buf_pos < m_buf_len) ? m_buffer[m_buf_pos++] : m_gap_fill);
		break;

	default:
		break;
	}
}

void heath_h47_device::fdc_intrq_w(int state)
{
	if (!state)
	{
		return;
	}

	u8 const fdc_status = m_fdc->status_r();
	u8 const step       = m_step;

	LOGDRIVE("%s: intrq in step %d, fdc status 0x%02x\n", machine().describe_context(), step, fdc_status);

	m_step = ST_NONE;

	switch (step)
	{
	case ST_SEEK:
		if (fdc_status & FS_NOT_RDY)
		{
			finish(SB_UNR);
			return;
		}

		m_phys_track[m_unit] = m_track;

		switch (m_after_seek)
		{
		case ST_READ_ID:
			m_step    = ST_READ_ID;
			m_buf_pos = 0;
			m_fdc->cmd_w(FDC_READ_ID);
			break;

		case ST_READ:
			start_read();
			break;

		case ST_WRITE:
			take_bytes(sector_size());
			break;

		case ST_FORMAT:
			m_step = ST_FORMAT;
			build_track(m_double_density, m_format_sectors);
			start_write();
			break;

		default:
			finish(0);
			break;
		}
		return;

	case ST_READ_ID:
		if (fdc_status & (FS_NOT_FND | FS_CRC_ERR | FS_NOT_RDY))
		{
			if (!(fdc_status & FS_NOT_RDY) && !m_id_retried)
			{
				// The density guess was wrong, or this side has never been
				// formatted.  Try the other density once.
				m_id_retried     = true;
				m_double_density = !m_double_density;
				m_fdc->dden_w(!m_double_density);
				m_step    = ST_READ_ID;
				m_buf_pos = 0;
				m_fdc->cmd_w(FDC_READ_ID);
				return;
			}

			finish(translate_status(fdc_status));
			return;
		}

		m_size_code = m_last_id[3] & 3;

		LOGDRIVE("%s: track %d is %s, %d byte sectors\n", machine().describe_context(),
			m_track, m_double_density ? "double density" : "single density", sector_size());

		switch (m_after_id)
		{
		case ST_READ:
			start_read();
			break;

		case ST_WRITE:
			take_bytes(sector_size());
			break;

		case ST_COPY:
			// the sector is already in m_buffer, so write it without
			// asking the computer for data
			m_buf_pos = 0;
			m_buf_len = sector_size();
			start_write();
			break;

		default:
			// CMD_RAS - report the density, sector size and sides of the disk
			{
				floppy_image_device *floppy = unit();
				u8 aux = m_size_code & AS_SLM;

				if (m_double_density)
				{
					aux |= AS_0DD | AS_1DD;
				}
				if (floppy && (floppy->get_sides() > 1))
				{
					aux |= AS_S1A;
				}

				LOGCMD("%s: auxiliary status -> 0x%02x\n", machine().describe_context(), aux);
				m_buffer[0] = aux;
				send_bytes(1);
			}
			break;
		}
		return;

	case ST_READ:
		{
			u8 const status = translate_status(fdc_status);

			if (status)
			{
				finish(status);
				return;
			}

			if (m_copying)
			{
				// write the sector to the copy's destination; the computer
				// does not see it
				if (!select(m_dst_sus, true))
				{
					return;
				}

				m_track    = m_dst_track;
				m_after_id = ST_COPY;
				seek_then(ST_READ_ID);
				return;
			}

			m_sectors_left--;

			if (m_sectors_left)
			{
				// the address of the next sector to read, after the
				// computer has taken this one
				if (++m_sector > sectors_per_track())
				{
					m_sector = 1;
					m_track++;
				}
			}

			// Send the data from a sector with a deleted data mark too.
			// finish() reports the mark when the transfer ends.
			if (fdc_status & FS_DELETED)
			{
				m_deleted_seen = true;
			}

			m_buf_pos = 0;
			m_phase   = PH_DATA_OUT;
			set_dtr(1);
		}
		return;

	case ST_WRITE:
		{
			u8 const status = translate_status(fdc_status);

			if (status)
			{
				finish(status);
				return;
			}

			m_sectors_left--;

			if (!m_sectors_left)
			{
				finish(0);
				return;
			}

			if (m_copying)
			{
				u8 const per_track = sectors_per_track();

				if ((m_src_sus & 0x1f) >= per_track)
				{
					m_src_track++;
				}
				if ((m_dst_sus & 0x1f) >= per_track)
				{
					m_dst_track++;
				}

				m_src_sus = next_sector(m_src_sus, per_track);
				m_dst_sus = next_sector(m_dst_sus, per_track);

				if ((m_src_track >= TRACKS) || (m_dst_track >= TRACKS))
				{
					finish(SB_BTO);
					return;
				}

				copy_read();
				return;
			}

			if (++m_sector > sectors_per_track())
			{
				m_sector = 1;

				if (++m_track >= TRACKS)
				{
					finish(SB_BTO);
					return;
				}

				m_after_seek = ST_WRITE;
				seek_then(ST_WRITE);
				return;
			}

			take_bytes(sector_size());
		}
		return;

	case ST_FORMAT:
		{
			u8 const status = translate_status(fdc_status);

			if (status)
			{
				finish(status);
				return;
			}

			if (++m_track >= TRACKS)
			{
				LOGCMD("%s: format complete\n", machine().describe_context());
				finish(0);
				return;
			}

			m_step = ST_FORMAT;
			seek_then(ST_FORMAT);
		}
		return;

	default:
		break;
	}
}

DEFINE_DEVICE_TYPE(HEATH_H47, heath_h47_device, "heath_h47", "Heath H-47 Dual 8-inch Floppy Disk System");
