// license:BSD-3-Clause
// copyright-holders:Mark Garlanger, Paul Galbraith
/***************************************************************************

  Heathkit H9 Video Terminal

    A 1977 kit terminal built entirely from TTL logic - there is no
    microprocessor. 12 lines of 80 upper-case characters (or four columns
    of 12 lines of 20 in "short form") from a 1K x 8 static RAM, a 2513
    character generator, an MM5740AAE keyboard encoder and an AY-3-1015 UART.

    All housekeeping is done by the Timing and Processing Unit (TPU), a
    state machine built from three 32x8 PROMs (444-10, 444-11, 444-12),
    a PLA (443-787) and discrete gates. The TPU runs during vertical
    retrace, which is divided into eight complete TPU cycles; at 600 baud
    that services one received character per frame. When the refresh
    interrupt is enabled for higher baud rates, incoming data forces an
    early "artificial" retrace that blanks the rest of the frame while the
    data is serviced.

    Scrolling never moves data in the RAM; a scroll counter changes where
    screen refresh starts. The RAM is organized as 12 rows of four 20
    character blocks. Long form displays each row across the screen;
    short form displays each block as a 12 line column.

    The PROM and PLA images make the TPU decisions; the counters, latches
    and flip-flops are modeled from the circuit description in the
    Operation manual (595-2017-03).

    This is Mark Garlanger's h9 driver from MAME as a device, so that the
    rs232 option "h9" can use it as well as the h9 system. Heath's own
    interface standard pairs the H9 with the H8's H-8-5 serial card at
    600 baud, 8 bits, no parity (H9 Operations, page 10).

    TODO:
      - parallel input/output port and handshakes
      - reader start/stop (20 mA current loop) control
      - the PLA has not been dumped; the image was reconstructed from the
        Operation manual

****************************************************************************/

#include "emu.h"

#include "h9.h"

#include "speaker.h"

#include "heath_h9.lh"

#include <algorithm>


namespace {

// 100 character times per line (80 visible), 15,494 Hz.
// 256 lines per frame (192 visible), 60.5 Hz.
constexpr XTAL MASTER_CLOCK          = 12.395_MHz_XTAL;
constexpr int  DOTS_PER_CHAR         = 8;
constexpr int  CHARS_PER_SCAN_TOTAL  = 100;
constexpr int  LINES_PER_FRAME       = 256;
constexpr int  SCAN_LINES            = 16;   // per character row
constexpr int  ROWS                  = 12;
constexpr int  BLOCKS                = 4;
constexpr int  BLOCK_CHARS           = 20;
constexpr int  ROW_CHARS             = BLOCKS * BLOCK_CHARS;
constexpr int  VISIBLE_LINES         = ROWS * SCAN_LINES;
constexpr int  TPU_CYCLE_LINES       = 8;    // one complete TPU cycle

// 4800 baud (16x) clock is the character clock / 20 (IC215 QA).
// The I/O board divides it down to 300, and 110 (1200 / 11).
constexpr u32 BAUD_4800_DIVISOR = DOTS_PER_CHAR * 20;

constexpr u8 CHAR_BEL = 0x07;
constexpr u8 CHAR_BS  = 0x08;
constexpr u8 CHAR_LF  = 0x0a;
constexpr u8 CHAR_CR  = 0x0d;
constexpr u8 CHAR_SP  = 0x20;
constexpr u8 CHAR_DEL = 0x7f;

} // anonymous namespace


DEFINE_DEVICE_TYPE(HEATH_H9, heath_h9_device, "heath_h9", "Heathkit H9 Video Terminal")


heath_h9_device::heath_h9_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, HEATH_H9, tag, owner, clock)
	, m_screen(*this, "screen")
	, m_uart(*this, "uart")
	, m_uart_clock(*this, "uart_clock")
	, m_beep(*this, "beep")
	, m_kbdenc(*this, "kbdenc")
	, m_pla(*this, "ic702")
	, m_ic724(*this, "ic724")
	, m_ic728(*this, "ic728")
	, m_ic730(*this, "ic730")
	, m_chargen(*this, "chargen")
	, m_keys(*this, "X%u", 2U)
	, m_mods(*this, "MODS")
	, m_func(*this, "FUNC")
	, m_mode(*this, "MODE")
	, m_config(*this, "CONFIG")
	, m_write_sd(*this)
{
}

u16 heath_h9_device::ram_addr(u8 row, u8 blk, u8 col)
{
	return row * ROW_CHARS + blk * BLOCK_CHARS + col;
}


/***************************************************************************
    Video
***************************************************************************/

u16 heath_h9_device::display_addr(int line, int col) const
{
	// during refresh the ÷20 counter always cascades into the ÷4 counter, the
	// ÷12 counter is stepped by the character row counter
	u8 const blk = col / BLOCK_CHARS;

	if (short_form())
		return ram_addr(line, (m_scroll_blk + blk) % BLOCKS, col % BLOCK_CHARS);

	return ram_addr((m_scroll_row + line) % ROWS, blk, col % BLOCK_CHARS);
}

u32 heath_h9_device::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	bitmap.fill(0, cliprect);

	if (m_refresh_blank)
		return 0;

	bool const plot = mode(MODE_PLOT);

	// 4 Hz clock from the ÷16 repeat counter, used to blink control
	// characters in plot mode and the cursor in hold screen mode. Plot mode
	// also blanks the cursor.
	bool const blink       = BIT(m_frame, 3);
	bool const show_cursor = !plot && (!m_hold_screen || blink);
	u16 const cursor       = cursor_addr();

	for (int y = cliprect.top(); y <= cliprect.bottom(); y++)
	{
		int const line = y / SCAN_LINES;
		int const scan = y % SCAN_LINES;
		u16 *const pix = &bitmap.pix(y);

		// plot mode - the first 128 scan lines show a dash at the line
		// matching the 7-bit value of each character in the first row,
		// and the first row is repeated in the bottom four rows.
		if (plot && (line < 8))
		{
			for (int col = 0; col < ROW_CHARS; col++)
			{
				if ((m_ram[display_addr(0, col)] & 0x7f) == (127 - y))
				{
					for (int i = 0; i < 5; i++)
						pix[col * DOTS_PER_CHAR + i] = 1;
				}
			}
			continue;
		}

		int const disp_line = plot ? 0 : line;

		for (int col = 0; col < ROW_CHARS; col++)
		{
			u16 const addr = display_addr(disp_line, col);
			u8 const data  = m_ram[addr];
			u8 dots        = 0;

			if (scan < 8)
			{
				// only the low six bits address the character generator
				dots = m_chargen[(data & 0x3f) * 8 + scan] & 0x1f;

				// control characters are not displayed, except that they
				// blink in plot mode
				if (!(data & 0x60) && !(plot && blink))
					dots = 0;
			}
			else if (((scan == 12) || (scan == 13)) && (addr == cursor) && show_cursor)
			{
				dots = 0x1f;
			}

			for (int i = 0; i < 5; i++)
				pix[col * DOTS_PER_CHAR + i] = BIT(dots, 4 - i);
		}
	}

	return 0;
}


/***************************************************************************
    Cursor and scroll counters
***************************************************************************/

void heath_h9_device::cursor_home()
{
	// the cursor latches are loaded with the start of page
	m_cur_col = 0;

	if (short_form())
	{
		m_cur_row = 0;
		m_cur_blk = m_scroll_blk;
	}
	else
	{
		m_cur_row = m_scroll_row;
		m_cur_blk = 0;
	}
}


/***************************************************************************
    Erasing
***************************************************************************/

void heath_h9_device::erase_row(u8 row)
{
	std::fill_n(&m_ram[ram_addr(row, 0, 0)], ROW_CHARS, CHAR_SP);
}

void heath_h9_device::erase_block(u8 blk)
{
	for (int row = 0; row < ROWS; row++)
		std::fill_n(&m_ram[ram_addr(row, blk, 0)], BLOCK_CHARS, CHAR_SP);
}

void heath_h9_device::erase_page()
{
	std::fill_n(&m_ram[0], ROWS * ROW_CHARS, CHAR_SP);
	cursor_home();
	m_hold_screen = false;
}

void heath_h9_device::erase_eol()
{
	u8 const last_blk = short_form() ? m_cur_blk : BLOCKS - 1;

	for (int blk = m_cur_blk; blk <= last_blk; blk++)
	{
		for (int col = (blk == m_cur_blk) ? m_cur_col : 0; col < BLOCK_CHARS; col++)
			m_ram[ram_addr(m_cur_row, blk, col)] = CHAR_SP;
	}
}


/***************************************************************************
    Bell
***************************************************************************/

void heath_h9_device::ring_bell()
{
	// a ~30 ms burst of the 300 baud clock
	m_beep->set_state(1);
	m_bell_timer->adjust(attotime::from_msec(30));
}

TIMER_CALLBACK_MEMBER(heath_h9_device::bell_off)
{
	m_beep->set_state(0);
}


/***************************************************************************
    Keyboard
***************************************************************************/

template <unsigned N> u16 heath_h9_device::kbd_x_r()
{
	// the ports have Y1-Y7 in bits 0-6 and Y10 in bit 7
	u8 const data = m_keys[N - 2]->read();
	return ~((data & 0x7f) | ((data & 0x80) << 2)) & 0x3ff;
}

int heath_h9_device::kbd_shift_r()
{
	return (m_mods->read() & MOD_SHIFT) ? 1 : 0;
}

int heath_h9_device::kbd_control_r()
{
	return (m_mods->read() & MOD_CTRL) ? 1 : 0;
}

void heath_h9_device::kbd_data_ready_w(int state)
{
	// the negative true outputs are inverted onto the ASCII bus by IC403
	if (state)
	{
		m_kbd_data = ~m_kbdenc->b_r() & 0x7f;
		m_kbd_dav  = true;
	}
}

void heath_h9_device::update_repeat()
{
	// the REPT key connects the selected output of the ÷16 counter (IC401),
	// clocked by the vertical rate, to the encoder's repeat input
	int const bit   = 3 - (m_config->read() & 0x03);
	bool const line = (m_mods->read() & MOD_REPT) && BIT(m_frame, bit);

	m_kbdenc->repeat_line_w(line);
	m_repeat_line = line;
}

void heath_h9_device::update_config()
{
	m_mode_keys = m_mode->read();

	// scroll counters are held cleared in the unused direction
	if (short_form())
		m_scroll_row = 0;
	else
		m_scroll_blk = 0;

	// baud rate - the BAUD RATE key selects 110 baud or the rear panel
	// switch, which selects 300 baud or the preset jumper
	u32 const base      = MASTER_CLOCK.value() / BAUD_4800_DIVISOR;
	u8 const config     = m_config->read();
	bool const baud_110 = !mode(MODE_BAUD_RATE);
	u32 clock;

	if (baud_110)
		clock = base / 4 / 11;
	else if (!BIT(config, 7))
		clock = base / 16;
	else
		clock = (base * 2) >> ((config >> 4) & 0x07);

	if (clock != m_baud_clock)
	{
		m_baud_clock = clock;
		m_uart_clock->set_unscaled_clock(clock);
	}

	// two stop bits at 110 baud
	if (baud_110 != m_tsb)
	{
		m_tsb = baud_110;
		m_uart->write_tsb(m_tsb);
	}
}


/***************************************************************************
    TPU
***************************************************************************/

void heath_h9_device::frame_start()
{
	// once per frame: the keyboard repeat counter is clocked by the vertical
	// rate, and the cursor and transmit page keys set latches that the next
	// TPU cycle consumes
	update_config();

	// the cursor keys repeat on the same clock as the encoder
	bool const last_repeat   = m_repeat_line;
	update_repeat();
	bool const repeat_strobe = m_repeat_line && !last_repeat;

	u16 const func    = m_func->read();
	u16 const pressed = func & ~m_func_last;

	m_func_last     = func;
	m_cursor_latch |= (pressed | (repeat_strobe ? func : 0)) & FUNC_CURSOR;

	if (pressed & FUNC_XMIT_PAGE)
		m_xmit_request = true;

	m_frame++;
}

/*
    TPU driven by the IC728, IC730 and IC724 PROMs and the IC702 PLA. The
    counters, latches and flip-flops are modeled in C++; the PROMs and PLA
    make the decisions. Each TPU cycle has eight sub-cycles (pulses 0-7).
*/

u8 heath_h9_device::ic728_r(int pulse) const
{
	// CH is always high in the part of the scan line the TPU acts on
	u8 const addr = 0x10 | ((m_func->read() & FUNC_HOME) ? 0x08 : 0) | ((pulse == 7) ? 0 : 0x04) | ((pulse == 1) ? 0 : 0x02) | ((pulse == 0) ? 0 : 0x01);
	return m_ic728[addr];
}

u8 heath_h9_device::ic730_r() const
{
	// auto carry is 0 when selected, short form is 1 in long form, VN' is 1
	// during retrace and ÷4 EOL is 0 on the last block
	u8 const addr = (auto_carry() ? 0 : 0x10) | ((m_cur_col == BLOCK_CHARS - 1) ? 0x08 : 0) | (short_form() ? 0 : 0x04) | 0x02 | ((m_cur_blk == BLOCKS - 1) ? 0 : 0x01);
	return m_ic730[addr];
}

u8 heath_h9_device::ic724_r(u8 ic730, int pulse) const
{
	// the scroll command is 0 during a scroll
	u8 const addr = (m_scroll_cmd ? 0 : 0x10) | ((pulse == 6) ? 0 : 0x08) | ((pulse == 3) ? 0 : 0x04) | (BIT(ic730, 2) ? 0x02 : 0) | (BIT(ic730, 3) ? 0x01 : 0);
	return m_ic724[addr];
}

void heath_h9_device::prom_count_12(bool up, u8 ic724)
{
	bool wrap;

	if (up)
	{
		m_cu12 = true;
		wrap   = (++m_cur_row == ROWS);
		if (wrap)
			m_cur_row = 0;
	}
	else
	{
		wrap      = (m_cur_row == 0);
		m_cur_row = wrap ? (ROWS - 1) : (m_cur_row - 1);
	}

	// ÷12 -> ÷4
	if (wrap && BIT(ic724, 0))
		prom_count_4(up, ic724 & ~0x02);
}

void heath_h9_device::prom_count_4(bool up, u8 ic724)
{
	bool wrap;

	if (up)
	{
		m_cu4 = true;
		wrap  = (++m_cur_blk == BLOCKS);
		if (wrap)
			m_cur_blk = 0;
	}
	else
	{
		wrap      = (m_cur_blk == 0);
		m_cur_blk = wrap ? (BLOCKS - 1) : (m_cur_blk - 1);
	}

	// ÷4 -> ÷12
	if (wrap && BIT(ic724, 1))
		prom_count_12(up, ic724 & ~0x01);
}

void heath_h9_device::prom_count_20(bool up)
{
	// the cascades are selected before the count changes the EOL inputs
	u8 const c730 = ic730_r();
	u8 const c724 = ic724_r(c730, 1);
	bool wrap;

	if (up)
	{
		wrap = (++m_cur_col == BLOCK_CHARS);
		if (wrap)
			m_cur_col = 0;
	}
	else
	{
		wrap      = (m_cur_col == 0);
		m_cur_col = wrap ? (BLOCK_CHARS - 1) : (m_cur_col - 1);
	}

	if (!wrap)
		return;

	if (BIT(c730, 3))       // ÷20 -> ÷4
		prom_count_4(up, c724);
	if (BIT(c730, 1))       // ÷20 -> ÷12
		prom_count_12(up, c724);
}

void heath_h9_device::tpu_cycle_0()
{
	// OB1 - the DAV priority latch selects the UART, then the parallel input
	// (not emulated) and the keyboard. Hold screen inhibits the external
	// inputs, and the keyboard is only accepted when the output buffers are
	// empty (XBE).
	bool const xpeu  = m_uart->tbmt_r();
	bool const xbep  = true;     // parallel output not emulated
	bool const half  = !mode(MODE_FULL_DUPLEX);
	bool const plot  = mode(MODE_PLOT);
	bool const davu  = !m_hold_screen && m_uart->dav_r();
	bool const davpk = !davu && m_kbd_dav && xpeu && xbep;
	bool const xp    = m_xmit_page;
	bool const qn    = !m_xp_sent;

	m_count = false;

	// 0C - put the data on the bus
	u8 data;
	if (davu)
	{
		data = m_uart->receive();
	}
	else if (davpk)
	{
		data      = m_kbd_data;
		m_kbd_dav = false;

		// end of line bell at seven characters from the end of the line
		if ((m_cur_col == BLOCK_CHARS - 8) && (short_form() || (m_cur_blk == BLOCKS - 1)))
			ring_bell();
	}
	else if (xp && qn)
	{
		// the special character generator supplies the line feed after a
		// carriage return and the carriage return at the end of a line
		data = m_crff ? CHAR_LF : m_eolff ? CHAR_CR : m_ram[cursor_addr()];
	}
	else
	{
		return;
	}

	// special character detector - plot mode skips the special movements
	u8 const ch    = data & 0x7f;
	bool const ccr = !plot && (!(ch & 0x60) || (ch == CHAR_DEL));
	bool const cr  = !plot && (ch == CHAR_CR);
	bool const lf  = !plot && (ch == CHAR_LF);

	// the bell detector is gated with OB2 DAV, so transmit page doesn't ring it
	if ((ch == CHAR_BEL) && (davu || davpk))
		ring_bell();

	// a carriage return is only written over a space (IC704B)
	bool const address = !ccr || (cr && (m_ram[cursor_addr()] == CHAR_SP));

	// OB2 - the PLA enables the buffers and latches
	u32 const in = PLA_OB2 |
			(xpeu ? PLA_XPEU : 0) | (xbep ? PLA_XBEP : 0) | (half ? PLA_DUPLEX : 0) |
			(ccr ? PLA_CCR : 0) | (address ? PLA_ADDRESS : 0) | (qn ? PLA_QN : 0) |
			(davpk ? PLA_DAVPK : 0) | (davu ? PLA_DAVU : 0) | (cr ? PLA_CR : 0) |
			(m_crff ? PLA_CRFF : 0) | (xp ? 0 : PLA_XPN) | (lf ? PLA_LF : 0);
	u32 const out = m_pla->read(in);

	if (!(out & PLA_LOAD_UART_N))
		m_uart->transmit(data);

	if (!(out & PLA_RESELECT_RAM_N))
		m_ram[cursor_addr()] = data;

	if (!(out & PLA_OB2_XP_LOAD_N))
	{
		// character sent; the CR and EOL flip-flops are clocked, and
		// in half duplex a "fake" DAV counts the cursor up. The CR
		// flip-flop stays set until the line feed is counted at 1C,
		// which in full duplex is when its echo comes back.
		m_xp_sent  = true;
		m_eolff    = BIT(ic730_r(), 4) && !cr;
		m_fake_dav = half;
		if (cr)
			m_crff = true;
	}

	m_special = ((out & PLA_O_CCR) ? SPECIAL_CCR : 0) | ((out & PLA_O_LF) ? SPECIAL_LF : 0) |
			(cr ? SPECIAL_CR : 0) | ((!plot && (ch == CHAR_BS)) ? SPECIAL_BS : 0) | (davu ? SPECIAL_DAVU : 0);

	// the cursor counts up after data from the UART, after the keyboard in
	// half duplex (in full duplex the echo does it), or after a fake DAV
	m_count = davu || (davpk && half) || m_fake_dav;
}

void heath_h9_device::tpu_cycle_1()
{
	u8 const c730 = ic730_r();

	if (m_count)
	{
		if (!(m_special & SPECIAL_CCR))
		{
			// end of line without auto carry defeats the ÷20 count up
			if (!BIT(c730, 4))
				prom_count_20(true);
		}
		else if (m_special & SPECIAL_CR)
		{
			// assumed: in long form a carriage return also clears the block
			// counter, which the manual does not say
			m_cur_col = 0;
			if (!short_form())
				m_cur_blk = 0;
		}
		else if (m_special & SPECIAL_BS)
		{
			prom_count_20(false);
		}

		// a line feed also resets the CR flip-flop
		if (m_special & SPECIAL_LF)
		{
			prom_count_12(true, ic724_r(c730, 1));
			m_crff = false;
		}

		// clear the transmit page flip-flop after an echo (full duplex) or
		// any count up (half duplex)
		u32 const in = PLA_1C_DAV | (mode(MODE_FULL_DUPLEX) ? 0 : PLA_DUPLEX) | ((m_special & SPECIAL_DAVU) ? PLA_DAVU : 0) | PLA_XPN;
		if (!(m_pla->read(in) & PLA_CLR_QN_N))
			m_xp_sent = false;

		m_fake_dav = false;
		m_count    = false;
	}

	// cursor controls
	if (m_cursor_latch & FUNC_RIGHT)
		prom_count_20(true);
	if (m_cursor_latch & FUNC_LEFT)
		prom_count_20(false);
	if (m_cursor_latch & FUNC_DOWN)
		prom_count_12(true, ic724_r(c730, 1));
	if (m_cursor_latch & FUNC_UP)
		prom_count_12(false, ic724_r(c730, 1));

	m_cursor_latch = 0;
}

void heath_h9_device::tpu_cycle_2()
{
	// scroll check - a ÷4 (short form) or ÷12 (long form) count up onto the
	// start of page
	bool const coincidence = short_form() ? (m_cu4 && (m_cur_blk == m_scroll_blk)) : (m_cu12 && (m_cur_row == m_scroll_row));

	m_cu4  = false;
	m_cu12 = false;

	if (!coincidence)
		return;

	if (m_xmit_page)
		m_xp_end = true;            // scroll inhibited, transmit page ends at 7C
	else if (!mode(MODE_SCROLL))
		m_hold_screen = true;
	else
		m_scroll_cmd = true;
}

void heath_h9_device::tpu_cycle_3_6(int pulse)
{
	u8 const c724 = ic724_r(ic730_r(), pulse);

	// 3 - the scroll erase clears the row or block the cursor is in (not in
	// plot mode) and the scroll counter counts up
	if (BIT(c724, 2))
	{
		if (!mode(MODE_PLOT))
		{
			if (short_form())
				erase_block(m_cur_blk);
			else
				erase_row(m_cur_row);
		}

		if (short_form())
			m_scroll_blk = (m_scroll_blk + 1) % BLOCKS;
		else
			m_scroll_row = (m_scroll_row + 1) % ROWS;
	}

	if ((pulse == 3) && (m_func->read() & FUNC_ERASE_EOL))
		erase_eol();

	// 6 - the erase flip-flop is cleared, ending the scroll
	if ((pulse == 6) && !BIT(c724, 3))
		m_scroll_cmd = false;
}

void heath_h9_device::tpu_cycle_7()
{
	u16 const func = m_func->read();

	if (func & FUNC_HOME)
		cursor_home();

	if (m_hold_screen && mode(MODE_SCROLL))
		m_hold_screen = false;

	if (m_xp_end)
	{
		m_xp_end    = false;
		m_xmit_page = false;
	}

	if (m_xmit_request)
	{
		m_xmit_request = false;
		m_xmit_page    = true;
		m_xp_sent      = false;
		m_crff         = false;
		m_eolff        = false;
		m_fake_dav     = false;
	}

	if (func & FUNC_BREAK)
		m_xmit_page = false;
}

void heath_h9_device::tpu_step()
{
	// erase page is held preset while the key is down
	if (m_func->read() & FUNC_ERASE_PAGE)
		erase_page();

	for (int pulse = 0; pulse < 8; pulse++)
	{
		u8 const c728 = ic728_r(pulse);

		if (BIT(c728, 4))           // 0C
			tpu_cycle_0();
		if (BIT(c728, 3))           // 1C
			tpu_cycle_1();
		if (pulse == 2)             // 2C
			tpu_cycle_2();
		if ((pulse >= 3) && (pulse <= 6))
			tpu_cycle_3_6(pulse);
		if (BIT(c728, 2))           // 7C
			tpu_cycle_7();
	}

	update_txd();
}


TIMER_CALLBACK_MEMBER(heath_h9_device::tpu_cycle)
{
	int const line = m_screen->vpos() & ~(TPU_CYCLE_LINES - 1);

	// the refresh interrupt is enabled when the rear panel switch selects a
	// preset rate above 600 baud
	u8 const config = m_config->read();
	bool const fast = mode(MODE_BAUD_RATE) && BIT(config, 7) && (((config >> 4) & 0x07) < 4);

	// vertical retrace is divided into eight complete TPU cycles; an
	// artificial retrace runs complete TPU cycles for the rest of the frame
	if (line == VISIBLE_LINES)
	{
		// start of vertical retrace ends any artificial retrace
		m_screen->update_partial(VISIBLE_LINES - 1);
		m_refresh_blank = false;

		frame_start();
		tpu_step();
	}
	else if ((line > VISIBLE_LINES) || m_refresh_blank)
	{
		tpu_step();
	}
	else if (fast && !m_hold_screen && m_uart->dav_r())
	{
		// refresh interrupt - stop the screen refresh for the rest of the frame
		if (line > 0)
			m_screen->update_partial(line - 1);
		m_refresh_blank = true;

		tpu_step();
	}

	m_tpu_timer->adjust(m_screen->time_until_pos((line + TPU_CYCLE_LINES) % LINES_PER_FRAME));
}


/***************************************************************************
    Serial
***************************************************************************/

int heath_h9_device::uart_si_r()
{
	// off line holds the serial input at mark
	return mode(MODE_OFF_LINE) ? 1 : m_rxd;
}

void heath_h9_device::update_txd()
{
	// break forces a space, off line forces a mark
	u8 txd;
	if (mode(MODE_OFF_LINE))
		txd = 1;
	else if (m_func->read() & FUNC_BREAK)
		txd = 0;
	else
		txd = m_uart->so_r();

	if (txd != m_txd)
	{
		m_txd = txd;
		m_write_sd(txd);
	}
}


/***************************************************************************
    Machine
***************************************************************************/

void heath_h9_device::device_start()
{
	m_ram        = make_unique_clear<u8 []>(1024);

	m_tpu_timer  = timer_alloc(FUNC(heath_h9_device::tpu_cycle), this);
	m_bell_timer = timer_alloc(FUNC(heath_h9_device::bell_off), this);

	// the UART resets before device_reset_after_children() and reads the mode
	// keys through update_txd(); an impossible m_txd forces the first write
	m_mode_keys  = 0;
	m_rxd        = 1;
	m_txd        = 0xff;
	m_baud_clock = 0;
	m_tsb        = false;
	m_frame      = 0;

	save_pointer(NAME(m_ram), 1024);
	save_item(NAME(m_cur_col));
	save_item(NAME(m_cur_blk));
	save_item(NAME(m_cur_row));
	save_item(NAME(m_scroll_blk));
	save_item(NAME(m_scroll_row));
	save_item(NAME(m_cu4));
	save_item(NAME(m_cu12));
	save_item(NAME(m_hold_screen));
	save_item(NAME(m_refresh_blank));
	save_item(NAME(m_frame));
	save_item(NAME(m_mode_keys));
	save_item(NAME(m_xmit_page));
	save_item(NAME(m_kbd_dav));
	save_item(NAME(m_kbd_data));
	save_item(NAME(m_repeat_line));
	save_item(NAME(m_func_last));
	save_item(NAME(m_cursor_latch));
	save_item(NAME(m_xmit_request));
	save_item(NAME(m_xp_sent));
	save_item(NAME(m_crff));
	save_item(NAME(m_eolff));
	save_item(NAME(m_fake_dav));
	save_item(NAME(m_xp_end));
	save_item(NAME(m_scroll_cmd));
	save_item(NAME(m_count));
	save_item(NAME(m_special));
	save_item(NAME(m_rxd));
	save_item(NAME(m_txd));
	save_item(NAME(m_baud_clock));
	save_item(NAME(m_tsb));
}

void heath_h9_device::device_reset_after_children()
{
	// power up reset erases the page, momentarily selects short form to
	// clear the ÷4 scroll counter, and clears transmit page
	m_scroll_blk    = 0;
	m_scroll_row    = 0;
	m_cu4           = false;
	m_cu12          = false;
	m_refresh_blank = false;
	m_xmit_page     = false;
	m_kbd_dav       = false;
	m_kbd_data      = 0;
	m_repeat_line   = false;
	m_func_last     = 0;
	m_cursor_latch  = 0;
	m_xmit_request  = false;
	m_xp_sent       = false;
	m_crff          = false;
	m_eolff         = false;
	m_fake_dav      = false;
	m_xp_end        = false;
	m_scroll_cmd    = false;
	m_count         = false;
	m_special       = 0;

	// The UART word length and parity come from jumpers on the I/O board.
	// Setting the pins needs the input ports, which can't be read in
	// device_start().
	u16 const config = m_config->read();
	u8 const bits    = (config >> 8) & 0x03;     // 0 = 5 bits, 3 = 8 bits

	m_uart->write_swe(0);
	m_uart->write_np(BIT(config, 10) ? 0 : 1);
	m_uart->write_eps(BIT(config, 11));
	m_uart->write_nb1(BIT(bits, 0));
	m_uart->write_nb2(BIT(bits, 1));
	m_uart->write_cs(1);

	update_config();
	erase_page();

	m_beep->set_state(0);
	m_tpu_timer->adjust(m_screen->time_until_pos(0));
}


/***************************************************************************
    Inputs
***************************************************************************/

static INPUT_PORTS_START( h9 )
	// MM5740 key matrix - one port per X drive line, bits 0-6 are sense
	// lines Y1-Y7 and bit 7 is Y10
	PORT_START("X2")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_ENTER)      PORT_CHAR(13)                    PORT_NAME("RETURN")
	PORT_BIT(0x3c, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_SPACE)      PORT_CHAR(' ')
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_EQUALS)     PORT_CHAR('-') PORT_CHAR('=')

	PORT_START("X3")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_0)          PORT_CHAR('0')
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_RWIN)       PORT_CHAR(10)                    PORT_NAME("LINE FEED")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_P)          PORT_CHAR('P') PORT_CHAR('@')
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_DEL)        PORT_CHAR(127)                   PORT_NAME("RUB OUT")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_COLON)      PORT_CHAR(';') PORT_CHAR('+')
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_SLASH)      PORT_CHAR('/') PORT_CHAR('?')
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_MINUS)      PORT_CHAR(':') PORT_CHAR('*')

	PORT_START("X4")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_9)          PORT_CHAR('9') PORT_CHAR(')')
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_I)          PORT_CHAR('I')
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_O)          PORT_CHAR('O') PORT_CHAR('_')
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_K)          PORT_CHAR('K') PORT_CHAR('[')
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_L)          PORT_CHAR('L') PORT_CHAR('\\')
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_COMMA)      PORT_CHAR(',') PORT_CHAR('<')
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_STOP)       PORT_CHAR('.') PORT_CHAR('>')
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_8)          PORT_CHAR('8') PORT_CHAR('(')

	PORT_START("X5")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_6)          PORT_CHAR('6') PORT_CHAR('&')
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_U)          PORT_CHAR('U')
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_Y)          PORT_CHAR('Y')
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_J)          PORT_CHAR('J')
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_H)          PORT_CHAR('H')
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_M)          PORT_CHAR('M') PORT_CHAR(']')
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_N)          PORT_CHAR('N') PORT_CHAR('^')
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_7)          PORT_CHAR('7') PORT_CHAR('\'')

	PORT_START("X6")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_5)          PORT_CHAR('5') PORT_CHAR('%')
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_R)          PORT_CHAR('R')
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_T)          PORT_CHAR('T')
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F)          PORT_CHAR('F')
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_G)          PORT_CHAR('G')
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_V)          PORT_CHAR('V')
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_B)          PORT_CHAR('B')
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_4)          PORT_CHAR('4') PORT_CHAR('$')

	PORT_START("X7")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_E)          PORT_CHAR('E')
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_D)          PORT_CHAR('D')
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_C)          PORT_CHAR('C')
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_3)          PORT_CHAR('3') PORT_CHAR('#')

	PORT_START("X8")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_W)          PORT_CHAR('W')
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_S)          PORT_CHAR('S')
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_X)          PORT_CHAR('X')
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_2)          PORT_CHAR('2') PORT_CHAR('"')

	PORT_START("X9")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_Q)          PORT_CHAR('Q')
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_ESC)        PORT_CHAR(UCHAR_MAMEKEY(ESC))    PORT_NAME("ESC")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_A)          PORT_CHAR('A')
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_Z)          PORT_CHAR('Z')
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_1)          PORT_CHAR('1') PORT_CHAR('!')

	PORT_START("MODS")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_LCONTROL)   PORT_CHAR(UCHAR_SHIFT_2)         PORT_NAME("CTRL")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_LSHIFT)     PORT_CHAR(UCHAR_SHIFT_1)         PORT_NAME("Left SHIFT")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_RSHIFT)                                      PORT_NAME("Right SHIFT")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_RALT)                                        PORT_NAME("REPT")

	PORT_START("FUNC")
	PORT_BIT(0x001, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_UP)        PORT_CHAR(UCHAR_MAMEKEY(UP))     PORT_NAME("Up")
	PORT_BIT(0x002, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_DOWN)      PORT_CHAR(UCHAR_MAMEKEY(DOWN))   PORT_NAME("Down")
	PORT_BIT(0x004, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_LEFT)      PORT_CHAR(UCHAR_MAMEKEY(LEFT))   PORT_NAME("Left")
	PORT_BIT(0x008, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_RIGHT)     PORT_CHAR(UCHAR_MAMEKEY(RIGHT))  PORT_NAME("Right")
	PORT_BIT(0x010, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_HOME)      PORT_CHAR(UCHAR_MAMEKEY(HOME))   PORT_NAME("HOME")
	PORT_BIT(0x020, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F1)                                         PORT_NAME("ERASE EOL")
	PORT_BIT(0x040, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F2)                                         PORT_NAME("ERASE PAGE")
	PORT_BIT(0x080, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F3)                                         PORT_NAME("XMIT PAGE")
	PORT_BIT(0x100, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_PAUSE)                                      PORT_NAME("BREAK")

	// push-on/push-off keys, a set bit means the key is pushed in. The keys
	// that default to pushed in are active low.
	PORT_START("MODE")
	PORT_BIT(0x01, IP_ACTIVE_LOW,  IPT_KEYBOARD)  PORT_CODE(KEYCODE_F5)        PORT_TOGGLE                      PORT_NAME("AUTO CARRY")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD)  PORT_CODE(KEYCODE_F6)        PORT_TOGGLE                      PORT_NAME("SHORT FORM")
	PORT_BIT(0x04, IP_ACTIVE_LOW,  IPT_KEYBOARD)  PORT_CODE(KEYCODE_F7)        PORT_TOGGLE                      PORT_NAME("SCROLL")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD)  PORT_CODE(KEYCODE_F8)        PORT_TOGGLE                      PORT_NAME("PLOT")
	PORT_BIT(0x10, IP_ACTIVE_LOW,  IPT_KEYBOARD)  PORT_CODE(KEYCODE_F9)        PORT_TOGGLE                      PORT_NAME("BAUD RATE")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD)  PORT_CODE(KEYCODE_F10)       PORT_TOGGLE                      PORT_NAME("OFF LINE")
	PORT_BIT(0x40, IP_ACTIVE_LOW,  IPT_KEYBOARD)  PORT_CODE(KEYCODE_F11)       PORT_TOGGLE                      PORT_NAME("FULL DUPLEX")

	PORT_START("CONFIG")
	PORT_CONFNAME(0x03, 0x00, "Repeat rate")
	PORT_CONFSETTING(   0x00, "3.7 characters/second")
	PORT_CONFSETTING(   0x01, "7.5 characters/second")
	PORT_CONFSETTING(   0x02, "15 characters/second")
	PORT_CONFSETTING(   0x03, "30 characters/second")
	PORT_CONFNAME(0x70, 0x40, "Preset baud rate")
	PORT_CONFSETTING(   0x00, "9600")
	PORT_CONFSETTING(   0x10, "4800")
	PORT_CONFSETTING(   0x20, "2400")
	PORT_CONFSETTING(   0x30, "1200")
	PORT_CONFSETTING(   0x40, "600")
	PORT_CONFNAME(0x80, 0x80, "Rear panel BAUD RATE switch")
	PORT_CONFSETTING(   0x00, "300")
	PORT_CONFSETTING(   0x80, "Preset")
	// jumpers on the I/O board
	PORT_CONFNAME(0x300, 0x300, "Word length")
	PORT_CONFSETTING(   0x000, "5 bits")
	PORT_CONFSETTING(   0x100, "6 bits")
	PORT_CONFSETTING(   0x200, "7 bits")
	PORT_CONFSETTING(   0x300, "8 bits")
	PORT_CONFNAME(0x400, 0x000, "Parity")
	PORT_CONFSETTING(   0x000, DEF_STR( None ))
	PORT_CONFSETTING(   0x400, "Generated")
	PORT_CONFNAME(0x800, 0x000, "Parity sense")
	PORT_CONFSETTING(   0x000, "Odd")
	PORT_CONFSETTING(   0x800, "Even")
INPUT_PORTS_END

ioport_constructor heath_h9_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(h9);
}


/***************************************************************************
    Machine configuration
***************************************************************************/

void heath_h9_device::device_add_mconfig(machine_config &config)
{
	config.set_default_layout(layout_heath_h9);

	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_raw(MASTER_CLOCK, CHARS_PER_SCAN_TOTAL * DOTS_PER_CHAR, 0, ROW_CHARS * DOTS_PER_CHAR, LINES_PER_FRAME, 0, VISIBLE_LINES);
	m_screen->set_screen_update(FUNC(heath_h9_device::screen_update));
	m_screen->set_palette("palette");

	// The raster is 640x192 but fills a 12" 4:3 tube. Without this, MAME
	// takes the aspect from the visible area and draws a wide, short strip.
	m_screen->set_physical_aspect(4, 3);

	// P4 (white) phosphor
	PALETTE(config, "palette", palette_device::MONOCHROME);

	AY31015(config, m_uart);
	m_uart->read_si_callback().set(FUNC(heath_h9_device::uart_si_r));
	m_uart->write_so_callback().set(FUNC(heath_h9_device::uart_so_w));
	m_uart->set_auto_rdav(true);

	CLOCK(config, m_uart_clock, MASTER_CLOCK / BAUD_4800_DIVISOR / 8);
	m_uart_clock->signal_handler().set(m_uart, FUNC(ay31015_device::write_tcp));
	m_uart_clock->signal_handler().append(m_uart, FUNC(ay31015_device::write_rcp));

	// 14 input, 8 output, 96 term mask PLA
	PLA(config, m_pla, 14, 8, 96);

	// Heath 443-767 at IC402. Clocked by the 4800 baud clock.
	// Scan rate based on debounce capacitor C405 (0.001 uF)
	MM5740AAE(config, m_kbdenc, mm5740_device::calc_effective_clock_key_debounce(1000));
	// X1 is not connected
	m_kbdenc->x_cb<2>().set(FUNC(heath_h9_device::kbd_x_r<2>));
	m_kbdenc->x_cb<3>().set(FUNC(heath_h9_device::kbd_x_r<3>));
	m_kbdenc->x_cb<4>().set(FUNC(heath_h9_device::kbd_x_r<4>));
	m_kbdenc->x_cb<5>().set(FUNC(heath_h9_device::kbd_x_r<5>));
	m_kbdenc->x_cb<6>().set(FUNC(heath_h9_device::kbd_x_r<6>));
	m_kbdenc->x_cb<7>().set(FUNC(heath_h9_device::kbd_x_r<7>));
	m_kbdenc->x_cb<8>().set(FUNC(heath_h9_device::kbd_x_r<8>));
	m_kbdenc->x_cb<9>().set(FUNC(heath_h9_device::kbd_x_r<9>));
	m_kbdenc->shift_cb().set(FUNC(heath_h9_device::kbd_shift_r));
	m_kbdenc->control_cb().set(FUNC(heath_h9_device::kbd_control_r));
	m_kbdenc->data_ready_cb().set(FUNC(heath_h9_device::kbd_data_ready_w));

	// the bell is the 300 baud clock gated to the speaker
	SPEAKER(config, "mono").front_center();
	BEEP(config, m_beep, MASTER_CLOCK / BAUD_4800_DIVISOR / 16);
	m_beep->add_route(ALL_OUTPUTS, "mono", 0.50);
}


// ROM definition
ROM_START( h9 )
	ROM_REGION( 0x0200, "chargen", 0 )
	ROM_LOAD( "443-812.ic205", 0x0000, 0x0200,          CRC(a7e567fc) SHA1(b18aae0a2d4f92f5a7e22640719bbc4652f3f4ee) ) // RO-3-2513

	ROM_REGION( 0x20, "ic724", 0 )
	ROM_LOAD( "444-12.ic724",    0x00,   0x20,          CRC(966204b4) SHA1(1d68987809dfa8047f288d8a95ddc1590d4f519b) )

	ROM_REGION( 0x20, "ic728", 0 )
	ROM_LOAD( "444-10.ic728",    0x00,   0x20,          CRC(ec1d81db) SHA1(d694ba9ffc7f3218e95b254aa37ea2996464520c) )

	ROM_REGION( 0x20, "ic730", 0 )
	ROM_LOAD( "444-11.ic730",    0x00,   0x20,          CRC(e0aed960) SHA1(3a1506a4a8a59e2c42579299dc9ff74443525acb) )

	// PLA not dumped, typed from the "ROM and PLA Programs" table in
	// Operation manual (595-2017-03, page 88). Has 14 input, 8 output,
	// 96 term mask PLA; its 16 programmed terms are stored as a JEDEC
	// binary fuse map with the unused terms left unprogrammed.
	ROM_REGION( 0x1b5, "ic702", 0 )
	ROM_LOAD( "443-787.ic702",  0x000,  0x1b5, BAD_DUMP CRC(7fa9b871) SHA1(1efe606e041b1091964da56f20319133f1aef2b3) )
ROM_END

const tiny_rom_entry *heath_h9_device::device_rom_region() const
{
	return ROM_NAME(h9);
}
