// license:BSD-3-Clause
// copyright-holders:Fabio Priuli,Brian H. Gilbert
/***********************************************************************************************************

 NES/Famicom cartridge emulation for J.Y. Company

 iNES mappers: 90, 209, 211

 The J.Y. Company ASIC (NESdev "J.Y. Company ASIC"), used by the later single-game cartridges and most of the
 multicarts. Mapper 209 is the standard board, mapper 90 the one whose solder jumper inhibits ROM nametables and
 extended mirroring, mapper 211 a duplicate of 209 (NESdev: no PCB with the nametable features forced on exists).

 Registers (decoded with the masks given by NESdev; writes with address bit 11 set are ignored where noted):
   $5000/$5400/$5C00  read: solder pad register, bits 7-6 (fully decoded)
   $5800-$5803 (F803) multiplier (write operand 1, operand 2; read product low, high), accumulator ($5802: add,
					  $5803: clear and set the test register; both readable)
   $6000-$7FFF        WRAM or open bus, or the PRG-ROM bank of $8003 when $D000 bit 7 is set
   $8000-$8003 (F803) PRG banks, 7 bits
   $9000-$9007 (F807) CHR bank low bytes, $A000-$A007 high bytes
   $B000-$B003 (F807) nametable low bytes, $B004-$B007 high bytes
   $C000-$C007 (F007) IRQ; address bit 11 does not matter here
   $D000-$D003 (F803) mode, nametable arrangement, PPU memory configuration, outer bank

 IRQ: the prescaler is clocked from CPU M2, unfiltered PPU A12 rises, every PPU read or every CPU write cycle
 ($C001 bits 0-1), counting up or down ($C001 bits 6-7); a prescaler wrap clocks the counter and a counter wrap
 raises the IRQ. Disabling the IRQ ($C000 bit 0 clear, $C002) acknowledges it, inhibits counting and clears the
 prescaler.

 Not emulated, because nothing is documented about it (NESdev: no known game uses it):
 - the "funky" IRQ mode ($C001 bit 3, $C007)
 - the intermediate results of the multiplier during the 8 M2 cycles after the second operand is written
 - boards that do not connect CPU A11 (address masks $F000, multiplier unreachable)
 - the outer bank layouts of the NES 2.0 mappers 281, 282, 295, 358, 386, 387, 388 and 397

 The power-on state of the registers is not known: all of them are cleared here (32K PRG mode with the last
 bank fixed, 8K CHR mode, bank 0).

 ***********************************************************************************************************/


#include "emu.h"
#include "jy.h"

#ifdef NES_PCB_DEBUG
#define VERBOSE (LOG_GENERAL)
#else
#define VERBOSE (0)
#endif
#include "logmacro.h"


//-------------------------------------------------
//  constructor
//-------------------------------------------------

DEFINE_DEVICE_TYPE(NES_JY_TYPEA, nes_jy_typea_device, "nes_jya", "NES Cart JY Company Type A PCB")
DEFINE_DEVICE_TYPE(NES_JY_TYPEB, nes_jy_typeb_device, "nes_jyb", "NES Cart JY Company Type B PCB")
DEFINE_DEVICE_TYPE(NES_JY_TYPEC, nes_jy_typec_device, "nes_jyc", "NES Cart JY Company Type C PCB")


static INPUT_PORTS_START( jy_asic )
	PORT_START("JUMPER")
	PORT_CONFNAME( 0xc0, 0x00, "Solder Pads" )
	PORT_CONFSETTING(    0x00, "00" )
	PORT_CONFSETTING(    0x40, "01" )
	PORT_CONFSETTING(    0x80, "10" )
	PORT_CONFSETTING(    0xc0, "11" )
INPUT_PORTS_END

ioport_constructor nes_jy_typea_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( jy_asic );
}


nes_jy_typea_device::nes_jy_typea_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock, bool nt_jumper)
	: nes_nrom_device(mconfig, type, tag, owner, clock)
	, m_jumper(*this, "JUMPER")
	, m_nt_jumper(nt_jumper)
	, m_acc(0), m_test(0)
	, m_mode(0), m_mirror(0), m_ppucfg(0), m_outer(0)
	, m_bank_6000(0)
	, m_irq_enable(0), m_irq_source(0), m_irq_dir(0), m_irq_small(0), m_irq_funky(0)
	, m_irq_prescaler(0), m_irq_counter(0), m_irq_xor(0), m_irq_c007(0), m_a12(0)
{
}

nes_jy_typea_device::nes_jy_typea_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: nes_jy_typea_device(mconfig, NES_JY_TYPEA, tag, owner, clock, true)
{
}

nes_jy_typeb_device::nes_jy_typeb_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: nes_jy_typea_device(mconfig, type, tag, owner, clock, false)
{
}

nes_jy_typeb_device::nes_jy_typeb_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: nes_jy_typeb_device(mconfig, NES_JY_TYPEB, tag, owner, clock)
{
}

nes_jy_typec_device::nes_jy_typec_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: nes_jy_typeb_device(mconfig, NES_JY_TYPEC, tag, owner, clock)
{
}


void nes_jy_typea_device::device_start()
{
	common_start();

	save_item(NAME(m_mul));
	save_item(NAME(m_acc));
	save_item(NAME(m_test));
	save_item(NAME(m_prg_reg));
	save_item(NAME(m_chr_lo));
	save_item(NAME(m_chr_hi));
	save_item(NAME(m_nt_lo));
	save_item(NAME(m_nt_hi));
	save_item(NAME(m_mode));
	save_item(NAME(m_mirror));
	save_item(NAME(m_ppucfg));
	save_item(NAME(m_outer));
	save_item(NAME(m_chr_latch));
	save_item(NAME(m_bank_6000));
	save_item(NAME(m_irq_enable));
	save_item(NAME(m_irq_source));
	save_item(NAME(m_irq_dir));
	save_item(NAME(m_irq_small));
	save_item(NAME(m_irq_funky));
	save_item(NAME(m_irq_prescaler));
	save_item(NAME(m_irq_counter));
	save_item(NAME(m_irq_xor));
	save_item(NAME(m_irq_c007));
	save_item(NAME(m_a12));
}

void nes_jy_typea_device::pcb_reset()
{
	std::fill(std::begin(m_mul), std::end(m_mul), 0);
	m_acc = m_test = 0;
	std::fill(std::begin(m_prg_reg), std::end(m_prg_reg), 0);
	std::fill(std::begin(m_chr_lo), std::end(m_chr_lo), 0);
	std::fill(std::begin(m_chr_hi), std::end(m_chr_hi), 0);
	std::fill(std::begin(m_nt_lo), std::end(m_nt_lo), 0);
	std::fill(std::begin(m_nt_hi), std::end(m_nt_hi), 0);
	m_mode = m_mirror = m_ppucfg = m_outer = 0;
	m_chr_latch[0] = m_chr_latch[1] = 0;
	m_bank_6000 = 0;

	m_irq_enable = 0;
	m_irq_source = 0;
	m_irq_dir = 0;
	m_irq_small = 0;
	m_irq_funky = 0;
	m_irq_prescaler = 0;
	m_irq_counter = 0;
	m_irq_xor = 0;
	m_irq_c007 = 0;
	m_a12 = 0;

	update_prg();
	update_chr();
	update_nt();
}


//-------------------------------------------------
//  banking
//-------------------------------------------------

void nes_jy_typea_device::update_prg()
{
	const int mode = m_mode & 3;
	const bool sw_last = BIT(m_mode, 2);
	const int outer = BIT(m_outer, 1, 2) << 6;      // PRG A19-A20, in 8K banks

	// mode 3 is mode 2 with bits 0-6 of the bank numbers reversed
	auto reg = [this, mode] (int i)
	{
		const uint8_t r = m_prg_reg[i] & 0x7f;
		return (mode == 3) ? bitswap<7>(r, 0, 1, 2, 3, 4, 5, 6) : r;
	};

	switch (mode)
	{
	case 0: // 32K
		{
			const int bank = outer | ((sw_last ? (reg(3) & 0x0f) : 0x0f) << 2);
			for (int i = 0; i < 4; i++)
				prg8_x(i, bank + i);
			m_bank_6000 = outer | (((reg(3) << 2) | 3) & 0x3f);
		}
		break;

	case 1: // 16K
		{
			const int lower = outer | ((reg(1) & 0x1f) << 1);
			const int upper = outer | ((sw_last ? (reg(3) & 0x1f) : 0x1f) << 1);
			prg8_x(0, lower);
			prg8_x(1, lower + 1);
			prg8_x(2, upper);
			prg8_x(3, upper + 1);
			m_bank_6000 = outer | (((reg(3) << 1) | 1) & 0x3f);
		}
		break;

	case 2: // 8K
	case 3: // 8K, bank bits reversed
		prg8_x(0, outer | (reg(0) & 0x3f));
		prg8_x(1, outer | (reg(1) & 0x3f));
		prg8_x(2, outer | (reg(2) & 0x3f));
		prg8_x(3, outer | (sw_last ? (reg(3) & 0x3f) : 0x3f));
		m_bank_6000 = outer | (reg(3) & 0x3f);
		break;
	}
}

// bank number, in units of 1K >> shift, of CHR register reg: the register is masked to 256K (outer bit A18 from the
// outer bank register) or to 512K (outer bit ignored), with A19-A20 always coming from the outer bank register
uint32_t nes_jy_typea_device::chr_window(int reg, int shift) const
{
	const bool large = BIT(m_outer, 5);
	const uint32_t mask = large ? 0x1ff : 0xff;
	const uint32_t outer = (uint32_t(BIT(m_outer, 3, 2)) << 9) | (large ? 0 : (uint32_t(BIT(m_outer, 0)) << 8));
	const uint32_t value = m_chr_lo[reg] | (uint32_t(m_chr_hi[reg]) << 8);

	return (value & (mask >> shift)) | (outer >> shift);
}

void nes_jy_typea_device::update_chr()
{
	const bool mmc4 = BIT(m_outer, 7);

	switch (BIT(m_mode, 3, 2))
	{
	case 0: // 8K
		chr8(chr_window(0, 3), m_chr_source);
		break;

	case 1: // 4K, with the MMC4-like latches picking the second register of each half
		chr4_x(0, chr_window(mmc4 ? (m_chr_latch[0] << 1) : 0, 2), m_chr_source);
		chr4_x(4, chr_window(mmc4 ? (4 | (m_chr_latch[1] << 1)) : 4, 2), m_chr_source);
		break;

	case 2: // 2K
		for (int i = 0; i < 8; i += 2)
			chr2_x(i, chr_window(i, 1), m_chr_source);
		break;

	case 3: // 1K
		for (int i = 0; i < 8; i++)
			chr1_x(i, chr_window(i, 0), m_chr_source);
		break;
	}
}

void nes_jy_typea_device::update_nt()
{
	// mapper 90 inhibits ROM nametables and extended mirroring by jumper
	const bool rom_nt = !m_nt_jumper && BIT(m_mode, 5);
	const bool extended = !m_nt_jumper && BIT(m_mirror, 3);

	if (rom_nt && m_vrom_size)
	{
		for (int i = 0; i < 4; i++)
		{
			// all nametables from ROM, or per nametable: $B00x bit 7 against $D002 bit 7
			if (BIT(m_mode, 6) || (BIT(m_nt_lo[i], 7) != BIT(m_ppucfg, 7)))
				set_nt_page(i, VROM, m_nt_lo[i] | (m_nt_hi[i] << 8), 0);
			else
				set_nt_page(i, CIRAM, m_nt_lo[i] & 1, 1);
		}
	}
	else if (rom_nt || extended)
	{
		for (int i = 0; i < 4; i++)
			set_nt_page(i, CIRAM, m_nt_lo[i] & 1, 1);
	}
	else
	{
		switch (m_mirror & 3)
		{
		case 0: set_nt_mirroring(PPU_MIRROR_VERT); break;   // A10
		case 1: set_nt_mirroring(PPU_MIRROR_HORZ); break;   // A11
		case 2: set_nt_mirroring(PPU_MIRROR_LOW); break;
		case 3: set_nt_mirroring(PPU_MIRROR_HIGH); break;
		}
	}
}


//-------------------------------------------------
//  IRQ
//-------------------------------------------------

void nes_jy_typea_device::irq_disable()
{
	// acknowledges a pending IRQ, inhibits counting and clears the prescaler
	m_irq_enable = 0;
	m_irq_prescaler = 0;
	set_irq_line(CLEAR_LINE);
}

void nes_jy_typea_device::irq_tick()
{
	if (!m_irq_enable || (m_irq_dir != 1 && m_irq_dir != 2))
		return;

	const uint8_t mask = m_irq_small ? 0x07 : 0xff;
	uint8_t prescaler = m_irq_prescaler & mask;
	bool wrap;

	if (m_irq_dir == 1)
	{
		prescaler = (prescaler + 1) & mask;
		wrap = (prescaler == 0);
	}
	else
	{
		prescaler = (prescaler - 1) & mask;
		wrap = (prescaler == mask);
	}
	m_irq_prescaler = (m_irq_prescaler & ~mask) | prescaler;

	if (wrap)
	{
		if (m_irq_dir == 1)
		{
			m_irq_counter++;
			if (m_irq_counter == 0)
				set_irq_line(ASSERT_LINE);
		}
		else
		{
			m_irq_counter--;
			if (m_irq_counter == 0xff)
				set_irq_line(ASSERT_LINE);
		}
	}
}

void nes_jy_typea_device::m2_w(int state, offs_t address, bool write)
{
	if (!state)
		return;

	if (m_irq_source == 0 || (m_irq_source == 3 && write))
		irq_tick();
}

void nes_jy_typea_device::ppu_a12_level(int state)
{
	const int rise = state && !m_a12;
	m_a12 = state ? 1 : 0;

	if (rise && m_irq_source == 1)
		irq_tick();
}


//-------------------------------------------------
//  PPU accessors
//-------------------------------------------------

uint8_t nes_jy_typea_device::chr_r(offs_t offset)
{
	if (m_irq_source == 2)
		irq_tick();

	return nes_nrom_device::chr_r(offset);
}

uint8_t nes_jy_typea_device::nt_r(offs_t offset)
{
	if (m_irq_source == 2)
		irq_tick();

	return nes_nrom_device::nt_r(offset);
}

void nes_jy_typea_device::chr_w(offs_t offset, uint8_t data)
{
	// $D002 bit 6: CHR write enable
	if (BIT(m_ppucfg, 6))
		nes_nrom_device::chr_w(offset, data);
}

// MMC4-like mode ($D003 bit 7): the latch of each 4K half is set by PPU reads of $xFD8-$xFDF (0) and $xFE8-$xFEF (1)
// and takes effect for the following reads
void nes_jy_typea_device::ppu_read_latch(offs_t offset)
{
	if (!BIT(m_outer, 7))
		return;

	const int half = BIT(offset, 12);
	int latch;

	switch (offset & 0x0ff8)
	{
	case 0x0fd8: latch = 0; break;
	case 0x0fe8: latch = 1; break;
	default: return;
	}

	if (m_chr_latch[half] != latch)
	{
		m_chr_latch[half] = latch;
		if (BIT(m_mode, 3, 2) == 1)
			update_chr();
	}
}


//-------------------------------------------------
//  CPU accessors
//-------------------------------------------------

uint8_t nes_jy_typea_device::read_l(offs_t offset)
{
	const offs_t addr = offset + 0x4100;

	LOG("JY Company read_l, addr: %04x\n", addr);

	// solder pads, fully decoded
	if (addr == 0x5000 || addr == 0x5400 || addr == 0x5c00)
		return (get_open_bus() & 0x3f) | (m_jumper->read() & 0xc0);

	if ((addr & 0xf800) == 0x5800)
	{
		switch (addr & 3)
		{
		case 0: return (m_mul[0] * m_mul[1]) & 0xff;
		case 1: return ((m_mul[0] * m_mul[1]) >> 8) & 0xff;
		case 2: return m_acc;
		case 3: return m_test;
		}
	}

	return get_open_bus();
}

void nes_jy_typea_device::write_l(offs_t offset, uint8_t data)
{
	const offs_t addr = offset + 0x4100;

	LOG("JY Company write_l, addr: %04x, data: %02x\n", addr, data);

	if ((addr & 0xf800) == 0x5800)
	{
		switch (addr & 3)
		{
		case 0: m_mul[0] = data; break;
		case 1: m_mul[1] = data; break;
		case 2: m_acc += data; break;
		case 3: m_acc = 0; m_test = data; break;
		}
	}
}

uint8_t nes_jy_typea_device::read_m(offs_t offset)
{
	LOG("JY Company read_m, offset: %04x\n", offset);

	if (BIT(m_mode, 7))
		return m_prg[(m_bank_6000 & m_prg_mask) * 0x2000 + (offset & 0x1fff)];

	return nes_nrom_device::read_m(offset);
}

void nes_jy_typea_device::write_m(offs_t offset, uint8_t data)
{
	if (!BIT(m_mode, 7))
		nes_nrom_device::write_m(offset, data);
}

void nes_jy_typea_device::write_h(offs_t offset, uint8_t data)
{
	const offs_t addr = offset | 0x8000;

	LOG("JY Company write_h, addr: %04x, data: %02x\n", addr, data);

	switch (addr & 0xf000)
	{
	case 0x8000:
		if (!BIT(addr, 11))
		{
			m_prg_reg[addr & 3] = data & 0x7f;
			update_prg();
		}
		break;

	case 0x9000:
		if (!BIT(addr, 11))
		{
			m_chr_lo[addr & 7] = data;
			update_chr();
		}
		break;

	case 0xa000:
		if (!BIT(addr, 11))
		{
			m_chr_hi[addr & 7] = data;
			update_chr();
		}
		break;

	case 0xb000:
		if (!BIT(addr, 11))
		{
			if (BIT(addr, 2))
				m_nt_hi[addr & 3] = data;
			else
				m_nt_lo[addr & 3] = data;
			update_nt();
		}
		break;

	case 0xc000:
		switch (addr & 7)
		{
		case 0:
			if (BIT(data, 0))
				m_irq_enable = 1;
			else
				irq_disable();
			break;
		case 1:
			m_irq_source = data & 3;
			m_irq_small = BIT(data, 2);
			m_irq_funky = BIT(data, 3);
			m_irq_dir = BIT(data, 6, 2);
			break;
		case 2:
			irq_disable();
			break;
		case 3:
			m_irq_enable = 1;
			break;
		case 4:
			m_irq_prescaler = data ^ m_irq_xor;
			break;
		case 5:
			m_irq_counter = data ^ m_irq_xor;
			break;
		case 6:
			m_irq_xor = data;
			break;
		case 7:
			m_irq_c007 = data;      // "funky" mode configuration, not emulated
			break;
		}
		break;

	case 0xd000:
		if (!BIT(addr, 11))
		{
			switch (addr & 3)
			{
			case 0:
				m_mode = data;
				update_prg();
				update_chr();
				update_nt();
				break;
			case 1:
				m_mirror = data;
				update_nt();
				break;
			case 2:
				m_ppucfg = data;
				update_nt();
				break;
			case 3:
				m_outer = data;
				update_prg();
				update_chr();
				break;
			}
		}
		break;
	}
}
