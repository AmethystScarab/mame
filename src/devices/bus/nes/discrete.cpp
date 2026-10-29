// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
/***********************************************************************************************************


 NES/Famicom cartridge emulation for PCBs mostly based on discrete components

 iNES mappers: 11, 38, 70, 87, 152

 Here we emulate the following PCBs

 * PCB with IC 74x161x161x32 [mapper 70 & 152]
 * PCB with IC 74x139x74 [mapper 87]
 * PCB with IC 74x377 [mapper 11]
 * PCB with IC 74x161x138 [mapper 38]

 ***********************************************************************************************************/


#include "emu.h"
#include "discrete.h"


#ifdef NES_PCB_DEBUG
#define VERBOSE (LOG_GENERAL)
#else
#define VERBOSE (0)
#endif
#include "logmacro.h"


//-------------------------------------------------
//  constructor
//-------------------------------------------------

DEFINE_DEVICE_TYPE(NES_74X161X161X32, nes_74x161x161x32_device, "nes_74x161",      "NES Cart Discrete Logic (74*161/161/32) PCB")
DEFINE_DEVICE_TYPE(NES_74X139X74,     nes_74x139x74_device,     "nes_74x139",      "NES Cart Discrete Logic (74*139/74) PCB")
DEFINE_DEVICE_TYPE(NES_74X377,        nes_74x377_device,        "nes_74x377",      "NES Cart Discrete Logic (74*377) PCB")
DEFINE_DEVICE_TYPE(NES_74X161X138,    nes_74x161x138_device,    "nes_bitcorp_dis", "NES Cart Discrete Logic (74*161/138) PCB")


nes_74x161x161x32_device::nes_74x161x161x32_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: nes_nrom_device(mconfig, NES_74X161X161X32, tag, owner, clock)
{
}

nes_74x139x74_device::nes_74x139x74_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: nes_nrom_device(mconfig, NES_74X139X74, tag, owner, clock)
{
}

nes_74x377_device::nes_74x377_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: nes_nrom_device(mconfig, NES_74X377, tag, owner, clock)
{
}

nes_74x161x138_device::nes_74x161x138_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: nes_nrom_device(mconfig, NES_74X161X138, tag, owner, clock)
{
}




void nes_74x161x161x32_device::pcb_reset()
{
	prg16_89ab(0);
	prg16_cdef(m_prg_chunks - 1);
	chr8(0, m_chr_source);
	if (m_pcb_ctrl_mirror)
		set_nt_mirroring(PPU_MIRROR_LOW);
}




/*-------------------------------------------------
 mapper specific handlers
 -------------------------------------------------*/

/*-------------------------------------------------

 Discrete Logic board IC 74x161x161x32

 There are two variants (one with hardwired mirroring, the
 other with a mirroring control), making necessary two distinct
 mappers & pcb_id

 The register is at 0x8000-0xffff and the last 16K of PRG is fixed at 0xc000.
 Mapper 70 is PPPP CCCC (16K PRG, 8K CHR). Mapper 152 is MPPP CCCC, where M
 selects the one-screen mirroring page and PRG has only 3 bits.
 The real boards are subject to bus conflict.

 iNES: mappers 70 & 152

 -------------------------------------------------*/

// there are two 'variants' depending on hardwired or mapper ctrl mirroring
void nes_74x161x161x32_device::write_h(offs_t offset, u8 data)
{
	LOG("74x161x161x32 write_h, offset: %04x, data: %02x\n", offset, data);

	// this pcb is subject to bus conflict
	data = account_bus_conflict(offset, data);

	if (m_pcb_ctrl_mirror)
		set_nt_mirroring(BIT(data, 7) ? PPU_MIRROR_HIGH : PPU_MIRROR_LOW);
	chr8(data & 0x0f, CHRROM);
	prg16_89ab((data >> 4) & (m_pcb_ctrl_mirror ? 0x07 : 0x0f));
}

/*-------------------------------------------------

 Discrete Logic board IC 74x139x74 by Konami & Jaleco

 The register at 0x6000-0x7fff is xxxx xxLH and selects the 8K CHR bank
 as H*2 + L (the two bits are swapped). There are no bus conflicts.

 iNES: mapper 87

 -------------------------------------------------*/

void nes_74x139x74_device::write_m(offs_t offset, u8 data)
{
	LOG("74x139x74 write_m, offset: %04x, data: %02x\n", offset, data);

	chr8(bitswap<2>(data, 0, 1), CHRROM);
}

/*-------------------------------------------------

 Discrete Logic board IC 74x377 by Color Dreams / Nina-007 emulation

 Games: many Color Dreams and Wisdom Tree titles

 The register is CCCC LLPP (CHR bank, lockout defeat, PRG bank). The real boards
 are subject to bus conflict: NES 2.0 submapper 2 enables them and submapper 1
 disables them, while iNES images have none, because of the Free Fall prototype.

 iNES: mapper 11

 In MAME: Supported

 Note: bit2 & bit3 are actually related to CIC lockout
 defeating, and real Color Dreams titles use only
 bit0 & bit1 for PRG switching. Our usage allows for support
 of extended PRG games (e.g. homebrew)

 -------------------------------------------------*/

void nes_74x377_device::write_h(offs_t offset, u8 data)
{
	LOG("74x377 write_h, offset: %04x, data: %02x\n", offset, data);

	// this pcb is subject to bus conflict, but not the prototype of Secret Scout, which actually breaks in case of conflict...
	data = account_bus_conflict(offset, data);

	chr8(data >> 4, m_chr_source);
	prg32(data & 0x0f);
}

/*-------------------------------------------------

 Discrete Logic board IC 74x161x138

 Games: Crime Busters

 The register is xxxx CCPP and is decoded only at 0x7000-0x7fff. There are no
 bus conflicts and mirroring is hardwired. We do not mask the bank numbers, to
 allow for oversize ROMs like other emulators do.

 iNES: mapper 38

 -------------------------------------------------*/

void nes_74x161x138_device::write_m(offs_t offset, u8 data)
{
	LOG("74x161x138 write_m, offset: %04x, data: %02x\n", offset, data);

	if (offset < 0x1000)    // the register is not decoded at 0x6000-0x6fff
		return;

	chr8(data >> 2, CHRROM);
	prg32(data);
}
