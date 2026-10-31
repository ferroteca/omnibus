// license:BSD-3-Clause
// copyright-holders:Paul Galbraith
/***************************************************************************

  Heath/Zenith Z-89-47 8-inch Floppy Disk Interface

    This card connects an H-88, H-89 or Z-90 to the H-47 8" floppy disk
    system.  The WH8-47 does the same job on the H8.  Heath sold the cabinet
    as the H-47 and Zenith sold it as the Z-47.  The card was sold as the
    WH-88-47 (WH-89-47 on some lists) or the Z-89-47.  The Christmas 1980
    catalog lists it at $195 assembled, next to the H-88-7 replacement ROM
    set that the machine needs to boot from it.

    bus/heathzenith/h8x_common/h47_intf_base.cpp has the disk interface, and
    bus/heathzenith/h8x_common/h47.cpp has the H-47 cabinet.  The computer
    sees the same status port, data port, status word and command set as on
    the H8.  So one model serves both machines, and MTR-90 uses the same
    code for both.

    PORT ADDRESSES
    --------------
    The monitors know two disk port blocks, and SW501 says what is in each.
    The Configuration Guide for the H-88, H-89, Z-89 and Z-90 (597-2571-02,
    page 6) gives both settings: "01 Port 174(7CH)/177Q (7FH) is H/Z-47
    disk" and "01 Port 170(78H)/173Q(7BH) is H/Z-47 disk".

        170-173 octal   78-7B   Disk I/O #1, the decoder's /CASS line,
                                which goes to the P504 and P505 slots
        174-177 octal   7C-7F   Disk I/O #2, the decoder's /FLPY line,
                                which goes only to P506

    So a card in P506 answers at 174, and a card in P504 or P505 answers at
    170.  The slot sets the address, not a jumper on the card, because the
    decoder PROM drives a different select line to each slot.  Page 7 of
    the guide agrees: the board "may be installed in either the right or
    left positions in the right expansion area (P506/P512 or P504/P510).
    However, they must be jumpered differently, depending on which of these
    positions are actually used."

    THE PULL-UP AT P506
    -------------------
    Page 7 also tells the owner to fit a pull-up resistor "between pins 1
    and 12 of P512" when the board goes in P506.  The H-17 normally sits in
    P506, and the H-17 write-enables the 1K of floppy RAM on the CPU board.
    A card in its place must hold that line high, or the RAM stays write
    protected and HDOS will not boot.  Heath sold the same kind of resistor
    with the Z-89-37 for the same reason, and MAME models it as the
    we_pullup device.  So this card asserts FMWE when it is in P506.

    The next sentence of the guide shows that the resistor is on the FMWE
    line.  It says the pull-up "is also included on Z-89-67, with a jumper
    connector to enable or disable it, depending on whether the Z-89-67 is
    installed at P506/P512 or P504/P510 (the pullup must be disabled if
    P504/P510 is used)".  The line must be held high only in P506, and FMWE
    is the only P506 line that a disk card would drive.

    An H-89 with this card at 174 has no H-17, because the H-17 also needs
    P506.

    THE I/O DECODE PROM
    -------------------
    The card answers at 170 or 174 only with the 444-61 I/O decode PROM.
    The H-88 and H-89 shipped with the 444-43, which decodes nothing below
    200 octal and puts /CASS on the cassette ports at 370-371.  With the
    444-43, a card in P504 answers at 370, where no software looks for it.
    The H-88-7 replacement ROM set includes the 444-61, which is why the
    catalog says to buy the set with this card.  Choose the 444-61 in the
    machine configuration menu, or with -h89bus:io_decoder 444_61.

    WHAT IS A GUESS
    ---------------
    We have no manual or schematic for this card.  The ports, the status
    word, the master reset bit and the interrupt enable bit come from what
    the software needs and from the WH8-47 manual, and are reliable.  Two
    things are guesses:

      The interrupt level.  The WH8-47 can send its disk interrupt to any
      of INT3 to INT7, and its installation steps use 5.  An H-89's
      right-hand slots carry INT3, INT4 and INT5.  The HDOS System
      Programmer's Guide (595-2553-01, page 8) lists level "5 - Reserved
      for H47 (if implemented)".  So this card defaults to level 5 and also
      offers 3 and 4.  No known software uses the interrupt: the HDOS H47
      driver, the CP/M BIOS and MTR-90 all poll the status port and leave
      the interrupt enable bit clear.

      SW101.  The WH8-47 returns four DIP switches in bits 1 to 4 of the
      status word, and its manual says they are "not presently used".  This
      card has the same status word, so it offers the same switches.

****************************************************************************/

#include "emu.h"

#include "z_89_47.h"

#include "bus/heathzenith/h8x_common/h47_intf_base.h"

#define LOG_SETUP (1U << 1)
#define LOG_ERR   (1U << 2)

#define VERBOSE (0)

#include "logmacro.h"

#define LOGSETUP(...)  LOGMASKED(LOG_SETUP, __VA_ARGS__)
#define LOGERR(...)    LOGMASKED(LOG_ERR, __VA_ARGS__)

#ifdef _MSC_VER
#define FUNCNAME __func__
#else
#define FUNCNAME __PRETTY_FUNCTION__
#endif

namespace {

class z_89_47_device : public heath_h47_intf_base_device
					 , public device_h89bus_right_card_interface
{
public:

	z_89_47_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 0);

protected:

	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;

	virtual void set_interrupt(int state) override;

	required_ioport m_config;

	bool m_installed;
	u8   m_level;
	int  m_intr;
};


z_89_47_device::z_89_47_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: heath_h47_intf_base_device(mconfig, H89BUS_Z_89_47, tag, owner, 0)
	, device_h89bus_right_card_interface(mconfig, *this)
	, m_config(*this, "CONFIG")
{
}

void z_89_47_device::set_interrupt(int state)
{
	m_intr = state;

	switch (m_level)
	{
		case 3: set_slot_int3(state); break;
		case 4: set_slot_int4(state); break;
		case 5: set_slot_int5(state); break;
	}
}

void z_89_47_device::device_start()
{
	heath_h47_intf_base_device::device_start();

	m_installed = false;
	m_level     = 0;
	m_intr      = 0;

	save_item(NAME(m_installed));
	save_item(NAME(m_level));
	save_item(NAME(m_intr));
}

void z_89_47_device::device_reset()
{
	if (!m_installed)
	{
		// /FLPY goes only to P506, so a card in P506 asks for /FLPY with the
		// P506 signals.  In P504 or P505 the disk block is /CASS.  Asking for
		// a line the slot does not carry returns no addresses.
		u8 const select(m_p506_signals ? h89bus::IO_FLPY : h89bus::IO_CASS);

		h89bus::addr_ranges addr_ranges = h89bus().get_address_ranges(select, m_p506_signals);

		if (addr_ranges.size() == 1)
		{
			h89bus::addr_range range = addr_ranges.front();

			LOGSETUP("%s: addr: 0x%02x-0x%02x\n", FUNCNAME, range.first, range.second);

			h89bus().install_io_device(range.first, range.second,
				read8sm_delegate(*this, FUNC(z_89_47_device::read)),
				write8sm_delegate(*this, FUNC(z_89_47_device::write)));
		}
		else
		{
			LOGERR("%s: no address provided for device\n", FUNCNAME);
		}

		m_installed = true;
	}

	m_level = m_config->read() & 0x07;
	m_intr  = 0;

	if (m_p506_signals)
	{
		set_slot_fmwe(ASSERT_LINE);
	}

	set_slot_int3(0);
	set_slot_int4(0);
	set_slot_int5(0);

	heath_h47_intf_base_device::device_reset();
}

static INPUT_PORTS_START( z_89_47_device )

	PORT_START("CONFIG")
	PORT_CONFNAME(0x07, 0x05, "Disk interrupt level")
	PORT_CONFSETTING(   0x00, DEF_STR( None ))
	PORT_CONFSETTING(   0x03, "Level 3")
	PORT_CONFSETTING(   0x04, "Level 4")
	PORT_CONFSETTING(   0x05, "Level 5")

	PORT_START("SW101")
	PORT_DIPNAME( 0x01, 0x00, "SW101-A" )   PORT_DIPLOCATION("SW101:1")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x00, "SW101-B" )   PORT_DIPLOCATION("SW101:2")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x00, "SW101-C" )   PORT_DIPLOCATION("SW101:3")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x00, "SW101-D" )   PORT_DIPLOCATION("SW101:4")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )

INPUT_PORTS_END

ioport_constructor z_89_47_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(z_89_47_device);
}

} // anonymous namespace

DEFINE_DEVICE_TYPE_PRIVATE(H89BUS_Z_89_47, device_h89bus_right_card_interface, z_89_47_device, "h89_z_89_47", "Heath/Zenith Z-89-47 8-inch Floppy Disk Interface");
