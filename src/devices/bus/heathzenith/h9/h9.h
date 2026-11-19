// license:BSD-3-Clause
// copyright-holders:Mark Garlanger, Paul Galbraith
/***************************************************************************

  Heathkit H9 Video Terminal

****************************************************************************/

#ifndef MAME_BUS_HEATHZENITH_H9_H9_H
#define MAME_BUS_HEATHZENITH_H9_H9_H

#pragma once

#include "machine/ay31015.h"
#include "machine/clock.h"
#include "machine/mm5740.h"
#include "machine/pla.h"
#include "sound/beep.h"

#include "emupal.h"
#include "screen.h"

#include <memory>


class heath_h9_device : public device_t
{
public:
	heath_h9_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 0);

	// Serial data out of the terminal. The H8 cable carries only Data+,
	// Data- and ground, so the terminal has no handshake lines (H9
	// Operations, Pictorial 2-3).
	auto serial_data_callback() { return m_write_sd.bind(); }

	// Serial data into the terminal.
	void serial_in_w(int state) { m_rxd = state; }

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset_after_children() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;

private:
	// MODS port
	static constexpr u8 MOD_CTRL         = 0x01;
	static constexpr u8 MOD_SHIFT        = 0x06;
	static constexpr u8 MOD_REPT         = 0x08;

	// FUNC port
	static constexpr u8 FUNC_UP          = 0x01;
	static constexpr u8 FUNC_DOWN        = 0x02;
	static constexpr u8 FUNC_LEFT        = 0x04;
	static constexpr u8 FUNC_RIGHT       = 0x08;
	static constexpr u8 FUNC_CURSOR      = 0x0f;
	static constexpr u8 FUNC_HOME        = 0x10;
	static constexpr u8 FUNC_ERASE_EOL   = 0x20;
	static constexpr u8 FUNC_ERASE_PAGE  = 0x40;
	static constexpr u8 FUNC_XMIT_PAGE   = 0x80;
	static constexpr u16 FUNC_BREAK      = 0x100;

	// MODE port - set when the push-on/push-off key is pushed in
	static constexpr u8 MODE_AUTO_CARRY  = 0x01;
	static constexpr u8 MODE_SHORT_FORM  = 0x02;
	static constexpr u8 MODE_SCROLL      = 0x04;
	static constexpr u8 MODE_PLOT        = 0x08;
	static constexpr u8 MODE_BAUD_RATE   = 0x10;
	static constexpr u8 MODE_OFF_LINE    = 0x20;
	static constexpr u8 MODE_FULL_DUPLEX = 0x40;

	// IC702 PLA inputs I1-I14
	static constexpr u32 PLA_XPEU      = 1 << 0;    // UART transmit buffer empty
	static constexpr u32 PLA_XBEP      = 1 << 1;    // parallel output buffer empty
	static constexpr u32 PLA_DUPLEX    = 1 << 2;    // half duplex
	static constexpr u32 PLA_CCR       = 1 << 3;    // control character or rubout
	static constexpr u32 PLA_ADDRESS   = 1 << 4;    // character can be written into the RAM
	static constexpr u32 PLA_OB2       = 1 << 5;
	static constexpr u32 PLA_1C_DAV    = 1 << 6;
	static constexpr u32 PLA_QN        = 1 << 7;    // transmit page character not loaded
	static constexpr u32 PLA_DAVPK     = 1 << 8;    // parallel or keyboard data available
	static constexpr u32 PLA_DAVU      = 1 << 9;    // UART data available
	static constexpr u32 PLA_CR        = 1 << 10;
	static constexpr u32 PLA_CRFF      = 1 << 11;
	static constexpr u32 PLA_XPN       = 1 << 12;   // not transmit page
	static constexpr u32 PLA_LF        = 1 << 13;

	// IC702 PLA outputs O1-O8
	static constexpr u32 PLA_LOAD_UART_N    = 1 << 0;
	static constexpr u32 PLA_LOAD_PARALLEL  = 1 << 1;
	static constexpr u32 PLA_OB2_XP_LOAD_N  = 1 << 2;
	static constexpr u32 PLA_CLR_QN_N       = 1 << 3;
	static constexpr u32 PLA_RESELECT_RAM_N = 1 << 4;
	static constexpr u32 PLA_O_CCR          = 1 << 5;
	static constexpr u32 PLA_O_LF           = 1 << 6;
	static constexpr u32 PLA_XBE            = 1 << 7;

	// special character latch
	static constexpr u8 SPECIAL_CCR  = 0x01;
	static constexpr u8 SPECIAL_LF   = 0x02;
	static constexpr u8 SPECIAL_CR   = 0x04;
	static constexpr u8 SPECIAL_BS   = 0x08;
	static constexpr u8 SPECIAL_DAVU = 0x10;    // data came from the UART

	required_device<screen_device>     m_screen;
	required_device<ay31015_device>    m_uart;
	required_device<clock_device>      m_uart_clock;
	required_device<beep_device>       m_beep;
	required_device<mm5740_device>     m_kbdenc;
	required_device<pla_device>        m_pla;
	required_region_ptr<u8>            m_ic724;
	required_region_ptr<u8>            m_ic728;
	required_region_ptr<u8>            m_ic730;
	required_region_ptr<u8>            m_chargen;
	required_ioport_array<8>           m_keys;
	required_ioport                    m_mods;
	required_ioport                    m_func;
	required_ioport                    m_mode;
	required_ioport                    m_config;

	devcb_write_line m_write_sd;

	std::unique_ptr<u8[]> m_ram;

	emu_timer *m_tpu_timer;
	emu_timer *m_bell_timer;

	// cursor latches
	u8   m_cur_col;
	u8   m_cur_blk;
	u8   m_cur_row;

	// scroll (start of page) counters
	u8   m_scroll_blk;
	u8   m_scroll_row;

	// count up flags used for the scroll check
	bool m_cu4;
	bool m_cu12;

	bool m_hold_screen;
	bool m_refresh_blank;
	u8   m_frame;
	u8   m_mode_keys;

	// transmit page
	bool m_xmit_page;

	// keyboard
	bool m_kbd_dav;
	u8   m_kbd_data;
	bool m_repeat_line;
	u16  m_func_last;
	u8   m_cursor_latch;
	bool m_xmit_request;

	// TPU flip-flops
	bool m_xp_sent;         // IC704A - transmit page character loaded, waiting for count up
	bool m_crff;            // IC738A - carriage return sent, send a line feed next
	bool m_eolff;           // IC738B - end of line without auto carry, send a carriage return next
	bool m_fake_dav;        // IC725A - half duplex transmit page count up
	bool m_xp_end;          // IC725B - end of page reached, end transmit page at 7C
	bool m_scroll_cmd;      // scroll command, TPU cycles 3-6
	bool m_count;           // 1C if DAV
	u8   m_special;         // special character latch (IC720)

	// serial
	u8   m_rxd;
	u8   m_txd;
	u32  m_baud_clock;
	bool m_tsb;

	TIMER_CALLBACK_MEMBER(tpu_cycle);
	TIMER_CALLBACK_MEMBER(bell_off);

	u32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	int  uart_si_r();
	void uart_so_w(int state) { update_txd(); }
	void update_txd();

	bool mode(u8 mask) const { return m_mode_keys & mask; }
	bool short_form() const { return mode(MODE_SHORT_FORM); }
	bool auto_carry() const { return mode(MODE_AUTO_CARRY); }

	static u16 ram_addr(u8 row, u8 blk, u8 col);
	u16  cursor_addr() const { return ram_addr(m_cur_row, m_cur_blk, m_cur_col); }
	u16  display_addr(int line, int col) const;

	u8   ic728_r(int pulse) const;
	u8   ic730_r() const;
	u8   ic724_r(u8 ic730, int pulse) const;
	void prom_count_20(bool up);
	void prom_count_4(bool up, u8 ic724);
	void prom_count_12(bool up, u8 ic724);
	void tpu_cycle_0();
	void tpu_cycle_1();
	void tpu_cycle_2();
	void tpu_cycle_3_6(int pulse);
	void tpu_cycle_7();

	void update_config();
	template <unsigned N> u16 kbd_x_r();
	int  kbd_shift_r();
	int  kbd_control_r();
	void kbd_data_ready_w(int state);
	void update_repeat();
	void frame_start();
	void tpu_step();
	void ring_bell();

	void cursor_home();

	void erase_page();
	void erase_eol();
	void erase_row(u8 row);
	void erase_block(u8 blk);
};

DECLARE_DEVICE_TYPE(HEATH_H9, heath_h9_device)

#endif // MAME_BUS_HEATHZENITH_H9_H9_H
