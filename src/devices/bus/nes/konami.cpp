// license:BSD-3-Clause
// copyright-holders:Fabio Priuli,Brian H. Gilbert
/***********************************************************************************************************


 NES/Famicom cartridge emulation for Konami PCBs

 iNES mappers: 21, 22, 23, 24, 25, 26, 73, 75, 85


 Here we emulate the following PCBs (multiple mappers needed due to different wirings in the pcbs
 causing different addresses to be used for bankswitch & irq)

 * Konami VRC-1 [mapper 75]
 * Konami VRC-2 [mapper 22,23,25]
 * Konami VRC-3 [mapper 73]
 * Konami VRC-4 [mapper 21,23,25]
 * Konami VRC-6 [mapper 24,26]
 * Konami VRC-7 [mapper 85]

 ***********************************************************************************************************/


#include "emu.h"
#include "konami.h"

#include "speaker.h"

#define LOG_UNHANDLED (1U << 1)

#ifdef NES_PCB_DEBUG
#define VERBOSE (LOG_UNHANDLED | LOG_GENERAL)
#else
#define VERBOSE (LOG_UNHANDLED)
#endif
#include "logmacro.h"


//-------------------------------------------------
//  constructor
//-------------------------------------------------

DEFINE_DEVICE_TYPE(NES_VRC1, nes_konami_vrc1_device, "nes_vrc1", "NES Cart Konami VRC-1 PCB")
DEFINE_DEVICE_TYPE(NES_VRC2, nes_konami_vrc2_device, "nes_vrc2", "NES Cart Konami VRC-2 PCB")
DEFINE_DEVICE_TYPE(NES_VRC3, nes_konami_vrc3_device, "nes_vrc3", "NES Cart Konami VRC-3 PCB")
DEFINE_DEVICE_TYPE(NES_VRC4, nes_konami_vrc4_device, "nes_vrc4", "NES Cart Konami VRC-4 PCB")
DEFINE_DEVICE_TYPE(NES_VRC6, nes_konami_vrc6_device, "nes_vrc6", "NES Cart Konami VRC-6 PCB")
DEFINE_DEVICE_TYPE(NES_VRC7, nes_konami_vrc7_device, "nes_vrc7", "NES Cart Konami VRC-7 PCB")


nes_konami_vrc1_device::nes_konami_vrc1_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: nes_nrom_device(mconfig, NES_VRC1, tag, owner, clock)
{
}

nes_konami_vrc2_device::nes_konami_vrc2_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock)
	: nes_nrom_device(mconfig, type, tag, owner, clock), m_latch(0)
{
}

nes_konami_vrc2_device::nes_konami_vrc2_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: nes_konami_vrc2_device(mconfig, NES_VRC2, tag, owner, clock)
{
}

nes_konami_vrc3_device::nes_konami_vrc3_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: nes_nrom_device(mconfig, NES_VRC3, tag, owner, clock), m_irq_count(0), m_irq_count_latch(0), m_irq_enable(0), m_irq_enable_latch(0), m_irq_mode(0)
{
}

nes_konami_vrc4_device::nes_konami_vrc4_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock)
	: nes_nrom_device(mconfig, type, tag, owner, clock), m_prg_flip(0), m_wram_enable(0), m_irq_count(0), m_irq_count_latch(0), m_irq_enable(0), m_irq_enable_latch(0), m_irq_mode(0), m_irq_prescale(0)
{
}

nes_konami_vrc4_device::nes_konami_vrc4_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: nes_konami_vrc4_device(mconfig, NES_VRC4, tag, owner, clock)
{
}

nes_konami_vrc6_device::nes_konami_vrc6_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: nes_konami_vrc4_device(mconfig, NES_VRC6, tag, owner, clock), m_vrc6snd(*this, "vrc6snd")
{
}

nes_konami_vrc7_device::nes_konami_vrc7_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: nes_konami_vrc4_device(mconfig, NES_VRC7, tag, owner, clock), m_vrc7snd(*this, "vrc7snd")
{
}



void nes_konami_vrc1_device::device_start()
{
	common_start();
	save_item(NAME(m_mmc_vrom_bank));
}

void nes_konami_vrc1_device::pcb_reset()
{
	prg16_89ab(0);
	prg16_cdef(m_prg_chunks - 1);
	chr8(0, m_chr_source);

	m_mmc_vrom_bank[0] = m_mmc_vrom_bank[1] = 0;
}

void nes_konami_vrc2_device::device_start()
{
	common_start();
	save_item(NAME(m_mmc_vrom_bank));
	save_item(NAME(m_latch));
}

void nes_konami_vrc2_device::pcb_reset()
{
	prg16_89ab(0);
	prg16_cdef(m_prg_chunks - 1);
	chr8(0, m_chr_source);

	m_latch = 0;
	std::fill(std::begin(m_mmc_vrom_bank), std::end(m_mmc_vrom_bank), 0x00);
}

void nes_konami_vrc3_device::device_start()
{
	common_start();

	save_item(NAME(m_irq_mode));
	save_item(NAME(m_irq_enable));
	save_item(NAME(m_irq_enable_latch));
	save_item(NAME(m_irq_count));
	save_item(NAME(m_irq_count_latch));
}

void nes_konami_vrc3_device::pcb_reset()
{
	prg16_89ab(0);
	prg16_cdef(m_prg_chunks - 1);
	chr8(0, m_chr_source);

	m_irq_mode = 0;
	m_irq_enable = 0;
	m_irq_enable_latch = 0;
	m_irq_count = 0;
	m_irq_count_latch = 0;
}

void nes_konami_vrc4_device::device_start()
{
	common_start();

	save_item(NAME(m_irq_mode));
	save_item(NAME(m_irq_prescale));
	save_item(NAME(m_irq_enable));
	save_item(NAME(m_irq_enable_latch));
	save_item(NAME(m_irq_count));
	save_item(NAME(m_irq_count_latch));
	save_item(NAME(m_prg_flip));
	save_item(NAME(m_wram_enable));
	save_item(NAME(m_mmc_prg_bank));
	save_item(NAME(m_mmc_vrom_bank));
}

void nes_konami_vrc4_device::pcb_reset()
{
	m_irq_mode = 0;
	m_irq_prescale = 0;
	m_irq_enable = 0;
	m_irq_enable_latch = 0;
	m_irq_count = 0;
	m_irq_count_latch = 0;

	m_prg_flip = 0;
	m_wram_enable = 0;
	m_mmc_prg_bank[0] = 0;
	m_mmc_prg_bank[1] = 0;
	set_prg();

	for (int i = 0; i < 8; i++)
		m_mmc_vrom_bank[i] = i;
	set_chr();
}

void nes_konami_vrc7_device::pcb_reset()
{
	nes_konami_vrc4_device::pcb_reset();
	prg32((m_prg_chunks >> 1) - 1);
}



/*-------------------------------------------------
 mapper specific handlers
 -------------------------------------------------*/

/*-------------------------------------------------

 Konami VRC1 and Jaleco JF20, JF22

 Games: Exciting Boxing, Ganbare Goemon!, Tetsuwan Atom

 iNES: mapper 75

 In MAME: Supported.

 -------------------------------------------------*/

void nes_konami_vrc1_device::write_h(offs_t offset, u8 data)
{
	LOG("VRC-1 write_h, offset: %04x, data: %02x\n", offset, data);

	switch (offset & 0x7000)
	{
		case 0x0000:
		case 0x2000:
		case 0x4000:
			prg8_x(BIT(offset, 13, 2), data & 0x0f);
			break;
		case 0x1000:
			// the mirroring bit is ignored on boards wired for four-screen VRAM (Vs. System VRC1 games; NESdev/Mesen)
			if (get_mirroring() != PPU_MIRROR_4SCREEN)
				set_nt_mirroring(data & 1 ? PPU_MIRROR_HORZ : PPU_MIRROR_VERT);
			m_mmc_vrom_bank[0] = (m_mmc_vrom_bank[0] & 0x0f) | BIT(data, 1) << 4;
			m_mmc_vrom_bank[1] = (m_mmc_vrom_bank[1] & 0x0f) | BIT(data, 2) << 4;
			chr4_0(m_mmc_vrom_bank[0], m_chr_source);
			chr4_4(m_mmc_vrom_bank[1], m_chr_source);
			break;
		case 0x6000:
			m_mmc_vrom_bank[0] = (m_mmc_vrom_bank[0] & 0x10) | (data & 0x0f);
			chr4_0(m_mmc_vrom_bank[0], m_chr_source);
			break;
		case 0x7000:
			m_mmc_vrom_bank[1] = (m_mmc_vrom_bank[1] & 0x10) | (data & 0x0f);
			chr4_4(m_mmc_vrom_bank[1], m_chr_source);
			break;
	}
}

/*-------------------------------------------------

 Konami VRC-2

 In MAME: Supported.

 -------------------------------------------------*/

u8 nes_konami_vrc2_device::read_m(offs_t offset)
{
	LOG("VRC-2 read_m, offset: %04x\n", offset);

	if (!m_battery.empty() || !m_prgram.empty())
		return device_nes_cart_interface::read_m(offset);
	else    // VRC2 was planned with EEPROM support; the non-working feature behaves as a vestigial 1-bit latch in $6000-$6fff on certain boards (contraj, ggoemon2 depend on this)
		return (offset < 0x1000) ? (get_open_bus() & 0xfe) | (m_latch & 1) : get_open_bus();
}

void nes_konami_vrc2_device::write_m(offs_t offset, u8 data)
{
	LOG("VRC-2 write_m, offset: %04x, data: %02x\n", offset, data);

	if (!m_battery.empty() || !m_prgram.empty())
		device_nes_cart_interface::write_m(offset, data);
	else if (offset < 0x1000)
		m_latch = data;
}

void nes_konami_vrc2_device::write_h(offs_t offset, u8 data)
{
	LOG("VRC-2 write_h, offset: %04x, data: %02x\n", offset, data);

	u8 addr_lines = bitswap<2>(offset, m_vrc_ls_prg_a, m_vrc_ls_prg_b);

	switch (offset & 0x7000)
	{
		case 0x0000:
		case 0x2000:
			prg8_x(BIT(offset, 13), data);
			break;
		case 0x1000:
			set_nt_mirroring(data & 1 ? PPU_MIRROR_HORZ : PPU_MIRROR_VERT);
			break;
		case 0x3000:
		case 0x4000:
		case 0x5000:
		case 0x6000:
		{
			int bank = 2 * (BIT(offset, 12, 3) - 3) + BIT(addr_lines, 1);
			int shift = BIT(addr_lines, 0) * 4;
			int mask = 0x0f << shift;
			m_mmc_vrom_bank[bank] = (m_mmc_vrom_bank[bank] & ~mask) | (((data >> m_vrc_ls_chr) << shift) & mask);
			chr1_x(bank, m_mmc_vrom_bank[bank], m_chr_source);
			break;
		}
		default:
			LOGMASKED(LOG_UNHANDLED, "VRC-2 write_h uncaught write, addr: %04x value: %02x\n", offset + 0x8000, data);
			break;
	}
}

/*-------------------------------------------------

 Konami VRC3

 Games: Salamander

 iNES: mapper 73

 In MAME: Supported.

 -------------------------------------------------*/

void nes_konami_vrc3_device::irq_clock()
{
	if (m_irq_enable)
	{
		u16 mask = m_irq_mode ? 0x00ff : 0xffff; // 8 or 16 bit mode?

		// upper byte only incremented and reloaded in 16-bit mode
		m_irq_count = (m_irq_count & ~mask) | ((m_irq_count + 1) & mask);
		if (!(m_irq_count & mask))
		{
			set_irq_line(ASSERT_LINE);
			m_irq_count = (m_irq_count & ~mask) | (m_irq_count_latch & mask);
		}
	}
}

void nes_konami_vrc3_device::write_h(offs_t offset, u8 data)
{
	LOG("VRC-3 write_h, offset: %04x, data: %02x\n", offset, data);

	switch (offset & 0x7000)
	{
		case 0x0000:
		case 0x1000:
		case 0x2000:
		case 0x3000:
		{
			int shift = 4 * BIT(offset, 12, 2);
			m_irq_count_latch &= ~(0x000f << shift);
			m_irq_count_latch |= (data & 0x0f) << shift;
			break;
		}
		case 0x4000:
			m_irq_mode = data & 0x04;
			m_irq_enable = data & 0x02;
			m_irq_enable_latch = data & 0x01;
			if (m_irq_enable)
				m_irq_count = m_irq_count_latch;
			set_irq_line(CLEAR_LINE);
			break;
		case 0x5000:
			m_irq_enable = m_irq_enable_latch;
			set_irq_line(CLEAR_LINE);
			break;
		case 0x7000:
			prg16_89ab(data & 0x07);
			break;
		default:
			LOGMASKED(LOG_UNHANDLED, "VRC-3 write_h uncaught write, offset %04x, data: %02x\n", offset, data);
			break;
	}
}

/*-------------------------------------------------

 Konami VRC-4

 In MAME: Supported

 -------------------------------------------------*/

void nes_konami_vrc4_device::irq_tick()
{
	if (m_irq_count == 0xff)
	{
		m_irq_count = m_irq_count_latch;
		set_irq_line(ASSERT_LINE);
	}
	else
		m_irq_count++;
}

void nes_konami_vrc4_device::irq_clock()
{
	if (m_irq_enable)
	{
		if (m_irq_mode) // cycle mode
			irq_tick();
		else    // scanline mode
		{
			// A prescaler divides the passing CPU cycles by 114, 114, then 113 (and repeats that order).
			// This approximates 113+2/3 CPU cycles, which is one NTSC scanline.
			// Since this is a CPU-based IRQ, though, it is triggered also during non visible scanlines...
			m_irq_prescale -= 3;

			if (m_irq_prescale <= 0)
			{
				m_irq_prescale += 341;
				irq_tick();
			}
		}
	}
}

void nes_konami_vrc4_device::irq_ack_w()
{
	m_irq_enable = m_irq_enable_latch;
	set_irq_line(CLEAR_LINE);
}

void nes_konami_vrc4_device::irq_ctrl_w(u8 data)
{
	m_irq_mode = data & 0x04;
	m_irq_enable = data & 0x02;
	m_irq_enable_latch = data & 0x01;
	// NESdev VRC IRQ: any write acknowledges the IRQ and resets the prescaler; with E set the counter is also reloaded
	m_irq_prescale = 341;
	if (m_irq_enable)
		m_irq_count = m_irq_count_latch;
	set_irq_line(CLEAR_LINE);
}

void nes_konami_vrc4_device::set_mirror(u8 data)
{
	switch (data & 0x03)
	{
		case 0x00: set_nt_mirroring(PPU_MIRROR_VERT); break;
		case 0x01: set_nt_mirroring(PPU_MIRROR_HORZ); break;
		case 0x02: set_nt_mirroring(PPU_MIRROR_LOW); break;
		case 0x03: set_nt_mirroring(PPU_MIRROR_HIGH); break;
	}
}

void nes_konami_vrc4_device::set_prg(int prg_base, int prg_mask)
{
	prg8_x(0 ^ m_prg_flip, prg_base | (m_mmc_prg_bank[0] & prg_mask));
	prg8_x(1, prg_base | (m_mmc_prg_bank[1] & prg_mask));
	prg8_x(2 ^ m_prg_flip, prg_base | (prg_mask & ~1));
	prg8_x(3, prg_base | prg_mask);
}

void nes_konami_vrc4_device::set_chr(int chr_base, int chr_mask)
{
	for (int i = 0; i < 8; i++)
		chr1_x(i, chr_base | (m_mmc_vrom_bank[i] & chr_mask), m_chr_source);
}

u8 nes_konami_vrc4_device::read_m(offs_t offset)
{
	LOG("VRC-4 read_m, offset: %04x\n", offset);

	if (m_wram_enable)
		return device_nes_cart_interface::read_m(offset);
	else
		return get_open_bus();
}

void nes_konami_vrc4_device::write_m(offs_t offset, u8 data)
{
	LOG("VRC-4 write_m, offset: %04x, data: %02x\n", offset, data);

	if (m_wram_enable)
		device_nes_cart_interface::write_m(offset, data);
}

void nes_konami_vrc4_device::write_h(offs_t offset, u8 data)
{
	LOG("VRC-4 write_h, offset: %04x, data: %02x\n", offset, data);

	u8 addr_lines = bitswap<2>(offset, m_vrc_ls_prg_a, m_vrc_ls_prg_b);

	switch (offset & 0x7000)
	{
		case 0x0000:
		case 0x2000:
			m_mmc_prg_bank[BIT(offset, 13)] = data;
			set_prg();
			break;
		case 0x1000:
			if (BIT(addr_lines, 1))
			{
				m_wram_enable = data & 0x01;
				m_prg_flip = data & 0x02;
				set_prg();
			}
			else
				set_mirror(data);
			break;
		case 0x3000:
		case 0x4000:
		case 0x5000:
		case 0x6000:
		{
			int bank = 2 * (BIT(offset, 12, 3) - 3) + BIT(addr_lines, 1);
			int shift = BIT(addr_lines, 0) * 4;
			int mask = shift ? 0x1f0 : 0x0f;
			m_mmc_vrom_bank[bank] = (m_mmc_vrom_bank[bank] & ~mask) | ((data << shift) & mask);
			set_chr();
			break;
		}
		case 0x7000:
			switch (addr_lines)
			{
				case 0:
					m_irq_count_latch = (m_irq_count_latch & 0xf0) | (data & 0x0f);
					break;
				case 1:
					m_irq_count_latch = (m_irq_count_latch & 0x0f) | (data & 0x0f) << 4;
					break;
				case 2:
					irq_ctrl_w(data);
					break;
				case 3:
					irq_ack_w();
					break;
			}
			break;
	}
}

/*-------------------------------------------------

 Konami VRC-6

 In MAME: Supported. It also uses konami_irq (there are IRQ
 issues though: see Akumajou Densetsu intro).

 TODO: Implement other CHR modes not used by any of
 the three released VRC6 games?

 -------------------------------------------------*/

void nes_konami_vrc6_device::device_start()
{
	nes_konami_vrc4_device::device_start();
	save_item(NAME(m_vrc6_chr));
	save_item(NAME(m_vrc6_mode));
}

void nes_konami_vrc6_device::pcb_reset()
{
	nes_konami_vrc4_device::pcb_reset();
	std::fill(std::begin(m_vrc6_chr), std::end(m_vrc6_chr), 0);
	m_vrc6_mode = 0;
	vrc6_update_ppu();
}

// $B003 PPU banking style (NESdev VRC6, behaviour as implemented by Mesen): bits 0-1 CHR layout, bit 5 pairs 1KiB
// banks (A10 from the bank number), bit 4 nametables from CHR-ROM instead of CIRAM, bits 2-3/5 nametable arrangement
void nes_konami_vrc6_device::vrc6_update_ppu()
{
	const u8 *r = m_vrc6_chr;
	const u8 mask = BIT(m_vrc6_mode, 5) ? 0xfe : 0xff;
	const u8 orv = BIT(m_vrc6_mode, 5) ? 0x01 : 0x00;

	switch (m_vrc6_mode & 0x03)
	{
		case 0:
			for (int i = 0; i < 8; i++)
				chr1_x(i, r[i], m_chr_source);
			break;
		case 1:
			for (int i = 0; i < 4; i++)
			{
				chr1_x(2 * i, r[i] & mask, m_chr_source);
				chr1_x(2 * i + 1, (r[i] & mask) | orv, m_chr_source);
			}
			break;
		default:
			for (int i = 0; i < 4; i++)
				chr1_x(i, r[i], m_chr_source);
			chr1_x(4, r[4] & mask, m_chr_source);
			chr1_x(5, (r[4] & mask) | orv, m_chr_source);
			chr1_x(6, r[5] & mask, m_chr_source);
			chr1_x(7, (r[5] & mask) | orv, m_chr_source);
			break;
	}

	u8 nt[4];
	switch (m_vrc6_mode & 0x2f)
	{
		case 0x20: case 0x27: nt[0] = r[6] & 0xfe; nt[1] = (r[6] & 0xfe) | 1; nt[2] = r[7] & 0xfe; nt[3] = (r[7] & 0xfe) | 1; break;
		case 0x23: case 0x24: nt[0] = r[6] & 0xfe; nt[1] = r[7] & 0xfe; nt[2] = (r[6] & 0xfe) | 1; nt[3] = (r[7] & 0xfe) | 1; break;
		case 0x28: case 0x2f: nt[0] = nt[1] = r[6] & 0xfe; nt[2] = nt[3] = r[7] & 0xfe; break;
		case 0x2b: case 0x2c: nt[0] = nt[2] = (r[6] & 0xfe) | 1; nt[1] = nt[3] = (r[7] & 0xfe) | 1; break;
		default:
			switch (m_vrc6_mode & 0x07)
			{
				case 1: case 5: nt[0] = r[4]; nt[1] = r[5]; nt[2] = r[6]; nt[3] = r[7]; break;
				case 2: case 3: case 4: nt[0] = nt[2] = r[6]; nt[1] = nt[3] = r[7]; break;
				default: nt[0] = nt[1] = r[6]; nt[2] = nt[3] = r[7]; break;   // 0, 6, 7
			}
			break;
	}
	for (int page = 0; page < 4; page++)
	{
		if (BIT(m_vrc6_mode, 4))
		{
			// nametables from CHR memory: CHR-ROM, or the board's CHR-RAM when it has no CHR-ROM
			if (m_vrom_chunks)
				set_nt_page(page, VROM, nt[page], 0);
			else
				set_nt_page(page, VRAM_NT, nt[page] & ((get_vram_size() / 0x400) - 1), 1);
		}
		else
			set_nt_page(page, CIRAM, nt[page] & 0x01, 1);
	}
}

void nes_konami_vrc6_device::write_h(offs_t offset, u8 data)
{
	LOG("VRC-6 write_h, offset: %04x, data: %02x\n", offset, data);

	u8 addr_lines = bitswap<2>(offset, m_vrc_ls_prg_a, m_vrc_ls_prg_b);

	switch (offset & 0x7000)
	{
		case 0x0000:
			prg16_89ab(data & 0x0f);
			break;
		case 0x4000:
			prg8_cd(data & 0x1f);
			break;
		case 0x1000:    // pulse 1 & global control
			m_vrc6snd->write(addr_lines, data);
			break;
		case 0x2000:    // pulse 2
			m_vrc6snd->write(addr_lines | 0x100, data);
			break;
		case 0x3000:
			if (addr_lines == 3)
			{
				m_vrc6_mode = data;
				m_wram_enable = BIT(data, 7);
				vrc6_update_ppu();
			}
			else    // saw
				m_vrc6snd->write(addr_lines | 0x200, data);
			break;
		case 0x5000:
		case 0x6000:
			m_vrc6_chr[4 * BIT(offset, 13) + addr_lines] = data;
			vrc6_update_ppu();
			break;
		case 0x7000:
			switch (addr_lines)
			{
				case 0:
					m_irq_count_latch = data;
					break;
				case 1:
					irq_ctrl_w(data);
					break;
				case 2:
					irq_ack_w();
					break;
				default:
					LOGMASKED(LOG_UNHANDLED, "VRC-6 write_h uncaught write, addr: %04x value: %02x\n", ((offset & 0x7000) | addr_lines) + 0x8000, data);
					break;
			}
			break;
	}
}


//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void nes_konami_vrc6_device::device_add_mconfig(machine_config &config)
{
	// additional sound hardware
	SPEAKER(config, "addon").front_center();

	// TODO: this is not how VRC6 clock signaling works!
	// The board uses the CLK pin in reality, not hardcoded NTSC values!
	// level measured with nes-audio-tests db_vrc6 against the hardware survey (+0.2 dB vs a full-volume APU square)
	VRC6(config, m_vrc6snd, XTAL(21'477'272)/12).add_route(ALL_OUTPUTS, "addon", 1.175);
}

/*-------------------------------------------------

 Konami VRC7

 Games: Lagrange Point, Tiny Toon Adventures 2

 iNES: mapper 85

 In MAME: Supported. It also uses konami_irq.

 -------------------------------------------------*/

void nes_konami_vrc7_device::write_h(offs_t offset, u8 data)
{
	LOG("VRC-7 write_h, offset: %04x, data: %02x\n", offset, data);

	// one decode for both wirings (as Mesen): the audio registers are $9010 (select) and $9030 (data) on VRC7a and
	// VRC7b alike; any other address with A4 set acts as A3, so VRC7a (A4) and VRC7b (A3) games both work without a
	// submapper (unspecified mapper 85 images were decoded as VRC7b only, which put VRC7a audio writes on PRG banks)
	u16 addr = offset | 0x8000;
	if ((addr & 0xf038) == 0x9010)
	{
		m_vrc7snd->address_w(data);
		return;
	}
	if ((addr & 0xf038) == 0x9030)
	{
		m_vrc7snd->data_w(data);
		return;
	}
	if (BIT(addr, 4))
		addr = (addr | 0x08) & ~0x10;
	u8 reg = bitswap<4>(addr, 14, 13, 12, 3);

	switch (reg)
	{
		case 0x0:
		case 0x1:
		case 0x2:
			prg8_x(reg, data & 0x3f);
			break;
		case 0x3:   // $9008/$9028: no register
			break;
		case 0x4:
		case 0x5:
		case 0x6:
		case 0x7:
		case 0x8:
		case 0x9:
		case 0xa:
		case 0xb:
			chr1_x(reg - 4, data, m_chr_source);
			break;
		case 0xc:
			set_mirror(data);
			m_wram_enable = BIT(data, 7);
			break;
		case 0xd:
			m_irq_count_latch = data;
			break;
		case 0xe:
			irq_ctrl_w(data);
			break;
		case 0xf:
			irq_ack_w();
			break;
	}
}


//-------------------------------------------------
//  MACHINE_DRIVER( vrc7 )
//-------------------------------------------------

// From NESdev wiki: The VRC7, in addition to being a mapper chip, also produces 6 channels of
// 2-operator FM Synthesis Audio. It is a derivative of the Yamaha YM2413 OPLL, implementing a
// subset of its features and containing a custom fixed patch set.
// The synthesis core is related to the Yamaha YM2413 OPLL:
// - Register layout is the same.
// - VRC7 has 6 channels, OPLL has 9.
// - VRC7 has no rhythm channels, OPLL does (the last 3 channels are either FM or Rhythm on OPLL).
// - VRC7 built-in instruments are not the same as OPLL instruments.
// - VRC7 has no readily-accessible status register, under normal circumstances it is write-only;
//   OPLL has an undocumented, 2-bit 'internal state' register.
// - VRC7 has an internal state output pin (may be serial version of the 2 bit internal state register)
//   and has one output pin for audio, multiplexed for all 6 channels; OPLL has two output pins, one for
//   FM and one for Rhythm, and has no special status pin.

void nes_konami_vrc7_device::device_add_mconfig(machine_config &config)
{
	// additional sound hardware
	SPEAKER(config, "addon").front_center();

	// TODO: this is not how VRC7 clock signaling works!
	// The board uses the CLK pin in reality, not hardcoded NTSC values!
	// level measured with nes-audio-tests db_vrc7 against the hardware survey (+11.3 dB vs a full-volume APU square)
	DS1001(config, m_vrc7snd, XTAL(21'477'272)/6).add_route(0, "addon", 2.43).add_route(1, "addon", 0.0);
}
