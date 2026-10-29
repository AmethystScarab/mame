// license:BSD-3-Clause
// copyright-holders:kmg
/***********************************************************************************************************


 NES/Famicom cartridge emulation for Sealie Computing and related PCBs

 iNES mappers: 29, 30, 409


 Here we emulate the following homebrew PCBs

 * SEALIE RET-CUFROM [mapper 29]
 * SEALIE DPCMcart [mapper 409]
 * SEALIE UNROM 512 [mapper 30]
 * SEALIE 8BIT XMAS [mapper 30]

 ***********************************************************************************************************/


#include "emu.h"
#include "sealie.h"


#ifdef NES_PCB_DEBUG
#define VERBOSE (LOG_GENERAL)
#else
#define VERBOSE (0)
#endif
#include "logmacro.h"


//-------------------------------------------------
//  constructor
//-------------------------------------------------

DEFINE_DEVICE_TYPE(NES_8BITXMAS, nes_8bitxmas_device, "nes_8bitxmas", "NES Cart Sealie 8BIT XMAS PCB")
DEFINE_DEVICE_TYPE(NES_CUFROM,   nes_cufrom_device,   "nes_cufrom",   "NES Cart Sealie RET-CUFROM PCB")
DEFINE_DEVICE_TYPE(NES_DPCMCART, nes_dpcmcart_device, "nes_dpcmcart", "NES Cart Sealie DPCMcart PCB")
DEFINE_DEVICE_TYPE(NES_UNROM512, nes_unrom512_device, "nes_unrom512", "NES Cart Sealie UNROM 512 PCB")


nes_cufrom_device::nes_cufrom_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: nes_nrom_device(mconfig, NES_CUFROM, tag, owner, clock)
{
}

nes_dpcmcart_device::nes_dpcmcart_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: nes_nrom_device(mconfig, NES_DPCMCART, tag, owner, clock)
{
}

nes_unrom512_device::nes_unrom512_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock)
	: nes_nrom_device(mconfig, type, tag, owner, clock), device_nvram_interface(mconfig, *this)
{
}

nes_unrom512_device::nes_unrom512_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: nes_unrom512_device(mconfig, NES_UNROM512, tag, owner, clock)
{
}

nes_8bitxmas_device::nes_8bitxmas_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: nes_unrom512_device(mconfig, NES_8BITXMAS, tag, owner, clock), m_led(0)
{
}



void nes_cufrom_device::pcb_reset()
{
	prg16_89ab(0);
	prg16_cdef(m_prg_chunks - 1);
	chr8(0, CHRRAM);
}

void nes_dpcmcart_device::pcb_reset()
{
	prg16_89ab(0);
	prg16_cdef(m_prg_chunks - 1);
}

void nes_unrom512_device::device_start()
{
	nes_nrom_device::device_start();
	save_item(NAME(m_latch));
	save_item(NAME(m_flash_step));
	save_item(NAME(m_flash_cmd));
}

void nes_unrom512_device::pcb_reset()
{
	prg16_89ab(0);
	prg16_cdef(m_prg_chunks - 1);
	chr8(0, CHRRAM);
	if (m_pcb_ctrl_mirror)
		set_nt_mirroring(PPU_MIRROR_LOW);
	m_latch = 0;
	m_flash_step = 0;

	// NESdev: the four-screen variant's nametables are the last 8 KiB of CHR-RAM ($2000-$2FFF; the $3000-$3EFF scratch
	// area is not separate here, it mirrors the nametables as MAME's PPU map does)
	if (get_mirroring() == PPU_MIRROR_4SCREEN && get_vram_size() >= 0x2000)
	{
		const int base = (get_vram_size() - 0x2000) / 0x400;
		for (int page = 0; page < 4; page++)
			set_nt_page(page, VRAM_NT, base + page, 1);
	}
}

// SST39SF0x0 command sequences (NESdev UNROM 512 "Flash ROM Save"): the flash address is the current $8000 bank
// (latch bits 0-4) with CPU A0-A13. Program and erase complete at once, so polling reads return the final data
void nes_unrom512_device::flash_w(offs_t offset, u8 data)
{
	const u32 size = m_prg_chunks * 0x4000;
	const u32 addr = (((m_latch & 0x1f) << 14) | (offset & 0x3fff)) & (size - 1);
	const u16 cmd = addr & 0x7fff;
	u8 *prg = get_prg_base();

	if (data == 0xf0)
	{
		m_flash_step = 0;   // reset / exit
		return;
	}
	switch (m_flash_step)
	{
		case 0: m_flash_step = (cmd == 0x5555 && data == 0xaa) ? 1 : 0; break;
		case 1: m_flash_step = (cmd == 0x2aaa && data == 0x55) ? 2 : 0; break;
		case 2:
			if (cmd == 0x5555 && (data == 0xa0 || data == 0x80))
			{
				m_flash_cmd = data;
				m_flash_step = (data == 0xa0) ? 3 : 4;
			}
			else
				m_flash_step = 0;
			break;
		case 3:   // byte program: a flash cell can only change 1s to 0s
			prg[addr] &= data;
			m_flash_step = 0;
			break;
		case 4: m_flash_step = (cmd == 0x5555 && data == 0xaa) ? 5 : 0; break;
		case 5: m_flash_step = (cmd == 0x2aaa && data == 0x55) ? 6 : 0; break;
		case 6:
			if (data == 0x30)   // 4 KiB sector erase
				std::fill_n(prg + (addr & ~0xfff), 0x1000, 0xff);
			else if (data == 0x10 && cmd == 0x5555)   // chip erase
				std::fill_n(prg, size, 0xff);
			m_flash_step = 0;
			break;
		default: m_flash_step = 0; break;
	}
}

bool nes_unrom512_device::nvram_read(util::read_stream &file)
{
	const u32 size = m_prg_chunks * 0x4000;
	auto const [err, actual] = util::read(file, get_prg_base(), size);
	return !err && (actual == size);
}

bool nes_unrom512_device::nvram_write(util::write_stream &file)
{
	const u32 size = m_prg_chunks * 0x4000;
	auto const [err, actual] = util::write(file, get_prg_base(), size);
	return !err;
}

void nes_8bitxmas_device::device_start()
{
	nes_unrom512_device::device_start();
	save_item(NAME(m_led));

	m_bus_conflict = false;
}

void nes_8bitxmas_device::pcb_reset()
{
	nes_unrom512_device::pcb_reset();

	m_led = 0;
	update_led();
}



/*-------------------------------------------------
 mapper specific handlers
 -------------------------------------------------*/

/*-------------------------------------------------

 Sealie RET-CUFROM board

 Games: Glider (only?)

 This homebrew mapper supports 8x16k PRG banks at 0x8000,
 8k WRAM at 0x6000, and 4x8k VRAM banks. PRG is stored on
 flash ROM, though unlike mapper 30 it doesn't appear to
 be self-flashable, only through an external tool.

 iNES: mapper 29

 In MAME: Supported.

 -------------------------------------------------*/

void nes_cufrom_device::write_h(offs_t offset, u8 data)
{
	LOG("cufrom write_h, offset: %04x, data: %02x\n", offset, data);

	prg16_89ab(BIT(data, 2, 3));
	chr8(data & 0x03, CHRRAM);
}

/*-------------------------------------------------

 Sealie DPCMcart board

 Games: A Winner is You

 This homebrew mapper supports a whopping 64MB which
 is paged in 16K chucks at 0x8000. 0xc000 is fixed.

 NES 2.0: mapper 409

 In MAME: Partially supported.

 TODO: Controls other than 'next track' don't work.

 -------------------------------------------------*/

void nes_dpcmcart_device::write_h(offs_t offset, u8 data)
{
	LOG("dpcmcart write_h, offset: %04x, data: %02x\n", offset, data);
	prg16_89ab(offset & 0x0fff);
}

/*-------------------------------------------------

 Sealie UNROM 512 board

 Games: Battle Kid 1 & 2, E.T., many more

 This board has several variations and jumper configurations.
 Currently we only support the Sealie nonflashable config
 with 32x16k PRG banks at 0x8000, 4x8k VRAM, and three
 mirroring configs (H, V, or PCB selected 1-screen modes).

 iNES: mapper 30

 In MAME: Preliminary partial support.

 -------------------------------------------------*/

void nes_unrom512_device::write_h(offs_t offset, u8 data)
{
	LOG("unrom512 write_h, offset: %04x, data: %02x\n", offset, data);

	// NESdev UNROM 512: the register is at $8000-$FFFF only for submapper 0 without the battery bit and submapper 2;
	// otherwise it is at $C000-$FFFF, and $8000-$BFFF carries the flash command sequences (battery bit) or, on
	// submapper 4, the cartridge LED register (no LEDs to show here)
	if (offset < 0x4000)
	{
		if (m_flashable)
		{
			flash_w(offset, data);
			return;
		}
		if (m_submapper == 1 || m_submapper == 3 || m_submapper == 4)
			return;
	}

	data = account_bus_conflict(offset, data);
	m_latch = data;

	if (m_submapper == 3)   // mapper-controlled arrangement: 0 = horizontal mirroring, 1 = vertical mirroring
		set_nt_mirroring(BIT(data, 7) ? PPU_MIRROR_VERT : PPU_MIRROR_HORZ);
	else if (m_pcb_ctrl_mirror)
		set_nt_mirroring(BIT(data, 7) ? PPU_MIRROR_HIGH : PPU_MIRROR_LOW);
	prg16_89ab(data & 0x1f);
	chr8(BIT(data, 5, 2), CHRRAM);
}

/*-------------------------------------------------

 Sealie 8BIT XMAS revD board

 Games: 8-bit Xmas 2012-2016 and 2018-2021?

 This board is a variant of UNROM512 with 16 LEDs
 in 4 colors, blue, yellow, green, red, which are
 controlled in pairs by each byte written to
 0x8000-0xbfff. Bits are [BYGR bygr] where bygr
 control LEDs 1,2 and BYGR control LEDs 3,4. On
 the 8BIT XMAS revD 2012 board LEDS are arranged:

  _______________________________
 | RED2                     GRN3 |
 |                               |
 | YEL2                     BLU3 |
 |                               |
 | BLU1                     YEL4 |
 |                               |
 | GRN1                     RED4 |
 |                               |
 | RED1                     GRN4 |
 |                               |
 | YEL1                     BLU4 |
  --                           --
    |   GRN2 BLU2 YEL3 RED3   |
    |                         |

 iNES: mapper 30

 In MAME: Preliminary partial support.

 -------------------------------------------------*/

void nes_8bitxmas_device::update_led()
{
	// TODO: add artwork
}

void nes_8bitxmas_device::write_h(offs_t offset, u8 data)
{
	LOG("8bitxmas write_h, offset: %04x, data: %02x\n", offset, data);

	if (BIT(offset, 14))
		nes_unrom512_device::write_h(offset, data);
	else if (m_led != data)
	{
		m_led = data;
		update_led();
	}
}
