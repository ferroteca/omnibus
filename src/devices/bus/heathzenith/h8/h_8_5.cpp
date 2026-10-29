// license:BSD-3-Clause
// copyright-holders:Mark Garlanger
/***************************************************************************

  Heathkit H-8-5 Serial I/O and Cassette Interface

****************************************************************************/

#include "emu.h"

#include "h_8_5.h"

#include "imagedev/cassette.h"
#include "machine/clock.h"
#include "machine/i8251.h"
#include "machine/timer.h"
#include "bus/rs232/rs232.h"
#include "formats/h8_cas.h"

#include "softlist_dev.h"
#include "speaker.h"

#define LOG_LINES (1U << 1)
#define LOG_CASS  (1U << 2)
#define LOG_FUNC  (1U << 3)
#define VERBOSE   (0)

#include "logmacro.h"

#define LOGLINES(...)      LOGMASKED(LOG_LINES, __VA_ARGS__)
#define LOGCASS(...)       LOGMASKED(LOG_CASS, __VA_ARGS__)
#define LOGFUNC(...)       LOGMASKED(LOG_FUNC, __VA_ARGS__)

#ifdef _MSC_VER
#define FUNCNAME __func__
#else
#define FUNCNAME __PRETTY_FUNCTION__
#endif

namespace {

class h_8_5_device : public device_t
				   , public device_h8bus_card_interface
{
public:

	h_8_5_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 0);

protected:

	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;
	virtual void map_io(address_space_installer & space) override ATTR_COLD;

	void uart_rts(u8 data);
	void uart_tx_empty(u8 data);
	void console_rxrdy_w(int state);
	void console_dtr_w(int state);
	void update_console_int();

	TIMER_DEVICE_CALLBACK_MEMBER(kansas_r);
	TIMER_DEVICE_CALLBACK_MEMBER(kansas_w);

	required_device<i8251_device>          m_uart;
	required_device<i8251_device>          m_console;
	required_device<rs232_port_device>     m_rs232;
	required_device<cassette_image_device> m_cass_player;
	required_device<cassette_image_device> m_cass_recorder;
	required_device<clock_device>          m_console_rx_clock;
	required_device<clock_device>          m_console_tx_clock;
	required_device<clock_device>          m_tape_rx_clock;
	required_device<clock_device>          m_tape_tx_clock;
	required_ioport                        m_jumpers;

	u8   m_cass_data[4];
	bool m_cassbit;
	bool m_cassold;
	bool m_console_rxrdy;
	bool m_console_dtr_n;
};

h_8_5_device::h_8_5_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, H8BUS_H_8_5, tag, owner, 0)
	, device_h8bus_card_interface(mconfig, *this)
	, m_uart(*this, "uart")
	, m_console(*this, "console")
	, m_rs232(*this, "rs232")
	, m_cass_player(*this, "cassette_player")
	, m_cass_recorder(*this, "cassette_recorder")
	, m_console_rx_clock(*this, "console_rx_clock")
	, m_console_tx_clock(*this, "console_tx_clock")
	, m_tape_rx_clock(*this, "tape_rx_clock")
	, m_tape_tx_clock(*this, "tape_tx_clock")
	, m_jumpers(*this, "JUMPERS")
{
}

TIMER_DEVICE_CALLBACK_MEMBER(h_8_5_device::kansas_w)
{
	m_cass_data[3]++;

	if (m_cassbit != m_cassold)
	{
		LOGCASS("%s: m_cassbit changed : %d\n", FUNCNAME, m_cassbit);
		m_cass_data[3] = 0;
		m_cassold = m_cassbit;
	}

	LOGCASS("%s: m_cassbit: %d\n", FUNCNAME, m_cassbit);
	// 2400Hz -> 0, 1200Hz -> 1
	const int bit_pos = m_cassbit ? 0 : 1;

	m_cass_recorder->output(BIT(m_cass_data[3], bit_pos) ? -1.0 : +1.0);
}

TIMER_DEVICE_CALLBACK_MEMBER(h_8_5_device::kansas_r)
{
	// cassette - turn 1200/2400Hz to a bit
	m_cass_data[1]++;
	u8 cass_ws = (m_cass_player->input() > +0.03) ? 1 : 0;

	LOGCASS("%s: cass_ws: %d\n", FUNCNAME, cass_ws);

	if (cass_ws != m_cass_data[0])
	{
		LOGCASS("%s: cass_ws has changed value\n", FUNCNAME);
		m_cass_data[0] = cass_ws;
		m_uart->write_rxd((m_cass_data[1] < 12) ? 1 : 0);
		m_cass_data[1] = 0;
	}
}

void h_8_5_device::uart_rts(u8 data)
{
	LOGLINES("%s: data: %d\n", FUNCNAME, data);

	m_cass_player->change_state(bool(data) ? CASSETTE_STOPPED : CASSETTE_PLAY, CASSETTE_MASK_UISTATE);
}

void h_8_5_device::uart_tx_empty(u8 data)
{
	LOGLINES("%s: data: %d\n", FUNCNAME, data);

	m_cass_recorder->change_state(bool(data) ? CASSETTE_STOPPED : CASSETTE_RECORD, CASSETTE_MASK_UISTATE);
}

void h_8_5_device::console_rxrdy_w(int state)
{
	LOGFUNC("%s: state: %d\n", FUNCNAME, state);

	m_console_rxrdy = bool(state);

	update_console_int();
}

void h_8_5_device::console_dtr_w(int state)
{
	LOGLINES("%s: state: %d\n", FUNCNAME, state);

	m_console_dtr_n = bool(state);

	m_rs232->write_dtr(state);

	update_console_int();
}

void h_8_5_device::update_console_int()
{
	// Traced on the H-8-5 schematic (595-2032-01):
	// IC124 pin 24 (/DTR) goes to IC122B, a NAND with its other input at +5V,
	// so it inverts.  IC122B pin 6 drives the RS-232 DTR transistor Q107 and
	// the INT ON jumper, which feeds IC129D pin 12.  IC124 pin 14 (RxRDY) goes
	// to IC129D pin 13, and IC129D drives the RxR interrupt pad.  The INT OFF
	// jumper setting is not modelled.
	//
	// /DTR is active low, so m_console_dtr_n is false while it is asserted.
	set_slot_int3((m_console_rxrdy && !m_console_dtr_n) ? 1 : 0);
}

void h_8_5_device::device_start()
{
	m_console_rxrdy = false;
	m_console_dtr_n = true;

	save_item(NAME(m_console_rxrdy));
	save_item(NAME(m_console_dtr_n));
	save_item(NAME(m_cass_data));
	save_item(NAME(m_cassbit));
	save_item(NAME(m_cassold));
}

void h_8_5_device::device_reset()
{
	LOGFUNC("%s\n", FUNCNAME);

	// cassette
	m_cassbit      = 1;
	m_cassold      = 0;
	m_cass_data[0] = 0;
	m_cass_data[1] = 0;
	m_cass_data[2] = 0;
	m_cass_data[3] = 0;

	m_uart->write_cts(0);
	m_uart->write_dsr(0);
	m_uart->write_rxd(0);

	// Console UART clock is 16X the baud rate.
	static constexpr u32 BAUD[8] = { 110, 150, 300, 600, 1200, 2400, 4800, 9600 };
	ioport_value const jumpers(m_jumpers->read());

	m_console_rx_clock->set_unscaled_clock(BAUD[jumpers & 0x07] * 16);
	m_console_tx_clock->set_unscaled_clock(BAUD[(jumpers >> 3) & 0x07] * 16);

	// The tape UART clock is also 16X the baud rate: 19.2 kHz for 1200 baud
	// and 4.8 kHz for 300.
	m_tape_rx_clock->set_unscaled_clock(BIT(jumpers, 6) ? 1200 * 16 : 300 * 16);
	m_tape_tx_clock->set_unscaled_clock(BIT(jumpers, 7) ? 1200 * 16 : 300 * 16);
}

void h_8_5_device::map_io(address_space_installer & space)
{
	space.install_readwrite_handler(0xf8, 0xf9,
		read8sm_delegate(m_uart, FUNC(i8251_device::read)),
		write8sm_delegate(m_uart, FUNC(i8251_device::write))
	);

	space.install_readwrite_handler(0xfa, 0xfb,
		read8sm_delegate(m_console, FUNC(i8251_device::read)),
		write8sm_delegate(m_console, FUNC(i8251_device::write))
	);
}


// The SER RX and SER TX jumpers pick the console's receive and transmit
// rates from eight taps, 110 to 9600 baud.  The manual (595-2032-03, page
// 28) says to match the terminal and gives 600 for the H9.  The default
// terminal here is an H19, whose SW401 defaults to 9600, so both jumpers
// default to 9600.
static INPUT_PORTS_START( h_8_5 )
	PORT_START("JUMPERS")
	PORT_CONFNAME(0x07, 0x07, "SER RX baud rate jumper")
	PORT_CONFSETTING(   0x00, "110")
	PORT_CONFSETTING(   0x01, "150")
	PORT_CONFSETTING(   0x02, "300")
	PORT_CONFSETTING(   0x03, "600")
	PORT_CONFSETTING(   0x04, "1200")
	PORT_CONFSETTING(   0x05, "2400")
	PORT_CONFSETTING(   0x06, "4800")
	PORT_CONFSETTING(   0x07, "9600")
	PORT_CONFNAME(0x38, 0x38, "SER TX baud rate jumper")
	PORT_CONFSETTING(   0x00, "110")
	PORT_CONFSETTING(   0x08, "150")
	PORT_CONFSETTING(   0x10, "300")
	PORT_CONFSETTING(   0x18, "600")
	PORT_CONFSETTING(   0x20, "1200")
	PORT_CONFSETTING(   0x28, "2400")
	PORT_CONFSETTING(   0x30, "4800")
	PORT_CONFSETTING(   0x38, "9600")
	// The tape jumpers pick 300 or 1200 baud.  The manual (595-2151, page 5)
	// says Heath software tapes require 1200 baud, so both default to 1200.
	// On the card the Tape RX jumpers and C107 set the centre frequency of the
	// receive clock's phase-locked loop; this models the clock it locks to.
	// The manual calls the low Tape RX setting 600 baud, but its
	// specifications, and the Tape TX jumper, give 300.
	PORT_CONFNAME(0x40, 0x40, "Tape RX baud rate jumpers")
	PORT_CONFSETTING(   0x00, "300 (L)")
	PORT_CONFSETTING(   0x40, "1200 (H)")
	PORT_CONFNAME(0x80, 0x80, "Tape TX baud rate jumper")
	PORT_CONFSETTING(   0x00, "300")
	PORT_CONFSETTING(   0x80, "1200")
INPUT_PORTS_END

ioport_constructor h_8_5_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(h_8_5);
}

// These reach only a generic serial terminal on the port.  The H19 takes
// its rate from its own SW401.
static DEVICE_INPUT_DEFAULTS_START(h19)
	DEVICE_INPUT_DEFAULTS("RS232_RXBAUD",   0xff, RS232_BAUD_9600)
	DEVICE_INPUT_DEFAULTS("RS232_TXBAUD",   0xff, RS232_BAUD_9600)
	DEVICE_INPUT_DEFAULTS("RS232_DATABITS", 0xff, RS232_DATABITS_8)
	DEVICE_INPUT_DEFAULTS("RS232_PARITY",   0xff, RS232_PARITY_NONE)
	DEVICE_INPUT_DEFAULTS("RS232_STOPBITS", 0xff, RS232_STOPBITS_1)
DEVICE_INPUT_DEFAULTS_END


void h_8_5_device::device_add_mconfig(machine_config &config)
{
	I8251(config, m_uart);
	m_uart->txd_handler().set([this] (bool state) { m_cassbit = state; });
	m_uart->rts_handler().set(FUNC(h_8_5_device::uart_rts));
	m_uart->txempty_handler().set(FUNC(h_8_5_device::uart_tx_empty));

	// device_reset sets these from the tape baud rate jumpers
	CLOCK(config, m_tape_rx_clock, 1200*16).signal_handler().set(m_uart, FUNC(i8251_device::write_rxc));
	CLOCK(config, m_tape_tx_clock, 1200*16).signal_handler().set(m_uart, FUNC(i8251_device::write_txc));

	I8251(config, m_console);

	m_console->txd_handler().set("rs232", FUNC(rs232_port_device::write_txd));
	m_console->rts_handler().set("rs232", FUNC(rs232_port_device::write_rts));
	// /DTR goes to the port and also gates the console interrupt.
	m_console->dtr_handler().set(FUNC(h_8_5_device::console_dtr_w));
	// The RxRdy pin on the 8251 USART is normally jumpered to generate a level 3 i/o interrupt.
	m_console->rxrdy_handler().set(FUNC(h_8_5_device::console_rxrdy_w));

	// device_reset sets these from the baud rate jumpers
	CLOCK(config, m_console_rx_clock, 9600*16).signal_handler().set(m_console, FUNC(i8251_device::write_rxc));
	CLOCK(config, m_console_tx_clock, 9600*16).signal_handler().set(m_console, FUNC(i8251_device::write_txc));

	rs232_port_device &rs232(RS232_PORT(config, "rs232", default_rs232_devices, "h19"));
	rs232.rxd_handler().set(m_console, FUNC(i8251_device::write_rxd));
	rs232.cts_handler().set(m_console, FUNC(i8251_device::write_cts));
	rs232.dsr_handler().set(m_console, FUNC(i8251_device::write_dsr));
	rs232.set_option_device_input_defaults("h19", DEVICE_INPUT_DEFAULTS_NAME(h19));

	SPEAKER(config, "mono").front_center();

	CASSETTE(config, m_cass_player);
	m_cass_player->set_formats(h8_cassette_formats);
	m_cass_player->set_default_state(CASSETTE_STOPPED | CASSETTE_MOTOR_ENABLED | CASSETTE_SPEAKER_ENABLED);
	m_cass_player->add_route(ALL_OUTPUTS, "mono", 0.15);
	m_cass_player->set_interface("h88_cass_player");

	CASSETTE(config, m_cass_recorder);
	m_cass_recorder->set_formats(h8_cassette_formats);
	m_cass_recorder->set_default_state(CASSETTE_STOPPED | CASSETTE_MOTOR_ENABLED | CASSETTE_SPEAKER_ENABLED);
	m_cass_recorder->add_route(ALL_OUTPUTS, "mono", 0.15);
	m_cass_recorder->set_interface("h88_cass_recorder");

	// The same tapes the H-88-5 reads.  Both cards are an 8251 answering at
	// 370-371 octal, running Kansas City at 1200 baud with the tape jumpers at
	// their defaults, so the media is interchangeable - and the list is H8
	// software to begin with:
	// BUG-8, Extended Benton Harbor BASIC, the H8 assembler and the H8 editor,
	// which is Heath's H-8-18 cassette package.  Catalog #853 spells the
	// pairing out, one Cassette Operating System needing "an H-8-5 Serial I/O
	// and Cassette Interface to run with the Heath H-8 Computer" and an
	// H-88-5 on an All-In-One.  The list is named and owned by the H-88-5 the
	// same way h17_flop is named for the H-88-1 and shared with the H-8-17.
	SOFTWARE_LIST(config, "cass_list").set_compatible("h88_cass");

	TIMER(config, "kansas_w").configure_periodic(FUNC(h_8_5_device::kansas_w), attotime::from_hz(4800));
	TIMER(config, "kansas_r").configure_periodic(FUNC(h_8_5_device::kansas_r), attotime::from_hz(40000));
}

} // anonymous namespace

DEFINE_DEVICE_TYPE_PRIVATE(H8BUS_H_8_5, device_h8bus_card_interface, h_8_5_device, "h8_h_8_5", "Heath H-8-5 Serial/Casette Card");
