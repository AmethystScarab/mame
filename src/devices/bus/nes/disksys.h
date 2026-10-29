// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
#ifndef MAME_BUS_NES_DISKSYS_H
#define MAME_BUS_NES_DISKSYS_H

#pragma once

#include "nxrom.h"
#include "imagedev/flopdrv.h"
#include "sound/rp2c33_snd.h"


// ======================> nes_disksys_device

class nes_disksys_device : public nes_nrom_device
{
public:
	// construction/destruction
	nes_disksys_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual uint8_t read_ex(offs_t offset) override;
	virtual uint8_t read_m(offs_t offset) override;
	virtual uint8_t read_h(offs_t offset) override;
	virtual void write_ex(offs_t offset, uint8_t data) override;
	virtual void write_m(offs_t offset, uint8_t data) override;
	virtual void write_h(offs_t offset, uint8_t data) override;

	virtual void disk_flip_side() override;

	virtual void pcb_reset() override;

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;

	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;

	required_region_ptr<uint8_t> m_2c33_rom;

private:
	static void load_proc(device_image_interface &image, bool is_created);
	static void unload_proc(device_image_interface &image);

	TIMER_CALLBACK_MEMBER(irq_timer_tick);
	// M2 at the cartridge connector: the IRQ counter is clocked once per CPU cycle, at the M2 rise (before that cycle's access)
	virtual bool wants_m2() override { return true; }
	virtual void m2_w(int state, offs_t address, bool write) override { if (state) irq_timer_tick(0); }
	// Brad Taylor's FDS technical reference documents a disk/RAM-adaptor transfer rate of 96.4 kHz +/-10%
	// (https://archive.nesdev.org/fdstech.txt); at NTSC's ~1.7898 MHz CPU clock that is ~18.57 CPU cycles per
	// bit, i.e. ~149 CPU cycles per 8-bit byte - matching the ~12 KB/s transfer rate independently confirmed on
	// the NESdev forum ("Using FDS Disk IRQ (not Timer IRQ) for frame timing") and already used by puNES as
	// drive_delay. The transfer IRQ is timed from this rate, not from the PPU hblank callback.
	TIMER_CALLBACK_MEMBER(transfer_tick);
	static constexpr unsigned TRANSFER_CYCLES = 149;

	std::unique_ptr<uint8_t[]> m_fds_data;    // here, we store a copy of the disk
	required_device<legacy_floppy_image_device> m_disk;
	required_device<rp2c33_sound_device> m_sound;

	emu_timer *irq_timer;
	emu_timer *transfer_timer;

	void load_disk(device_image_interface &image);
	void unload_disk(device_image_interface &image);

	uint16_t m_irq_count, m_irq_count_latch;
	bool m_irq_enable, m_irq_repeat, m_irq_transfer;
	bool m_disk_reg_enable;
	bool m_sound_en;

	uint8_t m_fds_motor_on;
	uint8_t m_fds_door_closed;
	uint8_t m_fds_current_side;
	uint32_t m_fds_head_position;
	uint8_t m_fds_status0;
	uint8_t m_read_mode;
	uint8_t m_drive_ready;

	uint8_t m_fds_sides;
	int32_t m_fds_last_side;
	int32_t m_fds_count;
};


// device type definition
DECLARE_DEVICE_TYPE(NES_DISKSYS, nes_disksys_device)

#endif // MAME_BUS_NES_DISKSYS_H
