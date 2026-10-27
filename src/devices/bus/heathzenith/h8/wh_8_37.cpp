// license:BSD-3-Clause
// copyright-holders:Paul Galbraith
/***************************************************************************

  Heath WH-8-37 Double-Density Disk Controller and Z-67 Interface

    The real card holds two controllers: the Z-37 soft-sectored floppy
    controller, for up to four H-17-1 or H-17-4 drives, and the host end of
    the Z-67 Winchester. Each half has a jumper to turn it off. Only the
    floppy half is modelled here. The card needs the HA-8-6 Z80 CPU board.
    Heath's catalog #860 (1983) lists it at $395 assembled. The quotations,
    jumper names and part numbers here come from its manual, 595-2859, and
    the schematic that comes with it
    [https://sebhc.github.io/sebhc/documentation/hardware/H8/H8-37_Op_Sc.zip].

    Software: HDOS 2.0 with the HOS-5-UP update drives the floppy half only.
    CP/M 2.2.03 and later drive both halves. PAM-37, the HA-8-6 monitor that
    comes with the card, boots from either.

    SET THE HA-8-6 INTERRUPT SOCKET TO h37, OR FLOPPY TRANSFERS HANG
    -----------------------------------------------------------------
    The floppy half runs its transfers through the CPU board's interrupt
    logic, as the Z-89-37 does on the H89. The manual's installation steps
    take the 74LS148 priority encoder out of U38 on the HA-8-6, and plug a
    16-conductor cable from this card into the empty socket. U37, U43 and the
    444-82 PAL U42 on this card then do the encoder's job. With the
    controller's DRQ enabled, that logic "blocks all interrupts to the
    processor (except its own)". It answers a DRQ with an EI, so the
    monitor's HALT loop reads the next byte. It raises level 4 when a command
    ends: "INT 4 is used by the Z-37 when running". PAM-37's read loop shows
    the HALT method: it copies HALT, IN A,(173Q), LD (DE),A, INC DE, JP (HL)
    into RAM and runs it there.

    The HA-8-6's interrupt socket models U38, and its "h37" option models
    this card's logic. Without "h37" the card still answers, but the first
    floppy transfer hangs at the HALT. The 444-82 has not been dumped, so the
    H89's Z-37 interrupt controller stands in for it.

    The same steps move the HA-8-6's 444-70 ROM to U19 and put PAM-37
    (444-140) in U13. ha_8_6.cpp has the detail.

    WHERE IT ANSWERS
    ----------------
    U14 decodes one block of eight ports. Two jumpers pick its base: "port
    base address 170 octal (normal)" or 270. The 444-117 PAL U26 splits the
    block between the two halves, using A2 and the two enable jumpers. The
    manual gives the result, not the equations. With both halves in use "the
    Z-37 device must be assigned to port 170 and the Z-67 device assigned to
    port 174". With only one, "either may be assigned to port 170". So the
    floppy half takes the first four ports of the block. It decodes only A0
    and A1 within them.

    WHAT IS NOT MODELLED
    --------------------
    The Z-67 half, with its jumper and its INT jumper. The PARITY and PARITY
    ERROR jumpers, which belong to the Z-67 half. PRE COMP, a write
    precompensation jumper: MAME's floppy layer has nothing for it to change.
    The 8-section DIP switch DS1, read through the 444-117's RD SWITCH
    strobe at a port only the undumped PAL knows. The manual says "they are
    not currently used by Heath or Zenith Data Systems software".

****************************************************************************/

#include "emu.h"

#include "wh_8_37.h"

#include "bus/heathzenith/h8x_common/z37_fdc_base.h"
#include "bus/heathzenith/intr_cntrl/intr_cntrl.h"

#include "softlist_dev.h"

#define LOG_SETUP (1U << 1)

#define VERBOSE (0)

#include "logmacro.h"

#define LOGSETUP(...)  LOGMASKED(LOG_SETUP, __VA_ARGS__)


namespace {

class wh_8_37_device : public heath_z37_fdc_base_device
					 , public device_h8bus_card_interface
{
public:

	wh_8_37_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 0);

protected:

	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;

	virtual void map_io(address_space_installer &space) override ATTR_COLD;

	virtual void set_irq_out(int state) override;
	virtual void set_drq_out(int state) override;
	virtual void block_interrupts_out(int state) override;

	required_ioport m_jumpers;

	// the CPU board's interrupt socket, U38 on the HA-8-6
	heath_intr_socket *m_intr_socket;

	// first of the floppy half's four ports, 0 while it is not answering
	u8 m_z37_port;
};


wh_8_37_device::wh_8_37_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: heath_z37_fdc_base_device(mconfig, H8BUS_WH_8_37, tag, owner, 0)
	, device_h8bus_card_interface(mconfig, *this)
	, m_jumpers(*this, "JUMPERS")
	, m_intr_socket(nullptr)
{
}

void wh_8_37_device::set_irq_out(int state)
{
	if (m_intr_socket)
	{
		m_intr_socket->set_irq(state);
	}
}

void wh_8_37_device::set_drq_out(int state)
{
	if (m_intr_socket)
	{
		m_intr_socket->set_drq(state);
	}
}

void wh_8_37_device::block_interrupts_out(int state)
{
	if (m_intr_socket)
	{
		m_intr_socket->block_interrupts(state);
	}
}

void wh_8_37_device::device_start()
{
	heath_z37_fdc_base_device::device_start();

	m_z37_port = 0;

	save_item(NAME(m_z37_port));
}

void wh_8_37_device::device_reset()
{
	// Every card has registered with the bus by now, so the CPU board can be
	// found. An 8080 CPU board has no socket. The floppy half's interrupts
	// then go nowhere, as on the real card, which needs the HA-8-6.
	m_intr_socket = h8bus().cpu_intr_socket();

	if (!m_intr_socket)
	{
		logerror("no CPU board interrupt socket found; the WH-8-37 needs the HA-8-6\n");
	}
	else if (!dynamic_cast<z37_intr_cntrl *>(m_intr_socket->get_card_device()))
	{
		logerror("%s is not set to h37; floppy transfers will hang\n", m_intr_socket->tag());
	}

	// The jumpers are read in map_io(), which the CPU card runs from its own
	// reset. They are not read here, because nothing orders the two resets.
	heath_z37_fdc_base_device::device_reset();
}

void wh_8_37_device::map_io(address_space_installer &space)
{
	ioport_value const jumpers(m_jumpers->read());

	u8 const base(BIT(jumpers, 0) ? 0xb8 : 0x78);
	u8 const z37_port(BIT(jumpers, 1) ? base : 0);

	// Unmap the old ports only if they moved. Another card's map_io() may
	// already have put its ports where this card used to be.
	if (m_z37_port && (m_z37_port != z37_port))
	{
		space.unmap_readwrite(m_z37_port, m_z37_port + 3);
	}

	m_z37_port = z37_port;

	if (m_z37_port)
	{
		LOGSETUP("Z-37 at %03o\n", m_z37_port);

		space.install_readwrite_handler(m_z37_port, m_z37_port + 3,
			read8sm_delegate(*this, FUNC(wh_8_37_device::read)),
			write8sm_delegate(*this, FUNC(wh_8_37_device::write)));
	}
}

void wh_8_37_device::device_add_mconfig(machine_config &config)
{
	heath_z37_fdc_base_device::device_add_mconfig(config);

	// the same disks as the H89's Z-89-37
	SOFTWARE_LIST(config, "flop_list").set_original("h37_flop");
}

static INPUT_PORTS_START( wh_8_37_jumpers )

	PORT_START("JUMPERS")
	PORT_CONFNAME(0x01, 0x00, "Port base address")
	PORT_CONFSETTING(   0x00, "170 octal")
	PORT_CONFSETTING(   0x01, "270 octal")
	PORT_CONFNAME(0x02, 0x02, "Z-37 jumper")
	PORT_CONFSETTING(   0x00, DEF_STR( Off ))
	PORT_CONFSETTING(   0x02, DEF_STR( On ))

INPUT_PORTS_END

ioport_constructor wh_8_37_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(wh_8_37_jumpers);
}

} // anonymous namespace

DEFINE_DEVICE_TYPE_PRIVATE(H8BUS_WH_8_37, device_h8bus_card_interface, wh_8_37_device, "h8_wh_8_37", "Heath WH-8-37 Double-Density Disk Controller and Z-67 Interface");
