// license:BSD-3-Clause
// copyright-holders:Paul Galbraith
/***************************************************************************

  Heath H47 Interface and Serial I/O Card

    Model number WH8-47.  This card connects an H8 to the H-47 8" floppy
    disk system.  It also has two RS-232C serial channels, which do not
    depend on the disk interface: "In addition to the disk interface
    circuitry, this Card contains two channels of RS232C asynchronous serial
    interface for interfacing to such devices as the Heath H19 Terminal, H14
    Printer, or a modem."  The quotations and jumper names in this file are
    from the card's Operation manual (595-2469, in
    [https://sebhc.github.io/sebhc/documentation/hardware/H8/H8-47_Op.zip]).

    bus/heathzenith/h8x_common/h47_intf_base.cpp has the disk interface, and
    bus/heathzenith/h8x_common/h47.cpp has the H-47 cabinet.  This file sets
    the card's port addresses and connects its interrupts.

    THE DISK ADDRESS
    ----------------
    "These jumpers select ports 170 through 173 for the H47 disk system."
    Pictorial 3 shows this setting, and Heath's software expects it.  The
    jumpers set the first two octal digits of the address.  A pair of ranges,
    XX0-XX3 or XX4-XX7, sets the third.  So the card takes a block of four
    ports, and its two ports appear twice in the block.  The monitors call
    the two blocks Disk I/O #1 (170) and Disk I/O #2 (174).  The H-89 uses
    the same two blocks.  This card offers those two addresses.  The H8's
    configuration switch calls them the "Port 170 device" and the "Port 174
    device".

    The DISK ENABLE jumper turns off the whole disk interface.  Use it when
    the card is fitted only for its two serial channels.

    THE SERIAL ADDRESSES
    --------------------
    "Any address may be assigned to any channel, but two channels must not be
    assigned the same address."  This card offers the six addresses in the
    manual's table of I/O address assignments:

        300-307 octal   alternate terminal 0
        310-317 octal   alternate terminal 1
        320-327 octal   alternate terminal 2, HDOS AT:
        330-337 octal   alternate terminal 3
        340-347 octal   line printer, HDOS LP:
        350-357 octal   console terminal, HDOS TT:

    Both channels are off by default, for the same reason as port 4 on the
    H-8-4.  HDOS moves its console to a serial port it finds at 350 octal.
    If nothing is attached to that port, the H8 shows no output.  When you
    turn a channel on, attach a terminal to it.

    THE INTERRUPTS
    --------------
    The card has three interrupt sources: the disk interface and the two
    serial channels.  The manual says "the interrupt signal from the disk
    I/O or either serial I/O channel can be connected to any H8 bus
    interrupt line (INT3, 4, 5, 6, or 7) by jumper plugs".  The installation
    steps say "Set DISK INTERRUPT jumper J104 to 5", so level 5 is the
    default here.  No known software uses the disk interrupt: the HDOS H47
    driver and MTR-90 both poll the status port and leave the interrupt
    enable bit clear.

    THE MONITOR DISABLE
    -------------------
    The card can turn off the H8's monitor ROM: "This is done by setting D5
    to a logic 1 and outputting it to I/O port 362 ... which latches the D5
    bit into flip-flop U105B.  This latched D5 bit is inverted by an open
    collector gate (U106B) and applied to pin 46 of the H8 bus."  The HA-8-8
    and the HA-8-6 do the same thing with the same bit of port 362.  Pin 46
    is wire-ORed across all the slots, so on a real H8 each of these cards
    has the same effect.

    This card installs only a write handler for port 362.  Reads of port 362
    return SW1 on the configuration card or CPU card.  If this card took the
    read handler, XCON8 would read every switch as off.  In a machine with
    two cards that write port 362, only one card's latch drives the line.
    Software cannot see the difference, because both cards drive the same
    line in the same way.

    WHAT IS NOT MODELLED
    --------------------
    The serial channels cannot be set for 20 mA current loop.  The card
    brings out a DCE connector and a DTE connector for each channel; this
    model gives each channel one RS-232 port.

****************************************************************************/

#include "emu.h"

#include "wh_8_47.h"

#include "bus/heathzenith/h8x_common/h47_intf_base.h"
#include "bus/rs232/rs232.h"
#include "machine/ins8250.h"

namespace {

class wh_8_47_device : public heath_h47_intf_base_device
					 , public device_h8bus_card_interface
{
public:

	wh_8_47_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 0);

protected:

	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;

	virtual void map_io(address_space_installer &space) override ATTR_COLD;

	virtual void set_interrupt(int state) override;

	void read_jumpers();
	void uart_int_w(int index, int state);
	void update_int(u8 level);
	void portf2_w(u8 data);

	required_device_array<ins8250_device, 2> m_uart;
	required_ioport_array<2>                 m_serial_jumpers;
	required_ioport                          m_disk_jumpers;

	// base address of the four port disk block, 0 with DISK ENABLE out
	u8  m_disk_base;

	// base address each serial channel answers at, 0 with CHAN ENABLE off
	u8  m_serial_base[2];

	// bus interrupt level each source is wired to, 0 when no wire is fitted
	u8  m_disk_level;
	u8  m_serial_level[2];

	int m_disk_intr;
	int m_serial_intr[2];

	u8  m_gpp;
};


wh_8_47_device::wh_8_47_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: heath_h47_intf_base_device(mconfig, H8BUS_WH_8_47, tag, owner, 0)
	, device_h8bus_card_interface(mconfig, *this)
	, m_uart(*this, "uart%u", 0U)
	, m_serial_jumpers(*this, "JUMPERS%u", 0U)
	, m_disk_jumpers(*this, "DISK")
{
}

void wh_8_47_device::read_jumpers()
{
	// in the order of the settings below.  The extra entries keep a value
	// from a hand-edited cfg file inside the array.
	static constexpr u8 DISK_BASE[4]   = { 0x00, 0x78, 0x7c, 0x00 };
	static constexpr u8 SERIAL_BASE[8] = { 0x00, 0xc0, 0xc8, 0xd0, 0xd8, 0xe0, 0xe8, 0x00 };

	ioport_value const disk(m_disk_jumpers->read());

	m_disk_base  = DISK_BASE[disk & 0x03];
	m_disk_level = (disk >> 2) & 0x07;

	for (int i = 0; i < 2; i++)
	{
		ioport_value const jumpers(m_serial_jumpers[i]->read());

		m_serial_base[i]  = SERIAL_BASE[jumpers & 0x07];
		m_serial_level[i] = (jumpers >> 3) & 0x07;
	}
}

void wh_8_47_device::set_interrupt(int state)
{
	m_disk_intr = state;

	update_int(m_disk_level);
}

void wh_8_47_device::uart_int_w(int index, int state)
{
	m_serial_intr[index] = state;

	update_int(m_serial_level[index]);
}

void wh_8_47_device::update_int(u8 level)
{
	// no wire fitted to this source's interrupt holes
	if (level == 0)
	{
		return;
	}

	// Sources set to the same level share one bus line.  The line stays
	// asserted while any of them asserts it.
	int state = 0;

	if (m_disk_base && (m_disk_level == level))
	{
		state |= m_disk_intr;
	}

	for (int i = 0; i < 2; i++)
	{
		if (m_serial_base[i] && (m_serial_level[i] == level))
		{
			state |= m_serial_intr[i];
		}
	}

	switch (level)
	{
		case 3: set_slot_int3(state); break;
		case 4: set_slot_int4(state); break;
		case 5: set_slot_int5(state); break;
		case 6: set_slot_int6(state); break;
		case 7: set_slot_int7(state); break;
	}
}

// "The Monitor ROM is reenabled by either outputting a D5 logic 0 to I/O port
// 362 or by a system RESET."
void wh_8_47_device::portf2_w(u8 data)
{
	if (BIT(data ^ m_gpp, 5))
	{
		set_slot_rom_disable(BIT(data, 5));
	}

	m_gpp = data;
}

void wh_8_47_device::device_start()
{
	heath_h47_intf_base_device::device_start();

	m_disk_base  = 0;
	m_disk_level = 0;
	m_disk_intr  = 0;
	m_gpp        = 0;

	for (int i = 0; i < 2; i++)
	{
		m_serial_base[i]  = 0;
		m_serial_level[i] = 0;
		m_serial_intr[i]  = 0;
	}

	save_item(NAME(m_disk_base));
	save_item(NAME(m_serial_base));
	save_item(NAME(m_disk_level));
	save_item(NAME(m_serial_level));
	save_item(NAME(m_disk_intr));
	save_item(NAME(m_serial_intr));
	save_item(NAME(m_gpp));
}

void wh_8_47_device::device_reset()
{
	// map_io() reads the jumpers, and the CPU card runs it from its own reset.
	// Nothing orders the two resets.  Reading the jumpers here as well would
	// stop map_io() from seeing that an address had moved.
	m_disk_intr = 0;
	m_gpp       = 0;

	for (int i = 0; i < 2; i++)
	{
		m_serial_intr[i] = 0;
	}

	for (u8 level = 3; level <= 7; level++)
	{
		update_int(level);
	}

	heath_h47_intf_base_device::device_reset();
}

void wh_8_47_device::map_io(address_space_installer &space)
{
	// The bus maps its cards from the CPU card's reset, so this runs on every
	// reset.  It reads the jumpers again, so an address changed in the machine
	// configuration menu takes effect at the next reset.  A port block that
	// moved gives up its old window.  This only unmaps windows this card
	// installed.
	u8 const previous_disk(m_disk_base);
	u8 const previous_serial[2] = { m_serial_base[0], m_serial_base[1] };

	read_jumpers();

	// Remove every old window before installing any new one.  Otherwise, when
	// two channels swap addresses, the second channel's unmap would remove the
	// window the first channel just installed.
	if (previous_disk && (previous_disk != m_disk_base))
	{
		space.unmap_readwrite(previous_disk, previous_disk + 3);
	}

	for (int i = 0; i < 2; i++)
	{
		if (previous_serial[i] && (previous_serial[i] != m_serial_base[i]))
		{
			space.unmap_readwrite(previous_serial[i], previous_serial[i] + 7);
		}
	}

	if (m_disk_base)
	{
		space.install_readwrite_handler(m_disk_base, m_disk_base + 3,
			read8sm_delegate(*this, FUNC(wh_8_47_device::read)),
			write8sm_delegate(*this, FUNC(wh_8_47_device::write))
		);
	}

	for (int i = 0; i < 2; i++)
	{
		if (m_serial_base[i])
		{
			space.install_readwrite_handler(m_serial_base[i], m_serial_base[i] + 7,
				read8sm_delegate(m_uart[i], FUNC(ins8250_device::ins8250_r)),
				write8sm_delegate(m_uart[i], FUNC(ins8250_device::ins8250_w))
			);
		}
	}

	// write only, so that reads still return SW1 from the card that has it
	space.install_write_handler(0xf2, 0xf2,
		write8smo_delegate(*this, FUNC(wh_8_47_device::portf2_w))
	);
}

void wh_8_47_device::device_add_mconfig(machine_config &config)
{
	heath_h47_intf_base_device::device_add_mconfig(config);

	static char const *const RS232_TAG[2] = { "rs232_0", "rs232_1" };

	for (int i = 0; i < 2; i++)
	{
		// "The 1.843 MHz crystal oscillator serves as the clock for the baud
		// rate generators of the ACE's."
		INS8250(config, m_uart[i], XTAL(1'843'200));

		m_uart[i]->out_tx_callback().set(RS232_TAG[i], FUNC(rs232_port_device::write_txd));
		m_uart[i]->out_dtr_callback().set(RS232_TAG[i], FUNC(rs232_port_device::write_dtr));
		m_uart[i]->out_rts_callback().set(RS232_TAG[i], FUNC(rs232_port_device::write_rts));

		// The JUMPERS ports set which bus level this goes to, if any; see
		// update_int().
		m_uart[i]->out_int_callback().set([this, i] (int state) { uart_int_w(i, state); });

		rs232_port_device &port(RS232_PORT(config, RS232_TAG[i], default_rs232_devices, nullptr));

		port.rxd_handler().set(m_uart[i], FUNC(ins8250_uart_device::rx_w));
		port.dcd_handler().set(m_uart[i], FUNC(ins8250_uart_device::dcd_w));
		port.dsr_handler().set(m_uart[i], FUNC(ins8250_uart_device::dsr_w));
		port.cts_handler().set(m_uart[i], FUNC(ins8250_uart_device::cts_w));
		port.ri_handler().set(m_uart[i], FUNC(ins8250_uart_device::ri_w));
	}
}

static INPUT_PORTS_START( wh_8_47_jumpers )

	PORT_START("DISK")
	PORT_CONFNAME(0x03, 0x01, "Disk address")
	PORT_CONFSETTING(   0x00, DEF_STR( None ))
	PORT_CONFSETTING(   0x01, "170-173 octal")
	PORT_CONFSETTING(   0x02, "174-177 octal")
	PORT_CONFNAME(0x1c, 0x14, "Disk interrupt")
	PORT_CONFSETTING(   0x00, DEF_STR( None ))
	PORT_CONFSETTING(   0x0c, "Level 3")
	PORT_CONFSETTING(   0x10, "Level 4")
	PORT_CONFSETTING(   0x14, "Level 5")
	PORT_CONFSETTING(   0x18, "Level 6")
	PORT_CONFSETTING(   0x1c, "Level 7")

	// "Disk I/O status, switch SW101 (not presently used)".  The status port
	// returns these four switches in bits 1 to 4.  No known software reads
	// them.
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

	PORT_START("JUMPERS0")
	PORT_CONFNAME(0x07, 0x00, "Channel 0 address")
	PORT_CONFSETTING(   0x00, DEF_STR( None ))
	PORT_CONFSETTING(   0x01, "300-307 octal")
	PORT_CONFSETTING(   0x02, "310-317 octal")
	PORT_CONFSETTING(   0x03, "320-327 octal")
	PORT_CONFSETTING(   0x04, "330-337 octal")
	PORT_CONFSETTING(   0x05, "340-347 octal")
	PORT_CONFSETTING(   0x06, "350-357 octal")
	PORT_CONFNAME(0x38, 0x00, "Channel 0 interrupt")
	PORT_CONFSETTING(   0x00, DEF_STR( None ))
	PORT_CONFSETTING(   0x18, "Level 3")
	PORT_CONFSETTING(   0x20, "Level 4")
	PORT_CONFSETTING(   0x28, "Level 5")
	PORT_CONFSETTING(   0x30, "Level 6")
	PORT_CONFSETTING(   0x38, "Level 7")

	PORT_START("JUMPERS1")
	PORT_CONFNAME(0x07, 0x00, "Channel 1 address")
	PORT_CONFSETTING(   0x00, DEF_STR( None ))
	PORT_CONFSETTING(   0x01, "300-307 octal")
	PORT_CONFSETTING(   0x02, "310-317 octal")
	PORT_CONFSETTING(   0x03, "320-327 octal")
	PORT_CONFSETTING(   0x04, "330-337 octal")
	PORT_CONFSETTING(   0x05, "340-347 octal")
	PORT_CONFSETTING(   0x06, "350-357 octal")
	PORT_CONFNAME(0x38, 0x00, "Channel 1 interrupt")
	PORT_CONFSETTING(   0x00, DEF_STR( None ))
	PORT_CONFSETTING(   0x18, "Level 3")
	PORT_CONFSETTING(   0x20, "Level 4")
	PORT_CONFSETTING(   0x28, "Level 5")
	PORT_CONFSETTING(   0x30, "Level 6")
	PORT_CONFSETTING(   0x38, "Level 7")

INPUT_PORTS_END

ioport_constructor wh_8_47_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(wh_8_47_jumpers);
}

} // anonymous namespace

DEFINE_DEVICE_TYPE_PRIVATE(H8BUS_WH_8_47, device_h8bus_card_interface, wh_8_47_device, "h8_wh_8_47", "Heath H47 Interface and Serial I/O Card (WH8-47)");
