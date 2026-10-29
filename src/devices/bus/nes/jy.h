// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
#ifndef MAME_BUS_NES_JY_H
#define MAME_BUS_NES_JY_H

#pragma once

#include "nxrom.h"


// ======================> nes_jy_typea_device
//
// J.Y. Company ASIC. Mapper 90 is the board with the solder jumper that inhibits ROM nametables and extended mirroring,
// mappers 209 and 211 the ones without it (NESdev: 211 is a duplicate of 209 with correct emulation)

class nes_jy_typea_device : public nes_nrom_device
{
public:
	// construction/destruction
	nes_jy_typea_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual uint8_t read_l(offs_t offset) override;
	virtual uint8_t read_m(offs_t offset) override;
	virtual void write_l(offs_t offset, uint8_t data) override;
	virtual void write_m(offs_t offset, uint8_t data) override;
	virtual void write_h(offs_t offset, uint8_t data) override;

	virtual uint8_t chr_r(offs_t offset) override;
	virtual void chr_w(offs_t offset, uint8_t data) override;
	virtual uint8_t nt_r(offs_t offset) override;

	// IRQ sources: CPU M2 rise and CPU writes (M2), unfiltered PPU A12 rise, every PPU read
	virtual bool wants_m2() override { return true; }
	virtual void m2_w(int state, offs_t address, bool write) override;
	virtual bool wants_a12_level() override { return true; }
	virtual void ppu_a12_level(int state) override;

	// MMC4-like CHR latches
	virtual bool wants_read_latch() override { return true; }
	virtual void ppu_read_latch(offs_t offset) override;

	virtual void pcb_reset() override;

protected:
	nes_jy_typea_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock, bool nt_jumper);

	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;

private:
	void update_prg();
	void update_chr();
	void update_nt();
	void irq_tick();
	void irq_disable();
	uint32_t chr_window(int reg, int shift) const;

	required_ioport m_jumper;
	const bool m_nt_jumper;     // mapper 90: ROM nametables and extended mirroring are inhibited

	// $5800-$5803
	uint8_t m_mul[2];
	uint8_t m_acc;
	uint8_t m_test;

	// $8000-$B007
	uint8_t m_prg_reg[4];
	uint8_t m_chr_lo[8];
	uint8_t m_chr_hi[8];
	uint8_t m_nt_lo[4];
	uint8_t m_nt_hi[4];

	// $D000-$D003
	uint8_t m_mode;
	uint8_t m_mirror;
	uint8_t m_ppucfg;
	uint8_t m_outer;

	uint8_t m_chr_latch[2];     // MMC4-like mode: selects the second register of each 4K half
	uint32_t m_bank_6000;

	// $C000-$C007
	uint8_t m_irq_enable;
	uint8_t m_irq_source;
	uint8_t m_irq_dir;
	uint8_t m_irq_small;
	uint8_t m_irq_funky;
	uint8_t m_irq_prescaler;
	uint8_t m_irq_counter;
	uint8_t m_irq_xor;
	uint8_t m_irq_c007;
	uint8_t m_a12;
};


// ======================> nes_jy_typeb_device

class nes_jy_typeb_device : public nes_jy_typea_device
{
public:
	// construction/destruction
	nes_jy_typeb_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	nes_jy_typeb_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);
};

// ======================> nes_jy_typec_device

class nes_jy_typec_device : public nes_jy_typeb_device
{
public:
	// construction/destruction
	nes_jy_typec_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};


// device type definition
DECLARE_DEVICE_TYPE(NES_JY_TYPEA, nes_jy_typea_device)
DECLARE_DEVICE_TYPE(NES_JY_TYPEB, nes_jy_typeb_device)
DECLARE_DEVICE_TYPE(NES_JY_TYPEC, nes_jy_typec_device)

#endif // MAME_BUS_NES_JY_H
