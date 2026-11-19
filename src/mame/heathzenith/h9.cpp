// license:BSD-3-Clause
// copyright-holders:Mark Garlanger, Paul Galbraith
/***************************************************************************

  Heathkit H9 Video Terminal

    Heath's first terminal, sold from 1977. It has no processor. The
    HEATH_H9 device in bus/heathzenith/h9 holds the whole terminal, so the
    rs232 option "h9" can use it too. This driver only connects it to a
    serial port.

****************************************************************************/

#include "emu.h"

#include "bus/heathzenith/h9/h9.h"
#include "bus/rs232/rs232.h"


namespace {

class h9_state : public driver_device
{
public:
	h9_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_h9(*this, "h9")
		, m_rs232(*this, "rs232")
	{
	}

	void h9(machine_config &config) ATTR_COLD;

private:
	required_device<heath_h9_device>   m_h9;
	required_device<rs232_port_device> m_rs232;
};


static DEVICE_INPUT_DEFAULTS_START( terminal )
	DEVICE_INPUT_DEFAULTS( "RS232_RXBAUD",   0xff, RS232_BAUD_600 )
	DEVICE_INPUT_DEFAULTS( "RS232_TXBAUD",   0xff, RS232_BAUD_600 )
	DEVICE_INPUT_DEFAULTS( "RS232_DATABITS", 0xff, RS232_DATABITS_8 )
	DEVICE_INPUT_DEFAULTS( "RS232_PARITY",   0xff, RS232_PARITY_NONE )
	DEVICE_INPUT_DEFAULTS( "RS232_STOPBITS", 0xff, RS232_STOPBITS_1 )
DEVICE_INPUT_DEFAULTS_END

void h9_state::h9(machine_config &config)
{
	HEATH_H9(config, m_h9);
	m_h9->serial_data_callback().set(m_rs232, FUNC(rs232_port_device::write_txd));

	RS232_PORT(config, m_rs232, default_rs232_devices, "loopback");
	m_rs232->rxd_handler().set(m_h9, FUNC(heath_h9_device::serial_in_w));
	m_rs232->set_option_device_input_defaults("terminal", DEVICE_INPUT_DEFAULTS_NAME(terminal));
	m_rs232->set_option_device_input_defaults("null_modem", DEVICE_INPUT_DEFAULTS_NAME(terminal));
}


ROM_START( h9 )
ROM_END

} // anonymous namespace

//    year  name  parent  compat  machine  input  class     init        company          fullname               flags
COMP( 1977, h9,   0,      0,      h9,      0,     h9_state, empty_init, "Heath Company", "H-9 Video Terminal",  MACHINE_SUPPORTS_SAVE )
