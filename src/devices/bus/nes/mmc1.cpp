// license:BSD-3-Clause
// copyright-holders:Fabio Priuli,Brian H. Gilbert
/***********************************************************************************************************


 NES/Famicom cartridge emulation for Nintendo MMC-1 PCBs

 iNES mappers: 1, 155


 Here we emulate the Nintendo SxROM / MMC-1 PCBs + older variants without WRAM protect bit
 iNES mapper 155 is the same as mapper 1 but forces the use of the MMC1A

 Known issues on specific mappers:

 * 001 Yoshi flashes in-game.

 TODO:
 - Combine 2 versions of set_prg in SxROM base class. This means dealing with
   variant boards SNROM, SUROM, etc which repurpose bits in the MMC1 regs.
 - Determine if "MMC1" marked chips, the earliest version, ignores WRAM
   enable/disable bit like its first revision, MMC1A. Also determine if MMC1C
   really exists. It's described by kevtris, but it's not in BootGod's DB.

 ***********************************************************************************************************/


#include "emu.h"
#include "mmc1.h"

#ifdef NES_PCB_DEBUG
#define VERBOSE (LOG_GENERAL)
#else
#define VERBOSE (0)
#endif
#include "logmacro.h"


//-------------------------------------------------
//  constructor
//-------------------------------------------------

DEFINE_DEVICE_TYPE(NES_SXROM, nes_sxrom_device, "nes_sxrom", "NES Cart SxROM (MMC-1) PCB")
DEFINE_DEVICE_TYPE(NES_SOROM, nes_sorom_device, "nes_sorom", "NES Cart SOROM (MMC-1) PCB")
DEFINE_DEVICE_TYPE(NES_SZROM, nes_szrom_device, "nes_szrom", "NES Cart SZROM (MMC-1) PCB")



nes_sxrom_device::nes_sxrom_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock)
	: nes_nrom_device(mconfig, type, tag, owner, clock), m_last_write_cycle(~u64(0)), m_latch(0), m_count(0)
{
}

nes_sxrom_device::nes_sxrom_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: nes_sxrom_device(mconfig, NES_SXROM, tag, owner, clock)
{
}

nes_sorom_device::nes_sorom_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: nes_sxrom_device(mconfig, NES_SOROM, tag, owner, clock)
{
}

nes_szrom_device::nes_szrom_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: nes_sxrom_device(mconfig, NES_SZROM, tag, owner, clock)
{
}



void nes_sxrom_device::device_start()
{
	common_start();
	save_item(NAME(m_latch));
	save_item(NAME(m_count));
	save_item(NAME(m_reg));
	save_item(NAME(m_last_write_cycle));
	save_item(NAME(m_ppu_a12));
}

void nes_sxrom_device::pcb_reset()
{
	m_latch = 0;
	m_count = 0;
	m_reg[0] = 0x0f;
	m_reg[1] = 0;
	m_reg[2] = 0;
	m_reg[3] = 0;
	m_last_write_cycle = ~u64(0);

	set_chr();
	set_prg();
	set_mirror();
}




/*-------------------------------------------------
 mapper specific handlers
 -------------------------------------------------*/


/*-------------------------------------------------

 SxROM (MMC1 based) board emulation

 iNES: mapper 1 (and 155 for the MMC1A variant which does not
 have WRAM disable bit)

 -------------------------------------------------*/


// Standard MMC1 PRG banking with base and mask (to support multicarts, etc)
void nes_sxrom_device::set_prg(int prg_base, int prg_mask)
{
	u8 bank = prg_base | (m_reg[3] & prg_mask);

	switch (BIT(m_reg[0], 2, 2))
	{
		case 0:
		case 1:
			prg32(bank >> 1);
			break;
		case 2:
			prg16_89ab(prg_base);
			prg16_cdef(bank);
			break;
		case 3:
			prg16_89ab(bank);
			prg16_cdef(prg_base | prg_mask);
			break;
	}
}

void nes_sxrom_device::set_prg()
{
	// NES 2.0 mapper 1 submapper 5 (SEROM/SHROM/SH1ROM): PRG-ROM A14 is wired to CPU A14, so PRG is a fixed 32K
	if (m_mmc1_fixed_prg)
	{
		prg32(0);
		return;
	}

	const u8 prg_mode = m_reg[0] & 0x0c;
	// SUROM/SXROM: CHR bank bit 4 is PRG-ROM A18 (256K outer bank); on smaller boards it falls outside the ROM.
	// In 4K CHR mode it comes from the CHR register PPU A12 selects (see ppu_a12_level)
	const u8 prg_offset = chr_hi_reg() & 0x10;

	switch (prg_mode)
	{
		case 0x00:
		case 0x04:
			prg32((prg_offset + m_reg[3]) >> 1);
			break;
		case 0x08:
			// NESdev MMC1: on the MMC1A, PRG bit 3 bypasses the fixed-bank logic in 16K modes: the fixed bank then only
			// drives A16-A14 while bit 3 drives A17 directly, so in mode 2 the bank fixed at $8000 becomes bank 8
			prg16_89ab(prg_offset + ((m_mmc1_type == mmc1_type::MMC1A) ? (m_reg[3] & 0x08) : 0));
			prg16_cdef(prg_offset + m_reg[3]);
			break;
		case 0x0c:
			prg16_89ab(prg_offset + m_reg[3]);
			prg16_cdef(prg_offset + 0x0f);
			break;
	}
}

// Standard MMC1 CHR banking with base and mask (to support multicarts, etc)
void nes_sxrom_device::set_chr(int chr_base, int chr_mask)
{
	if (BIT(m_reg[0], 4))
	{
		chr4_0(chr_base | (m_reg[1] & chr_mask), m_chr_source);
		chr4_4(chr_base | (m_reg[2] & chr_mask), m_chr_source);
	}
	else
		chr8((chr_base | (m_reg[1] & chr_mask)) >> 1, m_chr_source);
}

void nes_sxrom_device::set_mirror()
{
	static constexpr u8 mirr[4] = { PPU_MIRROR_LOW, PPU_MIRROR_HIGH, PPU_MIRROR_VERT, PPU_MIRROR_HORZ };

	set_nt_mirroring(mirr[m_reg[0] & 0x03]);
}

// this allows for easier implementation of MMC1 subclasses
void nes_sxrom_device::update_regs(int reg)
{
	switch (reg)
	{
		case 0:
			set_chr();
			set_prg();
			set_mirror();
			break;
		case 1:
			set_chr();
			set_prg();
			break;
		case 2:
			set_chr();
			set_prg();   // in 4K CHR mode $C000 drives PRG A18 while PPU A12 is high
			break;
		case 3:
			set_prg();
			break;
	}
}

void nes_sxrom_device::write_h(offs_t offset, u8 data)
{
	LOG("sxrom write_h, offset: %04x, data: %02x\n", offset, data);

	// There is only one latch and shift counter, shared amongst the 4 regs (testcase: Space Shuttle)

	/* here we would need to add an if(cpu_cycles_passed>1) test, and
	 if requirement is not met simply return without writing anything.
	 Some games (AD&D Hillsfar, Bill & Ted Excellent Adventure, Cosmic
	 Wars, Rocket Ranger, Sesame Street 123 and Snow Brothers) rely on
	 this behavior!! */
	// NESdev MMC1: when the CPU writes on consecutive cycles (read-modify-write instructions), every write but the first
	// is ignored (AD&D Hillsfar, Bill & Ted's Excellent Adventure, Cosmic Wars, Rocket Ranger, Snow Brothers rely on it)
	const u64 cycle = cpu_total_cycles();
	const bool consecutive = (cycle == m_last_write_cycle + 1);
	m_last_write_cycle = cycle;
	if (consecutive)
		return;

	if (data & 0x80)
	{
		m_count = 0;

		// Set reg at 0x8000 to size 16k and lower half swap - needed for Robocop 3, Dynowars
		m_reg[0] |= 0x0c;
		set_prg();
		return;
	}

	m_latch >>= 1;
	m_latch |= (data & 1) << 4;
	m_count = (m_count + 1) % 5;

	if (!m_count)
	{
		int reg = BIT(offset, 13, 2);
		m_reg[reg] = m_latch;
		update_regs(reg);
	}
}

// PRG-RAM decoding on the SxROM family (NESdev MMC1 / NES 2.0 submappers):
//  - MMC1B and later: PRG bank bit 4 disables PRG-RAM (ignored by the MMC1A)
//  - SNROM (8K CHR, PRG <= 256K, 8K PRG-RAM): CHR bank bit 4 (CHR A16) is an extra PRG-RAM disable
//  - SOROM (16K PRG-RAM): CHR bank bit 3 is PRG-RAM A13; the first 8K is work RAM, the second battery-backed
//  - SXROM (32K PRG-RAM): CHR bank bits 2-3 are PRG-RAM A13-A14
// In 4K CHR mode these bits come from whichever CHR register PPU A12 selects (see ppu_a12_level).
u32 nes_sxrom_device::prgram_bank_offset(bool &enabled) const
{
	const u32 total = m_prgram.size() + m_battery.size();
	const u8 chr = chr_bits_drive_prg() ? chr_hi_reg() : 0;
	enabled = !BIT(m_reg[3], 4) || m_mmc1_type == mmc1_type::MMC1A;
	if (total == 0x2000 && m_vrom_chunks <= 1 && m_prg_chunks <= 16 && BIT(chr, 4))
		enabled = false;
	if (total == 0x4000)
		return BIT(chr, 3) * 0x2000;
	if (total >= 0x8000)
		return BIT(chr, 2, 2) * 0x2000;
	return 0;
}

// NESdev MMC1: in 4K CHR mode the MMC1's upper CHR outputs (used by SNROM/SOROM/SUROM/SXROM for PRG-RAM and PRG-ROM)
// come from $A000 while PPU A12 is low and from $C000 while it is high. The PPU reports A12 as of its last catch-up
// (every PPU register access and every scanline end), so CPU accesses during active rendering may see it up to a
// scanline late; NESdev notes games keep both registers' upper bits equal, so this only affects ones that do not.
void nes_sxrom_device::ppu_a12_level(int state)
{
	const bool a12 = state != 0;
	if (a12 == m_ppu_a12)
		return;
	m_ppu_a12 = a12;
	if (!chr_bits_drive_prg())
		return;
	if (BIT(m_reg[0], 4) && ((m_reg[1] ^ m_reg[2]) & 0x10) && !m_mmc1_fixed_prg)
		set_prg();
}

void nes_sxrom_device::write_m(offs_t offset, u8 data)
{
	LOG("sxrom write_m, offset: %04x, data: %02x\n", offset, data);
	bool enabled;
	const u32 addr = prgram_bank_offset(enabled) + offset;
	if (!enabled)
		return;
	if (!m_prgram.empty() && !m_battery.empty())
	{
		// work RAM first, then battery RAM (SOROM)
		if (addr < m_prgram.size())
			m_prgram[addr] = data;
		else
			m_battery[(addr - m_prgram.size()) & (m_battery.size() - 1)] = data;
		return;
	}
	if (!m_battery.empty())
		m_battery[addr & (m_battery.size() - 1)] = data;
	if (!m_prgram.empty())
		m_prgram[addr & (m_prgram.size() - 1)] = data;
}

u8 nes_sxrom_device::read_m(offs_t offset)
{
	LOG("sxrom read_m, offset: %04x\n", offset);
	bool enabled;
	const u32 addr = prgram_bank_offset(enabled) + offset;
	if (enabled)
	{
		if (!m_prgram.empty() && !m_battery.empty())
			return (addr < m_prgram.size()) ? m_prgram[addr] : m_battery[(addr - m_prgram.size()) & (m_battery.size() - 1)];
		if (!m_battery.empty())
			return m_battery[addr & (m_battery.size() - 1)];
		if (!m_prgram.empty())
			return m_prgram[addr & (m_prgram.size() - 1)];
	}
	return get_open_bus();
}

// SOROM has two RAM banks, the first is not battery backed up, the second is.
void nes_sorom_device::write_m(offs_t offset, u8 data)
{
	u8 type = BIT(chr_hi_reg(), 3);   // NESdev: SOROM implements only the upper S bit (CHR bank bit 3) as PRG-RAM A13
	LOG("sorom write_m, offset: %04x, data: %02x\n", offset, data);

	if (!BIT(m_reg[3], 4) || m_mmc1_type == mmc1_type::MMC1A)  // WRAM enabled
	{
		if (type)
			m_battery[offset & (m_battery.size() - 1)] = data;
		else
			m_prgram[offset & (m_prgram.size() - 1)] = data;
	}
}

u8 nes_sorom_device::read_m(offs_t offset)
{
	u8 type = BIT(chr_hi_reg(), 3);   // NESdev: SOROM implements only the upper S bit (CHR bank bit 3) as PRG-RAM A13
	LOG("sorom read_m, offset: %04x\n", offset);

	if (!BIT(m_reg[3], 4) || m_mmc1_type == mmc1_type::MMC1A)  // WRAM enabled
	{
		if (type)
			return m_battery[offset & (m_battery.size() - 1)];
		else
			return m_prgram[offset & (m_prgram.size() - 1)];
	}

	return get_open_bus();
}

// SZROM has two RAM banks, the first is not battery backed up, the second is.
void nes_szrom_device::write_m(offs_t offset, u8 data)
{
	LOG("szrom write_m, offset: %04x, data: %02x\n", offset, data);

	if (!BIT(m_reg[3], 4) || m_mmc1_type == mmc1_type::MMC1A)  // WRAM enabled
	{
		if (BIT(chr_hi_reg(), 4))
			m_battery[offset & (m_battery.size() - 1)] = data;
		else
			m_prgram[offset & (m_prgram.size() - 1)] = data;
	}
}

u8 nes_szrom_device::read_m(offs_t offset)
{
	LOG("szrom read_m, offset: %04x\n", offset);

	if (!BIT(m_reg[3], 4) || m_mmc1_type == mmc1_type::MMC1A)  // WRAM enabled
	{
		if (BIT(chr_hi_reg(), 4))
			return m_battery[offset & (m_battery.size() - 1)];
		else
			return m_prgram[offset & (m_prgram.size() - 1)];
	}

	return get_open_bus();
}
