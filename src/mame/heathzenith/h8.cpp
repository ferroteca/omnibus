// license:BSD-3-Clause
// copyright-holders:Mark Garlanger
/***************************************************************************

    Heathkit H8

    This system uses Octal and Split-Octal rather than the usual hexadecimal.

    MACHINES
    --------
    Heath sold the H8 from 1977 to 1983, and the cards changed a great deal
    over that time.  Each machine here is one of the packaged systems from the
    Heathkit catalogs.  A package had fixed contents and one order number.

        h8        Kit H8 ($375, catalogs #817 to #840) and one H8-1 Memory
                  Board with 4K at 2000-2FFF.  There is no I/O card and no
                  terminal: the front panel is the only way in.

                  The H8 kit shipped with no RAM.  Catalogs #817, #819 and
                  #840 say it "Requires at least one H8-1 memory board", and
                  #840 says the H8 "will not operate without at least 4K of
                  optional memory".  The H8-1 came "supplied with 4K memory",
                  so this is the smallest H8 that runs.

                  HFS-800 (#842, 1978, $379, WHS-800 assembled $475) was the
                  H8 kit and the H-8-18 cassette software, with no memory, no
                  cassette interface and no terminal.  It has no machine of
                  its own.

        h8_system1  H8 System One, "the minimum recommended H8 system"
                  (catalog #817 $1239.75, #818 and #819 $1244.50; #840 calls
                  it the Minimum Complete System, $1210.75): H8, one H8-1 4K
                  Memory, one H8-3 4K Chip Set, H8-5 Serial I/O and Cassette
                  Interface, H9 Video Terminal and ECP-3801 Cassette
                  Recorder/Player.  The H8-3 fills the H8-1 out to 8K, at
                  2000-3FFF.  The H9 is on the H-8-5 at 600 baud.  8K runs
                  BASIC, HASL-8, TED-8 and BUG-8.

                  The later HKS-81 Hobbyist Computer System ($995, catalogs
                  #845 to #848) had the same cards and the H-8-18 cassette
                  software.  It has no machine of its own.

        h8_system2  H8 System Two, "a deluxe H8 system" (catalog #817
                  $1472.50, #818 and #819 $1477.25; #840 calls it the
                  Recommended System): H8, two H8-1 4K Memories, two H8-3 4K
                  Chip Sets, H8-5 Serial I/O and Cassette Interface, H9 Video
                  Terminal, ECP-3801 Cassette Player/Recorder and H8-13
                  Extended BASIC on cassette.  That is 16K, at 2000-5FFF.
                  16K runs everything on the cassette software list.  The
                  Cassette Operating System and Extended Benton Harbor BASIC
                  need 16K (catalog #853).

        h8_hks82  HKS-82 Advanced Computer System, $1995: H8, H-8-4 Four-Port
                  Serial I/O, WH-8-16 16K Memory, H-17 Floppy Disk, H19 CRT
                  Terminal and H-8-17 HDOS Software.  The Fall 1979 and
                  Christmas 1979 catalogs list these contents.  HDOS 1.x runs
                  in 16K.  HDOS 2.0 stops with "?01 HDOS REQUIRES AT LEAST
                  24K!".

                  WHS-83 (#848 and #849, 1980, $1495 and then $1595) was the
                  same cards assembled, with no terminal.  It has no machine of
                  its own.  Pick the terminal, or none, from the slot menu.

        h8_hks85  HKS-85 H-8 Hobbyist/Word Processing System, $1795, catalogs
                  #857 to #860 (1982): H8, three WH-8-16 16K Memory Boards
                  (48K), H-8-4, HA-8-8 Extended Configuration Option,
                  100K-byte H-17 and an H-19 Video Terminal.

        h8_wh837  Not a catalog package.  This is the most an H8 could hold
                  from Heath's own cards: an HA-8-6 Z80 CPU with PAM-37, 64K
                  on a WH-8-64 (32K and two HA-8-16 sets), a WH-8-37
                  soft-sectored controller with two drives, and an H19 on
                  H-8-4 port 4.  The WH-8-37 needs the HA-8-6, and catalog
                  #860 (1983) lists it.  The HA-8-6 switches the monitor ROM
                  out by itself, so the RAM under it is usable without an
                  HA-8-8.  It boots CP/M 2.2.04 for the H37.

    The terminal is on a second screen.  Show it with Video Options in
    settings.

    STATUS:
        It runs, keyboard works, you can enter data.
        Serial console works.

    Meaning of LEDs:
        PWR = Power is turned on (+5V is present at on front panel)
        MON = The front panel is being serviced by the cpu (controls should work)
        RUN = CPU is running (not halted)
        ION = Interrupts are enabled

    Pasting:
        H8    | mame key
    -----------------------
        0-F   | as is
        +     |   ^
        -     |   V
        MEM   |   -
        ALTER |   =

        Addresses must have all 6 digits entered. Data must have all 3 digits entered.
        System has a short beep for each key, and a slightly longer beep for each
        group of 3 digits. The largest number allowed is octal 377 (=256/0xFF).

    Test Paste:
        -041000=123 245 333 144 255 366 077=-041000
        Now press up-arrow to confirm the data has been entered.

    Official test program from pages 4 to 8 of the operator's manual:
        -040100=076 002 062 010 040 006 004 041 170 040 021 013 040 016 011 176
                022 043 023 015 302 117 040 016 003 076 377 315 053 000 015 302
                131 040 005 302 112 040 076 062 315 140 002 076 062 315 053 000
                076 062 315 140 002 303 105 040 377 262 270 272 275 377 222 200
                377 237 244 377 272 230 377 220 326 302 377 275 272 271 271 373
                271 240 377 236 376 362 236 376 362 236 376 362 R6=040100=4

****************************************************************************/

#include "emu.h"

#include "bus/heathzenith/h8/cards.h"
#include "bus/heathzenith/h8/h8bus.h"
#include "bus/heathzenith/intr_cntrl/intr_cntrl.h"
#include "bus/rs232/rs232.h"
#include "imagedev/floppy.h"

#include "natkeyboard.h"

#include <optional>

namespace {

class h8_state : public driver_device
{
public:
	h8_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_h8bus(*this, "h8bus")
		, m_p1(*this, "p1")
		, m_p2(*this, "p2")
		, m_p3(*this, "p3")
		, m_p8(*this, "p8")
		, m_p9(*this, "p9")
		, m_p10(*this, "p10")
		{}

	void h8(machine_config &config);
	void h8_system1(machine_config &config);
	void h8_system2(machine_config &config);
	void h8_hks82(machine_config &config);
	void h8_hks85(machine_config &config);
	void h8_wh837(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void device_config_complete() override  ATTR_COLD;

private:
	required_device<h8bus_device> m_h8bus;
	required_device<h8bus_slot_device> m_p1;
	required_device<h8bus_slot_device> m_p2;
	required_device<h8bus_slot_device> m_p3;
	required_device<h8bus_slot_device> m_p8;
	required_device<h8bus_slot_device> m_p9;
	required_device<h8bus_slot_device> m_p10;

	// enable the terminal's keyboard by default, not the front panel's
	bool m_terminal_keyboard = false;

	static void h_8_5_with_h9(device_t *device);
	static void h_8_4_with_h19(device_t *device);
	static void h_8_17_two_drives(device_t *device);
	static void ha_8_6_with_h37(device_t *device);
	static void wh_8_37_two_drives(device_t *device);
};

// Input ports
static INPUT_PORTS_START( h8 )
INPUT_PORTS_END

// Heath sold the H9 with the H8, two years before the H19.  H9 Operations
// (595-2017-03, page 10) gives the connection to the H8 as 600 baud, EIA
// levels, 8 bits and no parity, and Pictorial 2-2 there shows the H-8-5 with
// SER RX and SER TX both jumpered to 600 baud.
static DEVICE_INPUT_DEFAULTS_START( h8_console_600 )
	DEVICE_INPUT_DEFAULTS( "JUMPERS", 0x07, 0x03 )  // SER RX 600
	DEVICE_INPUT_DEFAULTS( "JUMPERS", 0x38, 0x18 )  // SER TX 600
DEVICE_INPUT_DEFAULTS_END

// The H-8-4 ships with port 4 off.  Put it at 350-357 octal, where HDOS
// looks for a console on this card.  Port 4's interrupt already
// defaults to level 3, which HDOS console input needs.
static DEVICE_INPUT_DEFAULTS_START( h8_h_8_4_console )
	DEVICE_INPUT_DEFAULTS( "JUMPERS4", 0x07, 0x04 )
DEVICE_INPUT_DEFAULTS_END

// The HKS-85 has 48K on three WH-8-16 boards.  Catalogs #857 and #858 (page
// 30) say "Included are the H-8 Computer, three WH-8-16 16K Memory Boards".
//
// The HA-8-8 manual (595-2509-1, in
// [https://sebhc.github.io/sebhc/documentation/hardware/H8/H8-8_As_Op_Sc.zip])
// sets the addresses.  Page 24, under "ROM DISABLE (ORG 0)", says "since the
// system RAM must be a continuous block originating at zero instead of 8k,
// your first RAM circuit board will be addressed at 000 instead of 040".  So
// the three boards are at 0000-BFFF, not at 2000-DFFF: one at 0000, one at
// 4000 and one at 8000.  CP/M needs RAM at 0000.  Page 24 also says HDOS
// still runs: "If you have less than 64k of memory, you can use HDOS Version
// 1.6".
//
// A WH-8-16 defaults to 2000.  Its SW1 sets address bits A12 to A15.
static DEVICE_INPUT_DEFAULTS_START( h8_wh_8_16_at_0000 )
	DEVICE_INPUT_DEFAULTS( "SW1", 0x02, 0x00 )  // A12
	DEVICE_INPUT_DEFAULTS( "SW1", 0x04, 0x00 )  // A13
	DEVICE_INPUT_DEFAULTS( "SW1", 0x08, 0x00 )  // A14
	DEVICE_INPUT_DEFAULTS( "SW1", 0x10, 0x00 )  // A15
DEVICE_INPUT_DEFAULTS_END

static DEVICE_INPUT_DEFAULTS_START( h8_wh_8_16_at_4000 )
	DEVICE_INPUT_DEFAULTS( "SW1", 0x02, 0x00 )
	DEVICE_INPUT_DEFAULTS( "SW1", 0x04, 0x00 )
	DEVICE_INPUT_DEFAULTS( "SW1", 0x08, 0x08 )
	DEVICE_INPUT_DEFAULTS( "SW1", 0x10, 0x00 )
DEVICE_INPUT_DEFAULTS_END

static DEVICE_INPUT_DEFAULTS_START( h8_wh_8_16_at_8000 )
	DEVICE_INPUT_DEFAULTS( "SW1", 0x02, 0x00 )
	DEVICE_INPUT_DEFAULTS( "SW1", 0x04, 0x00 )
	DEVICE_INPUT_DEFAULTS( "SW1", 0x08, 0x00 )
	DEVICE_INPUT_DEFAULTS( "SW1", 0x10, 0x10 )
DEVICE_INPUT_DEFAULTS_END

// The H-8-1 defaults to 2000 with 8K fitted.  The H8-1 was sold with 4K, and
// the H8-3 chip set filled it out to 8K.
static DEVICE_INPUT_DEFAULTS_START( h8_h_8_1_4k )
	DEVICE_INPUT_DEFAULTS( "CONFIG", 0x01, 0x00 )
DEVICE_INPUT_DEFAULTS_END

static DEVICE_INPUT_DEFAULTS_START( h8_h_8_1_at_4000 )
	DEVICE_INPUT_DEFAULTS( "JUMPERS", 0x07, 0x02 )
DEVICE_INPUT_DEFAULTS_END

void h8_state::h_8_5_with_h9(device_t *device)
{
	device->subdevice<rs232_port_device>("rs232")->set_default_option("h9");
}

// The H-8-4's ports are rs232_1 to rs232_4.  The terminal goes on port 4,
// where h8_h_8_4_console puts the console.  The H-8-4 has no rate jumper:
// the software sets each 8250's divisor.  HDOS measures the terminal's rate
// from the space bar, so the H19 can stay at its default 9600 baud.
void h8_state::h_8_4_with_h19(device_t *device)
{
	device->subdevice<rs232_port_device>("rs232_4")->set_default_option("h19");
}

// The H-17 cabinet held two drives.  A third needs the H-17-3 modification
// kit: new sheet metal, a longer cable and a bigger fan.  So floppy2 is
// empty.  Pick a drive for it from the slot menu to have three.
void h8_state::h_8_17_two_drives(device_t *device)
{
	device->subdevice<floppy_connector>("floppy2")->set_default_option(nullptr);
}

// Set the HA-8-8 boot mode switch (SW1:8) to Auto, so XCON8 boots the disk.
// At Normal, XCON8 waits for a command from the front panel and prints
// nothing, so the terminal stays blank.
static DEVICE_INPUT_DEFAULTS_START( h8_ha_8_8_auto )
	DEVICE_INPUT_DEFAULTS( "SW1", 0x80, 0x80 )
DEVICE_INPUT_DEFAULTS_END

// Open jumper X1-X2 on the CPU board, as Heath shipped it.  With X1-X2 open,
// /ROM DISABLE from the bus cannot switch out the monitor ROM at 0000-0FFF.
// cpu8080.cpp closes X1-X2 by default, so that an HA-8-8 can disable the ROM
// without a settings change.  Only h8_hks85 has an HA-8-8, and no other
// card drives /ROM DISABLE, so the other machines with the 8080 CPU board
// open the jumper to match the hardware.  h8_hks85 closes it again below.
// h8_wh837 has the HA-8-6 instead, which has no X1-X2.
//
// With X1-X2 open, CP/M does not boot and prints nothing.  It writes its
// jumps to 0000 and 0005, the writes go to ROM and are lost, and the warm
// boot jumps into monitor code.
static DEVICE_INPUT_DEFAULTS_START( h8_cpu_8080 )
	DEVICE_INPUT_DEFAULTS( "CONFIG", 0x04, 0x00 )
DEVICE_INPUT_DEFAULTS_END

// The HA-8-8 ROM disable modification on the H-8-17 (595-2509-1, pages 24
// and 25, Pictorials 2-3 and 2-4): R6 and C15 come off the board and a bare
// wire goes across C15's foils.  The card then no longer answers for
// 1400-1FFF.  h_8_17.cpp explains the circuit.
static DEVICE_INPUT_DEFAULTS_START( h8_h_8_17_org0 )
	DEVICE_INPUT_DEFAULTS( "CONFIG", 0x01, 0x01 )
DEVICE_INPUT_DEFAULTS_END

// The same modification on the CPU board (595-2509-1, page 26, Pictorial
// 2-5) cuts the jumpers at T1-T2, R1-R2, S2-S3, P2-P3 and Z2-Z3, adds wires
// P1-P2 and Z1-Z2, and then says "Solder a 3/4" bare wire from hole X1 to
// hole X2".  Page 27 fits the XCON8 ROM (444-70) at IC204 on the same board.
static DEVICE_INPUT_DEFAULTS_START( h8_cpu_8080_ha_8_8 )
	DEVICE_INPUT_DEFAULTS( "CONFIG", 0x04, 0x04 )
DEVICE_INPUT_DEFAULTS_END

// MAME enables the first keyboard by tag, which is the front panel in P1.
// On a machine with a terminal, the terminal is the keyboard in use, so enable
// the terminal's keyboard and disable the front panel's.  A saved cfg file
// loads after this and overrides it.
void h8_state::machine_start()
{
	if (!m_terminal_keyboard)
		return;

	natural_keyboard &natkbd = machine().natkeyboard();
	device_h8bus_card_interface *const card = m_p1->get_card_device();
	device_t *const panel = card ? &card->device() : nullptr;

	std::optional<size_t> panel_index, terminal_index;
	for (size_t i = 0; natkbd.keyboard_count() > i; ++i)
	{
		if (&natkbd.keyboard_device(i) == panel)
			panel_index = i;
		else if (!terminal_index && !natkbd.keyboard_is_keypad(i))
			terminal_index = i;
	}

	// leave the front panel enabled when no terminal keyboard is fitted
	if (panel_index && terminal_index)
	{
		natkbd.disable_keyboard(*panel_index);
		natkbd.enable_keyboard(*terminal_index);
	}
}

void h8_state::device_config_complete()
{
	// Connect up the p201 cable between p1 and p2 boards. This is separate from the h8bus.
	auto p1_lookup = m_p1.lookup()->get_card_device();
	auto p2_lookup = m_p2.lookup()->get_card_device();

	// avoid crash when there isn't a card installed in either slot.
	if (p1_lookup && p2_lookup)
	{
		device_p201_p1_card_interface *p1 = dynamic_cast<device_p201_p1_card_interface *>(p1_lookup);
		device_p201_p2_card_interface *p2 = dynamic_cast<device_p201_p2_card_interface *>(p2_lookup);

		p1->p201_reset_cb().set(*p2, FUNC(device_p201_p2_card_interface::p201_reset_w));
		p1->p201_int1_cb().set(*p2, FUNC(device_p201_p2_card_interface::p201_int1_w));
		p1->p201_int2_cb().set(*p2, FUNC(device_p201_p2_card_interface::p201_int2_w));

		p2->p201_inte_cb().set(*p1, FUNC(device_p201_p1_card_interface::p201_inte_w));
	}
}

void h8_state::h8(machine_config &config)
{
	H8BUS(config, m_h8bus);

	H8BUS_SLOT(config,  m_p1, "h8bus", h8_p1_cards,  "fp");
	H8BUS_SLOT(config,  m_p2, "h8bus", h8_p2_cards,  "cpu8080");

	// One H-8-1 as sold, with 4K at 2000-2FFF.
	H8BUS_SLOT(config,  m_p3, "h8bus", h8_cards,     "h_8_1");
	m_p3->set_option_device_input_defaults("h_8_1", DEVICE_INPUT_DEFAULTS_NAME(h8_h_8_1_4k));

	H8BUS_SLOT(config,  "p4", "h8bus", h8_cards,     nullptr);
	H8BUS_SLOT(config,  "p5", "h8bus", h8_cards,     nullptr);
	H8BUS_SLOT(config,  "p6", "h8bus", h8_cards,     nullptr);
	H8BUS_SLOT(config,  "p7", "h8bus", h8_cards,     nullptr);
	H8BUS_SLOT(config,  m_p8, "h8bus", h8_cards,     nullptr);
	H8BUS_SLOT(config,  m_p9, "h8bus", h8_cards,     nullptr);

	// Heath released the HA-8-8 late in 1980.  Of the packages here, only
	// the HKS-85 has one.
	H8BUS_SLOT(config, m_p10, "h8bus", h8_p10_cards, nullptr);

	m_p2->set_option_device_input_defaults("cpu8080", DEVICE_INPUT_DEFAULTS_NAME(h8_cpu_8080));
}

void h8_state::h8_system1(machine_config &config)
{
	h8(config);

	// The H-8-3 chip set fills the H-8-1 out to 8K, at 2000-3FFF.
	H8BUS_SLOT(config.replace(), m_p3, "h8bus", h8_cards, "h_8_1");

	// The H-8-5 is the tape interface and the console, with an H9 at 600 baud.
	H8BUS_SLOT(config.replace(), m_p9, "h8bus", h8_cards, "h_8_5");
	m_p9->set_option_machine_config("h_8_5", h_8_5_with_h9);
	m_p9->set_option_device_input_defaults("h_8_5", DEVICE_INPUT_DEFAULTS_NAME(h8_console_600));

	m_terminal_keyboard = true;
}

void h8_state::h8_system2(machine_config &config)
{
	h8_system1(config);

	// A second H-8-1 with its H-8-3, at 4000-5FFF.
	H8BUS_SLOT(config.replace(), "p4", "h8bus", h8_cards, "h_8_1")
		.set_option_device_input_defaults("h_8_1", DEVICE_INPUT_DEFAULTS_NAME(h8_h_8_1_at_4000));
}

void h8_state::h8_hks82(machine_config &config)
{
	h8(config);

	// One WH-8-16 in place of the H-8-1.  Its switches default to 2000-5FFF.
	H8BUS_SLOT(config.replace(), m_p3, "h8bus", h8_cards, "wh_8_16");

	// The H-17 operation manual puts the controller near the back of the
	// bus.  P8 is the last free slot before the serial card in P9.
	H8BUS_SLOT(config.replace(), m_p8, "h8bus", h8_cards, "h_8_17");
	m_p8->set_option_machine_config("h_8_17", h_8_17_two_drives);

	// This package has no H-8-5.  The console is on a port of the H-8-4.
	// The H-8-4's ports can only be at 320, 330, 340 or 350 octal.  The H-17
	// boot ROM prints to 372-373, so the screen stays blank until HDOS starts
	// its own console.  HDOS then waits for the space bar before it prints
	// "ACTION? <BOOT>".
	H8BUS_SLOT(config.replace(), m_p9, "h8bus", h8_cards, "h_8_4");
	m_p9->set_option_machine_config("h_8_4", h_8_4_with_h19);
	m_p9->set_option_device_input_defaults("h_8_4", DEVICE_INPUT_DEFAULTS_NAME(h8_h_8_4_console));

	// Standard PAM-8 cannot boot a disk.  The H-17 operation manual says to
	// enter 030 000 in the PC and press GO.  PAM-8/GO presets the PC to
	// 030 000, so a boot is one key press.
	m_p2->set_option_default_bios("cpu8080", "pam8go");

	m_terminal_keyboard = true;
}

void h8_state::h8_hks85(machine_config &config)
{
	h8_hks82(config);

	// Three WH-8-16 boards from 0000.  The RAM under the monitor ROM is only
	// usable when a card asserts /ROM DISABLE, and the HA-8-8 does that.
	m_p3->set_option_device_input_defaults("wh_8_16", DEVICE_INPUT_DEFAULTS_NAME(h8_wh_8_16_at_0000));
	H8BUS_SLOT(config.replace(), "p4", "h8bus", h8_cards, "wh_8_16")
		.set_option_device_input_defaults("wh_8_16", DEVICE_INPUT_DEFAULTS_NAME(h8_wh_8_16_at_4000));
	H8BUS_SLOT(config.replace(), "p5", "h8bus", h8_cards, "wh_8_16")
		.set_option_device_input_defaults("wh_8_16", DEVICE_INPUT_DEFAULTS_NAME(h8_wh_8_16_at_8000));

	H8BUS_SLOT(config.replace(), m_p10, "h8bus", h8_p10_cards, "ha_8_8");
	m_p10->set_option_device_input_defaults("ha_8_8", DEVICE_INPUT_DEFAULTS_NAME(h8_ha_8_8_auto));

	// The HA-8-8 has three functions: boot device selection, the status port
	// and ROM disable.  Page 22 of its manual (595-2509-1) tells the owner to
	// pick the functions and make the board modifications for them.  This
	// machine takes ROM disable (pages 24 to 27).  ROM disable needs three
	// changes:
	//   - the H-17 controller no longer answers for 1400-1FFF
	//     (h8_h_8_17_org0)
	//   - jumper X1-X2 is closed on the CPU board (h8_cpu_8080_ha_8_8)
	//   - the memory starts at 0000, not 2000 (h8_wh_8_16_at_0000)
	// They are settings, and do not follow from the cards fitted, because an
	// owner who bought the HA-8-8 for the other functions made none of them.
	//
	// The three changes only work together.  Without RAM at 1400-1FFF the
	// modified H-17 leaves a hole, and without X1-X2 the RAM under the ROM
	// cannot be reached.  With all three, CP/M runs.
	m_p2->set_option_device_input_defaults("cpu8080", DEVICE_INPUT_DEFAULTS_NAME(h8_cpu_8080_ha_8_8));
	m_p8->set_option_device_input_defaults("h_8_17", DEVICE_INPUT_DEFAULTS_NAME(h8_h_8_17_org0));

	// With the HA-8-8 fitted, the monitor can boot the disk itself.  XCON8
	// is the Heath part (444-70), and its image says so.  PAM-8/AT is not a
	// dump: it was assembled from a listing dated 27-Oct-81, and its text says
	// the original part number is unknown.
	m_p2->set_option_default_bios("cpu8080", "xcon8");
}

// Set PAM-37 to boot the device at port 170 by itself.  "Port 170 device"
// already defaults to H37, and "Primary Boot from" to port 170.
static DEVICE_INPUT_DEFAULTS_START( h8_ha_8_6_auto )
	DEVICE_INPUT_DEFAULTS( "SW1", 0x80, 0x80 )
DEVICE_INPUT_DEFAULTS_END

// The WH-8-37 takes over the HA-8-6's interrupt encoder socket, U38.
// wh_8_37.cpp explains why floppy transfers hang without this.
void h8_state::ha_8_6_with_h37(device_t *device)
{
	device->subdevice<heath_intr_socket>("intr_socket")->set_default_option("h37");
}

// Two drives, as on the HKS-82 and HKS-85.  The connectors are z37_fdc:0 to 3.
void h8_state::wh_8_37_two_drives(device_t *device)
{
	device->subdevice<floppy_connector>("z37_fdc:2")->set_default_option(nullptr);
}

void h8_state::h8_wh837(machine_config &config)
{
	h8(config);

	H8BUS_SLOT(config.replace(), m_p2, "h8bus", h8_p2_cards, "ha_8_6");
	m_p2->set_option_default_bios("ha_8_6", "pam37");
	m_p2->set_option_machine_config("ha_8_6", ha_8_6_with_h37);
	m_p2->set_option_device_input_defaults("ha_8_6", DEVICE_INPUT_DEFAULTS_NAME(h8_ha_8_6_auto));

	// The WH-8-64's switches default to all four banks, 0000-FFFF.
	H8BUS_SLOT(config.replace(), m_p3, "h8bus", h8_cards, "wh_8_64");

	H8BUS_SLOT(config.replace(), m_p8, "h8bus", h8_cards, "wh_8_37");
	m_p8->set_option_machine_config("wh_8_37", wh_8_37_two_drives);

	// CP/M's BIOS looks for its console at 350 octal.
	H8BUS_SLOT(config.replace(), m_p9, "h8bus", h8_cards, "h_8_4");
	m_p9->set_option_machine_config("h_8_4", h_8_4_with_h19);
	m_p9->set_option_device_input_defaults("h_8_4", DEVICE_INPUT_DEFAULTS_NAME(h8_h_8_4_console));

	m_terminal_keyboard = true;
}

// ROM definition
ROM_START( h8 )
ROM_END

#define rom_h8_system1 rom_h8
#define rom_h8_system2 rom_h8
#define rom_h8_hks82 rom_h8
#define rom_h8_hks85 rom_h8
#define rom_h8_wh837 rom_h8

} // anonymous namespace

// Driver

//    YEAR  NAME        PARENT  COMPAT  MACHINE     INPUT  CLASS,    INIT        COMPANY          FULLNAME                                      FLAGS
COMP( 1977, h8,         0,      0,      h8,         h8,    h8_state, empty_init, "Heath Company", "H-8",                                         MACHINE_SUPPORTS_SAVE )
COMP( 1977, h8_system1, h8,     0,      h8_system1, h8,    h8_state, empty_init, "Heath Company", "H-8 System One",                              MACHINE_SUPPORTS_SAVE )
COMP( 1977, h8_system2, h8,     0,      h8_system2, h8,    h8_state, empty_init, "Heath Company", "H-8 System Two",                              MACHINE_SUPPORTS_SAVE )
COMP( 1979, h8_hks82,   h8,     0,      h8_hks82,   h8,    h8_state, empty_init, "Heath Company", "H-8 HKS-82 Advanced Computer System",         MACHINE_SUPPORTS_SAVE )
COMP( 1982, h8_hks85,   h8,     0,      h8_hks85,   h8,    h8_state, empty_init, "Heath Company", "H-8 HKS-85 Hobbyist/Word Processing System",  MACHINE_SUPPORTS_SAVE )
COMP( 1983, h8_wh837,   h8,     0,      h8_wh837,   h8,    h8_state, empty_init, "Heath Company", "H-8 with WH-8-37",                            MACHINE_SUPPORTS_SAVE )
