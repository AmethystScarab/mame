// license:GPL-2.0+
// copyright-holders:Matthew Conte,Brian H. Gilbert
/*****************************************************************************

  MAME/MESS NES APU CORE

  Based on the Nofrendo/Nosefart NES RP2A03 sound emulation core written by
  Matthew Conte (matt@conte.com) and redesigned for use in MAME/MESS by
  Who Wants to Know? (wwtk@mail.com)

  This core is written with the advise and consent of Matthew Conte and is
  released under the GNU Public License.

 *****************************************************************************

   NES_APU.H

   NES APU external interface.

 *****************************************************************************/

#ifndef MAME_SOUND_NES_APU_H
#define MAME_SOUND_NES_APU_H

#pragma once


#include "nes_defs.h"

/* AN EXPLANATION
 *
 * The NES APU is actually integrated into the Nintendo processor.
 * You must supply the same number of APUs as you do processors.
 * Also make sure to correspond the memory regions to those used in the
 * processor, as each is shared.
 */

class nesapu_device : public device_t,
						public device_sound_interface
{
public:
	nesapu_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	// configuration helpers
	auto irq() { return m_irq_handler.bind(); }
	auto mem_read() { return m_mem_read_cb.bind(); }
	// DMC DMA: when connected, sample fetches are timed in CPU cycles and requested from the CPU's DMA unit
	// (the callback gets the address), which hands the byte back through dmc_dma_done()
	auto dmc_dma() { return m_dmc_dma_cb.bind(); }
	void dmc_dma_done(u8 data);

	virtual void device_reset() override ATTR_COLD;
	virtual void device_clock_changed() override;

	u8 status_r();
	// DMC memory reader, stepped by the CPU after its access of every cycle (TriCNES's lockstep 2A03 model)
	void dmc_cycle_end(bool get);
	bool dmc_dma_allowed() const { return m_dmc_status || m_dmc_impl_abort; }
	void dmc_implicit_clear() { m_dmc_impl_abort = false; }
	u16 dmc_address() const { return m_dmc_addr; }
	void write(offs_t offset, u8 data);

protected:
	nesapu_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock);

	// device-level overrides
	virtual void device_start() override ATTR_COLD;

	// sound stream update overrides
	virtual void sound_stream_update(sound_stream &stream) override;

	virtual void update_lfsr(apu_t::noise_t &chan);

private:
	/* GLOBAL CONSTANTS */
	static constexpr unsigned  SYNCS_MAX1     = 0x20;
	static constexpr unsigned  SYNCS_MAX2     = 0x80;
	static constexpr u32       NTSC_APU_CLOCK = 21477272 / 12;
	static constexpr u32       PAL_APU_CLOCK  = 26601712 / 16;

	// internal state
	apu_t   m_APU;                   /* Actual APUs */
	u8      m_is_pal;
	u32     m_samps_per_sync;        /* Number of samples per vsync */
	u32     m_vbl_times[SYNCS_MAX1];   /* VBL durations in samples */
	u32     m_sync_times1[SYNCS_MAX1]; /* Samples per sync table */
	u32     m_sync_times2[SYNCS_MAX2]; /* Samples per sync table */
	sound_stream::sample_t m_square_lut[31];       // Non-linear Square wave output LUT
	sound_stream::sample_t m_tnd_lut[16][16][128]; // Non-linear Triangle, Noise, DMC output LUT

	sound_stream *m_stream;
	devcb_write_line m_irq_handler;
	devcb_read8 m_mem_read_cb;
	devcb_write16 m_dmc_dma_cb;

	// cycle-timed DMC memory reader (NESdev APU DMC), active when the DMA callback is connected
	bool m_dmc_engine;
	emu_timer *m_dmc_timer;        // output unit: one tick per bit at the DMC rate
	emu_timer *m_dmc_load_timer;   // fetch after enabling the channel with an empty sample buffer
	u16 m_dmc_addr;
	u16 m_dmc_bytes;               // bytes remaining
	u8 m_dmc_buf;
	bool m_dmc_buf_full;
	int m_dmc_bits;                // bits left in the output unit's current cycle
	u8 m_dmc_fifo[256];            // fetched bytes, consumed in order by the sound stream
	int m_dmc_fifo_head, m_dmc_fifo_count;
	u8 m_dmc_shift;                // sound side: shift register being played
	int m_dmc_shift_bits;
	bool m_dmc_silent;
	bool m_dmc_fetching;            // a fetch was requested and the byte has not arrived yet
	// a $4015 enable that restarts the sample while a byte is still playing starts no load DMA (the output unit is not
	// silent); the fetch at the next byte boundary is an ordinary reload, but cannot start before the enable takes
	// effect on m_dmc_enable_cycle
	bool m_dmc_load_pending;
	u16 m_dmc_lfsr = 0;            // the DMC rate timer: a 9-bit LFSR, stepped once per APU cycle, reloaded when it reaches 1
	bool m_dmc_status = false;      // APU_Status_DMC: lets DMC DMAs run
	bool m_dmc_status_next = false; // value a $4015 write is moving it to
	u8 m_dmc_status_delay = 0;      // CPU cycles until it gets there
	u8 m_dmc_load_delay = 0;        // APU cycles until a load DMA (enable while quiet)
	bool m_dmc_quiet = true;        // TriCNES APU_Silent: the last byte boundary had nothing to play
	bool m_dmc_impl_abort = false;  // implicit abort: a DMC DMA may run for one cycle with the status off
	bool m_dmc_impl_abort_set = false;
	void dmc_step_lfsr();
	void dmc_step_sample();
	u64 m_dmc_enable_cycle = 0;
	attotime m_dmc_reload_time;  // when the DMC timer last reloaded (a $4010 write in that same CPU cycle still counts)
	// NESdev/AccuracyCoin "Frame Counter IRQ": a $4015 read on a "put" cycle defers the frame IRQ flag's
	// clear to the next "get" cycle, rather than clearing it immediately
	emu_timer *m_fc_clear_timer;
	u64 m_fc_clear_due = 0;      // CPU cycle a pending $4015-read clear of the frame IRQ flag takes effect on
	TIMER_CALLBACK_MEMBER(fc_clear_tick);
	TIMER_CALLBACK_MEMBER(dmc_bit_tick);

	// cycle-accurate APU core (NESdev APU, APU Frame Counter, APU Envelope/Length Counter/Sweep/Pulse/Triangle/Noise),
	// used together with the DMC reader above when the APU sits in a 2A03; the VT APUs keep the legacy model
	struct acc_env_t { bool start = false; u8 divider = 0; u8 decay = 0; };
	struct acc_pulse_t
	{
		u8 duty = 0, step = 0, vol = 0, length = 0;
		bool halt = false, constvol = false, enabled = false;
		acc_env_t env;
		bool sweep_en = false, sweep_neg = false, sweep_reload = false;
		u8 sweep_p = 0, sweep_shift = 0, sweep_div = 0;
		u16 period = 0;
		int timer = 0;
	};
	struct acc_tri_t
	{
		bool control = false, reload_flag = false, enabled = false;
		u8 reload = 0, linear = 0, length = 0, step = 0;
		u16 period = 0;
		int timer = 0;
	};
	struct acc_noise_t
	{
		bool halt = false, constvol = false, mode = false, enabled = false;
		u8 vol = 0, period_idx = 0, length = 0;
		acc_env_t env;
		u16 lfsr = 1;
		int timer = 0;
	};
	bool m_acc;
	acc_pulse_t m_p[2];
	acc_tri_t m_t;
	acc_noise_t m_n;
	bool m_fc_mode5, m_fc_inhibit;
	int m_fc_step;
	attotime m_fc_base;
	u8 m_fc_value;            // last value written to $4017 (rewritten on reset)
	emu_timer *m_fc_timer;
	emu_timer *m_fc_reset_timer;
	emu_timer *m_fc_irq_timer;
	// same-cycle races with the half-frame length clock (blargg apu_test readme): halt writes take effect one
	// clock late, and a length reload during the clock is dropped if the counter was non-zero before it
	u64 m_half_cycle;        // CPU cycle of the last half-frame clock
	u8 m_len_before[4];      // length counters just before that clock (pulse 1, pulse 2, triangle, noise)
	bool m_halt_old[4];      // halt flag before the last halt write
	u64 m_halt_cycle[4];     // CPU cycle of the last halt write
	u64 m_skip_clock[4];     // a reload that collided with a half-frame clock: that clock does not decrement
	bool half_clock_due(u64 cycle) const;
	u8 &acc_length(int ch);
	bool &acc_halt(int ch);
	void acc_halt_write(int ch, bool halt);
	void acc_length_load(int ch, u8 value);
	TIMER_CALLBACK_MEMBER(fc_irq_tick);
	TIMER_CALLBACK_MEMBER(fc_step_tick);
	TIMER_CALLBACK_MEMBER(fc_reset_tick);
	void fc_schedule();
	u64 cycle_at(const attotime &t) const;
	void acc_reset();
	void acc_write(offs_t offset, u8 value);
	void acc_enable(u8 value);
	u8 acc_status();
	void acc_quarter();
	void acc_half();
	void acc_advance(int cycles);
	u8 acc_pulse_out(int i) const;
	u16 acc_sweep_target(int i) const;
	TIMER_CALLBACK_MEMBER(dmc_load_tick);
	void dmc_fetch(bool load);

	emu_timer *m_frame_timer;
	attotime m_frame_period;
	u16 m_frame_clocks;

	TIMER_CALLBACK_MEMBER(frame_timer_cb);
	void calculate_rates();
	void apu_square(apu_t::square_t *chan);
	void apu_triangle(apu_t::triangle_t *chan);
	void apu_noise(apu_t::noise_t *chan);
	void apu_dpcm(apu_t::dpcm_t *chan);
};

class apu2a03_device : public nesapu_device
{
public:
	apu2a03_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

protected:
	virtual void update_lfsr(apu_t::noise_t &chan) override;
};


DECLARE_DEVICE_TYPE(NES_APU,  nesapu_device)
DECLARE_DEVICE_TYPE(APU_2A03, apu2a03_device)

#endif // MAME_SOUND_NES_APU_H
