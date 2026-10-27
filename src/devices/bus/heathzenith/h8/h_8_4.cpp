// license:BSD-3-Clause
// copyright-holders:Paul Galbraith
/***************************************************************************

  Heathkit H-8-4 Four Port Serial I/O

    Four INS8250 ACEs on one card. Each has its own RS-232 connector, its own
    address selection jumpers and its own interrupt select jumpers. The four
    channels are the same circuit. A port's jumpers and the software decide
    what it is used for.

    The four addresses offered here are the ones Heath software expects.
    They are the same four an H-89 decodes on its own CPU board (see the
    decoder tables in bus/heathzenith/h89/h89bus.cpp), so a program written
    for an H-89 finds the ports where it looks for them:

        320-327 octal   D0-D7   alternate terminal, HDOS AT:
        330-337 octal   D8-DF   third serial port
        340-347 octal   E0-E7   line printer, HDOS LP:
        350-357 octal   E8-EF   console terminal, HDOS TT:

    HDOS from issue 50.03.00 on ships a driver for this card, ATH84.DVD. Its
    source listing (ATDVD in HOS-1-SL, 595-2466) has

        IF H84IO=0 THEN PORT = 374-5 ELSE PORT = 320-7

    374-375 octal is where the H-8-5 build of the same driver looks, and 320
    is this card's first port. So that driver finds this card at its default
    settings.

    Port 4 ships switched off, because HDOS moves its console onto a card it
    finds at 350 octal. The comment above h_8_4_jumpers explains why.

    Software programs the speed: one 1.8432 MHz oscillator clocks all four
    ACEs, and each ACE's divisor latch sets its rate.

    Not modelled: the reproduction boards take a 16550 in place of the 8250,
    and the original had a 20 mA current loop option for a Teletype.

    The card description and address list come from the SEBHC reproduction's
    documentation [https://sebhc.github.io/sebhc/pcbs/H8-4_doc.pdf] and its
    8250 programming notes
    [https://sebhc.github.io/sebhc/project8080/pgs/8250.html].

****************************************************************************/

#include "emu.h"

#include "h_8_4.h"

#include "bus/rs232/rs232.h"
#include "machine/ins8250.h"

namespace {

class h_8_4_device : public device_t
				   , public device_h8bus_card_interface
{
public:

	h_8_4_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 0);

protected:

	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;
	virtual void map_io(address_space_installer &space) override ATTR_COLD;

	void read_jumpers();
	void uart_int_w(int index, int state);
	void update_int(u8 level);

	required_device_array<ins8250_device, 4> m_uart;
	required_ioport_array<4>                 m_jumpers;

	// base address each port answers at, 0 when its address jumpers are out
	u8  m_base[4];

	// bus interrupt level each port is wired to, 0 when no wire is fitted
	u8  m_level[4];

	int m_intr[4];
};

h_8_4_device::h_8_4_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, H8BUS_H_8_4, tag, owner, 0)
	, device_h8bus_card_interface(mconfig, *this)
	, m_uart(*this, "uart%u", 1U)
	, m_jumpers(*this, "JUMPERS%u", 1U)
{
}

void h_8_4_device::read_jumpers()
{
	// Address selection, in the order h_8_4_jumpers lists the settings. Index
	// 0 is a port with its address jumpers out. The three spare entries stop
	// a hand-edited cfg file from reading past the end of the table.
	static constexpr u8 BASE[8] = { 0x00, 0xd0, 0xd8, 0xe0, 0xe8, 0x00, 0x00, 0x00 };

	for (int i = 0; i < 4; i++)
	{
		ioport_value const jumpers(m_jumpers[i]->read());

		m_base[i]  = BASE[jumpers & 0x07];
		m_level[i] = (jumpers >> 3) & 0x07;
	}
}

void h_8_4_device::uart_int_w(int index, int state)
{
	m_intr[index] = state;

	update_int(m_level[index]);
}

void h_8_4_device::update_int(u8 level)
{
	// no wire fitted to this port's interrupt holes
	if (level == 0)
	{
		return;
	}

	// Ports jumpered to the same level share one bus line. The line stays
	// asserted while any of them asserts it, so check all four.
	int state = 0;

	for (int i = 0; i < 4; i++)
	{
		if (m_base[i] && (m_level[i] == level))
		{
			state |= m_intr[i];
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

void h_8_4_device::device_start()
{
	for (int i = 0; i < 4; i++)
	{
		m_base[i]  = 0;
		m_level[i] = 0;
		m_intr[i]  = 0;
	}

	save_item(NAME(m_base));
	save_item(NAME(m_level));
	save_item(NAME(m_intr));
}

void h_8_4_device::device_reset()
{
	// map_io() reads the jumpers, and the CPU card runs it from its own reset.
	// Nothing orders the two resets. Reading the jumpers here as well would
	// stop map_io() from seeing that an address had moved.
	for (int i = 0; i < 4; i++)
	{
		m_intr[i] = 0;
	}

	for (u8 level = 3; level <= 7; level++)
	{
		update_int(level);
	}
}

void h_8_4_device::map_io(address_space_installer &space)
{
	// The bus maps its cards from the CPU card's reset, so this runs on every
	// reset. It reads the jumpers again, so an address changed in the machine
	// configuration menu takes effect at the next reset. A port that moved
	// gives up its old window. This only unmaps windows this card installed.
	u8 const previous[4] = { m_base[0], m_base[1], m_base[2], m_base[3] };

	read_jumpers();

	// Remove every old window before installing any new one. Otherwise, when
	// two ports swap addresses, the second port's unmap would remove the
	// window the first port just installed.
	for (int i = 0; i < 4; i++)
	{
		if (previous[i] && (previous[i] != m_base[i]))
		{
			space.unmap_readwrite(previous[i], previous[i] + 7);
		}
	}

	for (int i = 0; i < 4; i++)
	{
		if (m_base[i])
		{
			space.install_readwrite_handler(m_base[i], m_base[i] + 7,
				read8sm_delegate(m_uart[i], FUNC(ins8250_device::ins8250_r)),
				write8sm_delegate(m_uart[i], FUNC(ins8250_device::ins8250_w))
			);
		}
	}
}

void h_8_4_device::device_add_mconfig(machine_config &config)
{
	static char const *const RS232_TAG[4] = { "rs232_1", "rs232_2", "rs232_3", "rs232_4" };

	for (int i = 0; i < 4; i++)
	{
		INS8250(config, m_uart[i], XTAL(1'843'200));

		m_uart[i]->out_tx_callback().set(RS232_TAG[i], FUNC(rs232_port_device::write_txd));
		m_uart[i]->out_dtr_callback().set(RS232_TAG[i], FUNC(rs232_port_device::write_dtr));
		m_uart[i]->out_rts_callback().set(RS232_TAG[i], FUNC(rs232_port_device::write_rts));

		// The interrupt jumpers decide whether this reaches the bus, and on
		// which level. See update_int() and the JUMPERS ports.
		m_uart[i]->out_int_callback().set([this, i] (int state) { uart_int_w(i, state); });

		rs232_port_device &port(RS232_PORT(config, RS232_TAG[i], default_rs232_devices, nullptr));

		port.rxd_handler().set(m_uart[i], FUNC(ins8250_uart_device::rx_w));
		port.dcd_handler().set(m_uart[i], FUNC(ins8250_uart_device::dcd_w));
		port.dsr_handler().set(m_uart[i], FUNC(ins8250_uart_device::dsr_w));
		port.cts_handler().set(m_uart[i], FUNC(ins8250_uart_device::cts_w));
		port.ri_handler().set(m_uart[i], FUNC(ins8250_uart_device::ri_w));
	}
}

// PORT 4 STARTS SWITCHED OFF, SO FITTING THIS CARD KEEPS THE H8'S CONSOLE
// ------------------------------------------------------------------------
// HDOS from 50.03.00 on looks for an 8250 at 350 octal. If it finds one, it
// runs the console there instead of on the H-8-5's USART. With port 4 on,
// HDOS probes 350 (IER 0, LCR 8N1, MCR loop) and sets the divisor latch to 48
// for 2400 baud. After that it writes nothing to the H-8-5. Measured on
// h8_h17 with HDOS 1.6: HDOS wrote 18 characters to the H-8-5's data port
// with port 4 off, and none with it on. With nothing attached to port 4, the
// machine looks dead.
//
// To use port 4 as the console, attach a terminal and set the terminal to
// 2400 baud. HDOS programs 350 for 2400 baud, but the H19 and the generic
// terminal both start at 9600. The H-8-5 has the same problem with the H19's
// SW401. The console stays silent until it receives a character, so send a
// SPACE first. ACTION? <BOOT> and the rest of the boot then appear on port 4.
//
// Port 4's interrupt is jumpered to level 3, so turning on the address and
// attaching a terminal is enough. HDOS's console input is interrupt driven.
// On the same h8_h17 boot, a command typed at the date prompt is echoed and
// answered with the wire fitted. With the wire out, HDOS prints but takes no
// keys at all. The H-8-5's "Console interrupt" jumper fails the same way.
//
// REMark issue 22 (1981) agrees that an H-8-4 console sits at 350 octal and
// runs on interrupts. It prints an INKEY routine "designed for the H8-4
// Multi-Port Serial Card with the console set to 350 octal". The routine
// wraps a polled read in OUT 233,0 and OUT 233,1. 233 decimal is 0351 octal,
// the 8250's interrupt enable register. That stops the console interrupt
// firing while the routine reads the line status at 237 (0355, LSR) and the
// byte from 232 (0350, RBR).
//
// The other three ports start with no interrupt wire. AT: and LP: are
// normally left that way: HDOS polls both, and its AT: driver documentation
// says not to use it with receiver interrupts. A modem does want an
// interrupt. REMark issue 17 (1981) takes a reader through connecting to
// CompuServe and gives both settings: "the interrupt on each machine must be
// jumpered to level 5. The port assignment to the channel used on the H8-4
// serial I/O board must also be jumpered to 330Q." An earlier version of this
// note said level 6, from
// [https://sebhc.github.io/sebhc/project8080/pgs/feat.html]. The REMark
// walkthrough describes the software people ran, so it is the better source.
// The assembly manual was not available, so the lettered holes the board
// provides are not known. Levels 3 to 7 are the levels the H8 bus carries and
// the levels the H-8-5 offers.
static INPUT_PORTS_START( h_8_4_jumpers )

	PORT_START("JUMPERS1")
	PORT_CONFNAME(0x07, 0x01, "Port 1 address")
	PORT_CONFSETTING(   0x00, DEF_STR( None ))
	PORT_CONFSETTING(   0x01, "320-327 octal")
	PORT_CONFSETTING(   0x02, "330-337 octal")
	PORT_CONFSETTING(   0x03, "340-347 octal")
	PORT_CONFSETTING(   0x04, "350-357 octal")
	PORT_CONFNAME(0x38, 0x00, "Port 1 interrupt")
	PORT_CONFSETTING(   0x00, DEF_STR( None ))
	PORT_CONFSETTING(   0x18, "Level 3")
	PORT_CONFSETTING(   0x20, "Level 4")
	PORT_CONFSETTING(   0x28, "Level 5")
	PORT_CONFSETTING(   0x30, "Level 6")
	PORT_CONFSETTING(   0x38, "Level 7")

	PORT_START("JUMPERS2")
	PORT_CONFNAME(0x07, 0x02, "Port 2 address")
	PORT_CONFSETTING(   0x00, DEF_STR( None ))
	PORT_CONFSETTING(   0x01, "320-327 octal")
	PORT_CONFSETTING(   0x02, "330-337 octal")
	PORT_CONFSETTING(   0x03, "340-347 octal")
	PORT_CONFSETTING(   0x04, "350-357 octal")
	PORT_CONFNAME(0x38, 0x00, "Port 2 interrupt")
	PORT_CONFSETTING(   0x00, DEF_STR( None ))
	PORT_CONFSETTING(   0x18, "Level 3")
	PORT_CONFSETTING(   0x20, "Level 4")
	PORT_CONFSETTING(   0x28, "Level 5")
	PORT_CONFSETTING(   0x30, "Level 6")
	PORT_CONFSETTING(   0x38, "Level 7")

	PORT_START("JUMPERS3")
	PORT_CONFNAME(0x07, 0x03, "Port 3 address")
	PORT_CONFSETTING(   0x00, DEF_STR( None ))
	PORT_CONFSETTING(   0x01, "320-327 octal")
	PORT_CONFSETTING(   0x02, "330-337 octal")
	PORT_CONFSETTING(   0x03, "340-347 octal")
	PORT_CONFSETTING(   0x04, "350-357 octal")
	PORT_CONFNAME(0x38, 0x00, "Port 3 interrupt")
	PORT_CONFSETTING(   0x00, DEF_STR( None ))
	PORT_CONFSETTING(   0x18, "Level 3")
	PORT_CONFSETTING(   0x20, "Level 4")
	PORT_CONFSETTING(   0x28, "Level 5")
	PORT_CONFSETTING(   0x30, "Level 6")
	PORT_CONFSETTING(   0x38, "Level 7")

	PORT_START("JUMPERS4")
	PORT_CONFNAME(0x07, 0x00, "Port 4 address")
	PORT_CONFSETTING(   0x00, DEF_STR( None ))
	PORT_CONFSETTING(   0x01, "320-327 octal")
	PORT_CONFSETTING(   0x02, "330-337 octal")
	PORT_CONFSETTING(   0x03, "340-347 octal")
	PORT_CONFSETTING(   0x04, "350-357 octal")
	PORT_CONFNAME(0x38, 0x18, "Port 4 interrupt")
	PORT_CONFSETTING(   0x00, DEF_STR( None ))
	PORT_CONFSETTING(   0x18, "Level 3")
	PORT_CONFSETTING(   0x20, "Level 4")
	PORT_CONFSETTING(   0x28, "Level 5")
	PORT_CONFSETTING(   0x30, "Level 6")
	PORT_CONFSETTING(   0x38, "Level 7")

INPUT_PORTS_END

ioport_constructor h_8_4_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(h_8_4_jumpers);
}

} // anonymous namespace

DEFINE_DEVICE_TYPE_PRIVATE(H8BUS_H_8_4, device_h8bus_card_interface, h_8_4_device, "h8_h_8_4", "Heath H-8-4 Four Port Serial I/O Card");
