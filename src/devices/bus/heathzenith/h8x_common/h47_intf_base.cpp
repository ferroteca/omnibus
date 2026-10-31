// license:BSD-3-Clause
// copyright-holders:Paul Galbraith
/***************************************************************************

  Heath H-47 interface, the part that is the same on both buses

    The interface has two ports and one interrupt.  This file is written
    from the "Disk I/O Modes", "Disk I/O Control Logic" and "Read/Write Port
    Decoder" sections of the WH8-47 card's Operation manual (595-2469, pages
    22 and 23, in
    [https://sebhc.github.io/sebhc/documentation/hardware/H8/H8-47_Op.zip]).
    The constant names are Heath's own, from the H47DEF deck of the HDOS
    3.02 sources.

    "When the disk system address is jumper programmed, it will respond to at
    least two consecutive I/O addresses; that is, 170 and 171.  The first
    address is considered the STATUS post address and the second is the DATA
    port address.  Reading and writing can be performed on both addresses."
    The truth table on page 22 decodes only A0.  Its four rows are write
    status, write data, read status and read data.  So a card set for a block
    of four ports answers at 170 and 171, and again at 172 and 173.

    READ STATUS
        D0  /ERR         error condition from disk system
        D1  SW101-A  |
        D2  SW101-B  |   disk I/O status, switch SW101
        D3  SW101-C  |   (not presently used)
        D4  SW101-D  |
        D5  /DONE        disk system waiting for command
        D6  /INT ENABLE  disk interrupt has been enabled
        D7  /TR          disk system requesting information transfer

    WRITE STATUS
        D6  interrupts enabled
        D1  master reset
        D0  flip-flop U137B set

    The manual does not say what U137B does.  This model stores D0 and does
    nothing else with it.  HDOS, CP/M and MTR-90 do not set it.

    The interrupt latch is U141A.  A hardware reset clears it, and so does
    any read or write of the data port: the manual says "Also, any interrupt
    that has occurred is cleared" for both.  The manual does not say what
    sets the latch.  It lists IDSK as one of the three interrupt sources
    wired to the jumper block.  This model sets the latch when the
    controller asks for a byte (DTR goes high) and when a command finishes
    (DONE goes high).  That is a guess, and no known software tests it:
    the HDOS driver and MTR-90 both poll the status port and leave D6 clear.

****************************************************************************/

#include "emu.h"

#include "h47_intf_base.h"

#define LOG_REG (1U << 1)   // port accesses
#define LOG_INT (1U << 2)   // interrupt state

#define VERBOSE (0)

#include "logmacro.h"

#define LOGREG(...)  LOGMASKED(LOG_REG, __VA_ARGS__)
#define LOGINT(...)  LOGMASKED(LOG_INT, __VA_ARGS__)


heath_h47_intf_base_device::heath_h47_intf_base_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, type, tag, owner, clock)
	, m_h47(*this, "h47")
	, m_sw101(*this, "SW101")
{
}

void heath_h47_intf_base_device::device_add_mconfig(machine_config &config)
{
	HEATH_H47(config, m_h47, 0);
	m_h47->done_cb().set(FUNC(heath_h47_intf_base_device::done_w));
	m_h47->dtr_cb().set(FUNC(heath_h47_intf_base_device::dtr_w));
	m_h47->error_cb().set(FUNC(heath_h47_intf_base_device::error_w));
}

void heath_h47_intf_base_device::device_start()
{
	// The cabinet's callbacks fire only when a line changes.  Start these
	// copies at the same values the cabinet starts with.  Otherwise the
	// status register would read uninitialized memory until each line
	// changed.
	m_done        = false;
	m_dtr         = false;
	m_error       = false;
	m_int_enabled = false;
	m_int_latched = false;
	m_u137b       = false;

	save_item(NAME(m_done));
	save_item(NAME(m_dtr));
	save_item(NAME(m_error));
	save_item(NAME(m_int_enabled));
	save_item(NAME(m_int_latched));
	save_item(NAME(m_u137b));
}

void heath_h47_intf_base_device::device_reset()
{
	m_int_enabled = false;
	m_int_latched = false;
	m_u137b       = false;

	set_interrupt(0);
}

void heath_h47_intf_base_device::device_reset_after_children()
{
	// "The H8 system can reset the disk I/O and disk by a hardware reset.
	// This reset is from the H8 bus (pin 29)."  Send it here, after the
	// cabinet's own device_reset().  This device's device_reset() runs
	// before the cabinet's.
	m_h47->mrst_w(1);
	m_h47->mrst_w(0);
}

void heath_h47_intf_base_device::done_w(int state)
{
	m_done = bool(state);

	if (state)
	{
		m_int_latched = true;
		update_interrupt();
	}
}

void heath_h47_intf_base_device::dtr_w(int state)
{
	m_dtr = bool(state);

	if (state)
	{
		m_int_latched = true;
		update_interrupt();
	}
}

void heath_h47_intf_base_device::error_w(int state)
{
	m_error = bool(state);
}

void heath_h47_intf_base_device::update_interrupt()
{
	int const state = (m_int_enabled && m_int_latched) ? 1 : 0;

	LOGINT("%s: interrupt %d (enabled %d latched %d)\n", machine().describe_context(),
		state, m_int_enabled, m_int_latched);

	set_interrupt(state);
}

u8 heath_h47_intf_base_device::read(offs_t offset)
{
	u8 value;

	if (BIT(offset, 0))
	{
		// Reading the data port takes a byte from the controller and moves
		// the transfer on, so the debugger must not read it.
		if (machine().side_effects_disabled())
		{
			return 0xff;
		}

		// Clear the latch before taking the byte.  The controller may ask for
		// the next byte at once, and that request sets the latch again.
		m_int_latched = false;
		update_interrupt();

		value = m_h47->data_r();
	}
	else
	{
		value = (m_sw101->read() << 1) & (S_SW0 | S_SW1 | S_SW2 | S_SW3);

		if (m_error)
		{
			value |= S_ERR;
		}
		if (m_done)
		{
			value |= S_DON;
		}
		if (m_int_enabled)
		{
			value |= S_IEN;
		}
		if (m_dtr)
		{
			value |= S_DTR;
		}
	}

	LOGREG("%s: read %s -> 0x%02x\n", machine().describe_context(),
		BIT(offset, 0) ? "data" : "status", value);

	return value;
}

void heath_h47_intf_base_device::write(offs_t offset, u8 data)
{
	LOGREG("%s: write %s <- 0x%02x\n", machine().describe_context(),
		BIT(offset, 0) ? "data" : "status", data);

	if (BIT(offset, 0))
	{
		m_int_latched = false;
		update_interrupt();

		m_h47->data_w(data);
	}
	else
	{
		m_u137b       = bool(data & W_U137B);
		m_int_enabled = bool(data & W_IEN);

		if (data & W_RES)
		{
			m_int_latched = false;
			m_h47->mrst_w(1);
			m_h47->mrst_w(0);
		}

		update_interrupt();
	}
}
