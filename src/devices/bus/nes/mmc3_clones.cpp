// license:BSD-3-Clause
// copyright-holders:Fabio Priuli,Brian H. Gilbert
/***********************************************************************************************************


 NES/Famicom cartridge emulation for MMC-3 clone PCBs

 iNES mappers: 44, 45, 49, 52, 114, 115, 121, 123, 126, 134, 182, 187, 189, 196, 197, 205, 208, 215,
               219, 238, 250, 254, 258, 259, 262, 263, 268, 287, 291, 292, 294, 313, 322, 325, 333,
               334, 339, 345, 348, 351, 353, 356, 361, 364, 366, 370, 372, 376, 377, 392, 393, 401,
               410, 411, 422, 512, 513, 516, 534


 Here we emulate several pirate PCBs based on MMC-3 boards

 ***********************************************************************************************************/


#include "emu.h"
#include "mmc3_clones.h"

#include <bit>
#include <cassert>

#define LOG_HIFREQ (1U << 1)

#ifdef NES_PCB_DEBUG
#define VERBOSE (LOG_GENERAL)
#else
#define VERBOSE (0)
#endif
#include "logmacro.h"


//-------------------------------------------------
//  constructor
//-------------------------------------------------

DEFINE_DEVICE_TYPE(NES_NITRA,         nes_nitra_device,         "nes_nitra",         "NES Cart Nitra PCB")
DEFINE_DEVICE_TYPE(NES_BMW8544,       nes_bmw8544_device,       "nes_bmw8544",       "NES Cart BMW8544 PCB")
DEFINE_DEVICE_TYPE(NES_FS6,           nes_fs6_device,           "nes_fs6",           "NES Cart Fight Street VI PCB")
DEFINE_DEVICE_TYPE(NES_SBROS11,       nes_sbros11_device,       "nes_smb11",         "NES Cart SMB 11 PCB")
DEFINE_DEVICE_TYPE(NES_MALISB,        nes_malisb_device,        "nes_malisb",        "NES Cart Mali Splash Bomb PCB")
DEFINE_DEVICE_TYPE(NES_FAMILY4646,    nes_family4646_device,    "nes_family4646",    "NES Cart BMC-FAMILY4646 PCB")
DEFINE_DEVICE_TYPE(NES_PIKAY2K,       nes_pikay2k_device,       "nes_pikay2k",       "NES Cart PIKACHU Y2K PCB")
DEFINE_DEVICE_TYPE(NES_8237,          nes_8237_device,          "nes_8237",          "NES Cart UNL-8237 PCB")
DEFINE_DEVICE_TYPE(NES_8237A,         nes_8237a_device,         "nes_8237a",         "NES Cart UNL-8237A PCB")
DEFINE_DEVICE_TYPE(NES_158B,          nes_158b_device,          "nes_158b",          "NES Cart UNL-158B PCB")
DEFINE_DEVICE_TYPE(NES_SG_LIONK,      nes_sglionk_device,       "nes_sglionk",       "NES Cart SuperGame Lion King PCB")
DEFINE_DEVICE_TYPE(NES_SG_BOOG,       nes_sgboog_device,        "nes_sgbooger",      "NES Cart SuperGame BoogerMan PCB")
DEFINE_DEVICE_TYPE(NES_KASING,        nes_kasing_device,        "nes_kasing",        "NES Cart Kasing PCB")
DEFINE_DEVICE_TYPE(NES_KAY,           nes_kay_device,           "nes_kay",           "NES Cart KAY PCB")
DEFINE_DEVICE_TYPE(NES_H2288,         nes_h2288_device,         "nes_h2288",         "NES Cart H-2288 PCB")
DEFINE_DEVICE_TYPE(NES_6035052,       nes_6035052_device,       "nes_6035052",       "NES Cart UNL-603-5052 PCB")
DEFINE_DEVICE_TYPE(NES_TXC_TW,        nes_txc_tw_device,        "nes_txc_tw",        "NES Cart TXC Thunder Warrior PCB")
DEFINE_DEVICE_TYPE(NES_KOF97,         nes_kof97_device,         "nes_kof97",         "NES Cart KoF 97 PCB")
DEFINE_DEVICE_TYPE(NES_KOF96,         nes_kof96_device,         "nes_kof96",         "NES Cart KoF 96 PCB")
DEFINE_DEVICE_TYPE(NES_SF3,           nes_sf3_device,           "nes_sf3",           "NES Cart Super Fighter III PCB")
DEFINE_DEVICE_TYPE(NES_COCOMA,        nes_cocoma_device,        "nes_cocoma",        "NES Cart Cocoma Core Pro PCB")
DEFINE_DEVICE_TYPE(NES_GOUDER,        nes_gouder_device,        "nes_gouder",        "NES Cart Gouder PCB")
DEFINE_DEVICE_TYPE(NES_SA9602B,       nes_sa9602b_device,       "nes_sa9602b",       "NES Cart SA-9602B PCB")
DEFINE_DEVICE_TYPE(NES_SACHEN_SHERO,  nes_sachen_shero_device,  "nes_shero",         "NES Cart Street Hero PCB")
DEFINE_DEVICE_TYPE(NES_SACHEN_ZGDH,   nes_sachen_zgdh_device,   "nes_zgdh",          "NES Cart Zhongguo Daheng PCB")
DEFINE_DEVICE_TYPE(NES_A9746,         nes_a9746_device,         "nes_bmc_a9746",     "NES Cart A-9746 PCB")

DEFINE_DEVICE_TYPE(NES_A88S1,         nes_a88s1_device,         "nes_a88s1",         "NES Cart BMC A88S-1 PCB")
DEFINE_DEVICE_TYPE(NES_BMC_EL86XC,    nes_bmc_el86xc_device,    "nes_bmc_el86xc",    "NES Cart BMC EL860947C/EL861121C PCB")
DEFINE_DEVICE_TYPE(NES_FK23C,         nes_fk23c_device,         "nes_fk23c",         "NES Cart FK23C PCB")
DEFINE_DEVICE_TYPE(NES_FK23CA,        nes_fk23ca_device,        "nes_fk23ca",        "NES Cart FK23CA PCB")
DEFINE_DEVICE_TYPE(NES_NT639,         nes_nt639_device,         "nes_nt639",         "NES Cart NT-639 PCB")
DEFINE_DEVICE_TYPE(NES_RESETTXROM,    nes_resettxrom_device,    "nes_resettxrom",    "NES Cart BMC RESET-TXROM PCB")
DEFINE_DEVICE_TYPE(NES_S24IN1SC03,    nes_s24in1sc03_device,    "nes_s24in1c03",     "NES Cart Super 24 in 1 SC-03 PCB")
DEFINE_DEVICE_TYPE(NES_TECHLINE9IN1,  nes_tech9in1_device,      "nes_tech9in1",      "NES Cart Techline 9 in 1 PCB")
DEFINE_DEVICE_TYPE(NES_BMC_5IN1,      nes_bmc_5in1_device,      "nes_bmc_5in1",      "NES Cart BMC 5 in 1 1993 Copyright PCB")
DEFINE_DEVICE_TYPE(NES_BMC_8IN1,      nes_bmc_8in1_device,      "nes_bmc_8in1",      "NES Cart BMC GRM070 8 in 1 PCB")
DEFINE_DEVICE_TYPE(NES_BMC_15IN1,     nes_bmc_15in1_device,     "nes_bmc_15in1",     "NES Cart BMC 15 in 1 PCB")
DEFINE_DEVICE_TYPE(NES_BMC_SBIG7,     nes_bmc_sbig7_device,     "nes_bmc_sbig7",     "NES Cart BMC Super BIG 7 in 1 PCB")
DEFINE_DEVICE_TYPE(NES_BMC_HIK8,      nes_bmc_hik8_device,      "nes_bmc_hik8",      "NES Cart BMC HIK 8 in 1 PCB")
DEFINE_DEVICE_TYPE(NES_BMC_JY208,     nes_bmc_jy208_device,     "nes_bmc_jy208",     "NES Cart BMC JY-208 PCB")
DEFINE_DEVICE_TYPE(NES_BMC_JY302,     nes_bmc_jy302_device,     "nes_bmc_jy302",     "NES Cart BMC JY-302 PCB")
DEFINE_DEVICE_TYPE(NES_BMC_KC885,     nes_bmc_kc885_device,     "nes_bmc_kc885",     "NES Cart BMC KC885 PCB")
DEFINE_DEVICE_TYPE(NES_BMC_SFC12,     nes_bmc_sfc12_device,     "nes_bmc_sfc12",     "NES Cart BMC SFC-12 PCB")
DEFINE_DEVICE_TYPE(NES_BMC_HIK4,      nes_bmc_hik4_device,      "nes_bmc_hik4",      "NES Cart BMC HIK 4 in 1 PCB")
DEFINE_DEVICE_TYPE(NES_BMC_F15,       nes_bmc_f15_device,       "nes_bmc_f15",       "NES Cart BMC F-15 PCB")
DEFINE_DEVICE_TYPE(NES_BMC_F600,      nes_bmc_f600_device,      "nes_bmc_f600",      "NES Cart BMC F600 PCB")
DEFINE_DEVICE_TYPE(NES_BMC_GN45,      nes_bmc_gn45_device,      "nes_bmc_gn45",      "NES Cart BMC GN-45 PCB")
DEFINE_DEVICE_TYPE(NES_BMC_GOLD7IN1,  nes_bmc_gold7in1_device,  "nes_bmc_gold7in1",  "NES Cart BMC Golden 7 in 1 PCB")
DEFINE_DEVICE_TYPE(NES_BMC_K3006,     nes_bmc_k3006_device,     "nes_bmc_k3006",     "NES Cart BMC K-3006 PCB")
DEFINE_DEVICE_TYPE(NES_BMC_K3033,     nes_bmc_k3033_device,     "nes_bmc_k3033",     "NES Cart BMC K-3033 PCB")
DEFINE_DEVICE_TYPE(NES_BMC_L6IN1,     nes_bmc_l6in1_device,     "nes_bmc_l6in1",     "NES Cart BMC L6IN1 PCB")
DEFINE_DEVICE_TYPE(NES_BMC_00202650,  nes_bmc_00202650_device,  "nes_bmc_00202650",  "NES Cart BMC 00202650 PCB")
DEFINE_DEVICE_TYPE(NES_BMC_411120C,   nes_bmc_411120c_device,   "nes_bmc_411120c",   "NES Cart BMC 411120C PCB")
DEFINE_DEVICE_TYPE(NES_BMC_810305C,   nes_bmc_810305c_device,   "nes_bmc_810305c",   "NES Cart BMC 81-03-05-C PCB")
DEFINE_DEVICE_TYPE(NES_BMC_820720C,   nes_bmc_820720c_device,   "nes_bmc_820720c",   "NES Cart BMC 820720C PCB")
DEFINE_DEVICE_TYPE(NES_BMC_830118C,   nes_bmc_830118c_device,   "nes_bmc_830118c",   "NES Cart BMC 830118C PCB")
DEFINE_DEVICE_TYPE(NES_BMC_830832C,   nes_bmc_830832c_device,   "nes_bmc_830832c",   "NES Cart BMC 830832C PCB")
DEFINE_DEVICE_TYPE(NES_BMC_YY841101C, nes_bmc_yy841101c_device, "nes_bmc_yy841101c", "NES Cart BMC YY841101C PCB")
DEFINE_DEVICE_TYPE(NES_BMC_YY841155C, nes_bmc_yy841155c_device, "nes_bmc_yy841155c", "NES Cart BMC YY841155C PCB")
DEFINE_DEVICE_TYPE(NES_PJOY84,        nes_pjoy84_device,        "nes_pjoy84",        "NES Cart TEC9719 / PowerJoy 84 PCB")
DEFINE_DEVICE_TYPE(NES_BS300,         nes_bs300_device,         "nes_bs300",         "NES Cart BS-300 Family PCB")
DEFINE_DEVICE_TYPE(NES_ING022,        nes_ing022_device,        "nes_ing022",        "NES Cart ING-022 PCB")
DEFINE_DEVICE_TYPE(NES_8025,          nes_8025_device,          "nes_8025",          "NES Cart 8025 Enhanced MMC3 PCB")
DEFINE_DEVICE_TYPE(NES_SMD133,        nes_smd133_device,        "nes_smd133",        "NES Cart SMD133 PCB")


INPUT_PORTS_START( bmc_8025 )
	PORT_START("CONFIG")
	PORT_CONFNAME( 0x07, 0x00, "8025 Register Address (Solder Pad)" )
	PORT_CONFSETTING(    0x00, "$5013" )
	PORT_CONFSETTING(    0x01, "$5023" )
	PORT_CONFSETTING(    0x02, "$5043" )
	PORT_CONFSETTING(    0x03, "$5083" )
	PORT_CONFSETTING(    0x04, "$5103" )
	PORT_CONFSETTING(    0x05, "$5203" )
	PORT_CONFSETTING(    0x06, "$5403" )
	PORT_CONFSETTING(    0x07, "$5803" )
INPUT_PORTS_END

INPUT_PORTS_START( family4646 )
	PORT_START("CONFIG")
	PORT_CONFNAME( 0x01, 0x00, "T4A54A solder pad" )
	PORT_CONFSETTING(    0x00, "Low" )
	PORT_CONFSETTING(    0x01, "High" )
INPUT_PORTS_END

INPUT_PORTS_START( sachen_shero )
	PORT_START("JUMPER")
	PORT_CONFNAME( 0x80, 0x00, "Title Screen" )
	PORT_CONFSETTING(    0x00, "Street Heroes" )
	PORT_CONFSETTING(    0x80, u8"\u4f8d\u9b42 (Shìhún)" )    // 侍魂
INPUT_PORTS_END

INPUT_PORTS_START( bmc_5in1 )
	PORT_START("JUMPER")
	PORT_CONFNAME( 0x01, 0x01, "Menu Type" )
	PORT_CONFSETTING(    0x00, "20 in 1" )
	PORT_CONFSETTING(    0x01, "5 in 1" )
INPUT_PORTS_END

INPUT_PORTS_START( bmc_f600 )
	PORT_START("JUMPER")
	PORT_CONFNAME( 0x80, 0x80, "Menu Type" )
	PORT_CONFSETTING(    0x00, "Mario 67 in 1" )
	PORT_CONFSETTING(    0x80, "Mario Party II (6 in 1)" )
INPUT_PORTS_END

INPUT_PORTS_START( bmc_kc885 )
	PORT_START("JUMPER")
	PORT_CONFNAME( 0x07, 0x07, "Menu Type" )
	PORT_CONFSETTING(    0x00, "6000 in 1" )
	PORT_CONFSETTING(    0x01, "400 in 1" )
	PORT_CONFSETTING(    0x02, "800 in 1" )
	PORT_CONFSETTING(    0x03, "5000 in 1" )
	PORT_CONFSETTING(    0x04, "190 in 1" )
	PORT_CONFSETTING(    0x05, "1000 in 1" )
	PORT_CONFSETTING(    0x06, "2000 in 1" )
	PORT_CONFSETTING(    0x07, "19 in 1" )
INPUT_PORTS_END


//-------------------------------------------------
//  input_ports - device-specific input ports
//-------------------------------------------------

ioport_constructor nes_sachen_shero_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( sachen_shero );
}

ioport_constructor nes_bmc_5in1_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( bmc_5in1 );
}

ioport_constructor nes_bmc_f600_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( bmc_f600 );
}

ioport_constructor nes_bmc_kc885_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( bmc_kc885 );
}

ioport_constructor nes_family4646_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( family4646 );
}

ioport_constructor nes_8025_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( bmc_8025 );
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

nes_nitra_device::nes_nitra_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: nes_txrom_device(mconfig, NES_NITRA, tag, owner, clock)
{
}

nes_bmw8544_device::nes_bmw8544_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: nes_txrom_device(mconfig, NES_BMW8544, tag, owner, clock), m_reg{0, 0, 0}, m_bus_latch(0)
{
}

nes_fs6_device::nes_fs6_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: nes_txrom_device(mconfig, NES_FS6, tag, owner, clock)
{
}

nes_sbros11_device::nes_sbros11_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: nes_txrom_device(mconfig, NES_SBROS11, tag, owner, clock)
{
}

nes_malisb_device::nes_malisb_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: nes_txrom_device(mconfig, NES_MALISB, tag, owner, clock)
{
}

nes_family4646_device::nes_family4646_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: nes_txrom_device(mconfig, NES_FAMILY4646, tag, owner, clock), m_config(*this, "CONFIG")
{
}

nes_pikay2k_device::nes_pikay2k_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: nes_txrom_device(mconfig, NES_PIKAY2K, tag, owner, clock)
{
}

nes_8237_device::nes_8237_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock, u8 board)
	: nes_txrom_device(mconfig, type, tag, owner, clock), m_board(board)
{
}

nes_8237_device::nes_8237_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: nes_8237_device(mconfig, NES_8237, tag, owner, clock, 0)
{
}

nes_8237a_device::nes_8237a_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: nes_8237_device(mconfig, NES_8237A, tag, owner, clock, 1)
{
}

nes_158b_device::nes_158b_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: nes_8237_device(mconfig, NES_158B, tag, owner, clock, 0)
{
}

nes_kasing_device::nes_kasing_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock)
	: nes_txrom_device(mconfig, type, tag, owner, clock), m_mmc3_mode(true)
{
}

nes_kasing_device::nes_kasing_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: nes_kasing_device(mconfig, NES_KASING, tag, owner, clock)
{
}

nes_sglionk_device::nes_sglionk_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock, u8 board)
	: nes_kasing_device(mconfig, type, tag, owner, clock), m_board(board)
{
}

nes_sglionk_device::nes_sglionk_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: nes_sglionk_device(mconfig, NES_SG_LIONK, tag, owner, clock, 0)
{
}

nes_sgboog_device::nes_sgboog_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: nes_sglionk_device(mconfig, NES_SG_BOOG, tag, owner, clock, 1)
{
}

nes_kay_device::nes_kay_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: nes_txrom_device(mconfig, NES_KAY, tag, owner, clock)
	, m_ovr{0, 0, 0}
	, m_outer(0x80)
	, m_prot_out(0)
	, m_prot_index(0)
	, m_prot_latch(0)
	, m_prot_lock(0)
{
}

nes_h2288_device::nes_h2288_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: nes_txrom_device(mconfig, NES_H2288, tag, owner, clock), m_mmc3_mode(true)
{
}

nes_6035052_device::nes_6035052_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: nes_txrom_device(mconfig, NES_6035052, tag, owner, clock), m_prot(0)
{
}

nes_txc_tw_device::nes_txc_tw_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: nes_txrom_device(mconfig, NES_TXC_TW, tag, owner, clock)
{
}

nes_kof97_device::nes_kof97_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: nes_txrom_device(mconfig, NES_KOF97, tag, owner, clock)
{
}

nes_kof96_device::nes_kof96_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: nes_txrom_device(mconfig, NES_KOF96, tag, owner, clock), m_mmc3_mode(true)
{
}

nes_sf3_device::nes_sf3_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: nes_txrom_device(mconfig, NES_SF3, tag, owner, clock)
{
}

nes_cocoma_device::nes_cocoma_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: nes_txrom_device(mconfig, NES_COCOMA, tag, owner, clock)
{
}

nes_gouder_device::nes_gouder_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: nes_txrom_device(mconfig, NES_GOUDER, tag, owner, clock), m_submapper(0)
{
}

nes_sa9602b_device::nes_sa9602b_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: nes_txrom_device(mconfig, NES_SA9602B, tag, owner, clock), m_reg(0), m_prg_chip(0)
{
}

nes_sachen_shero_device::nes_sachen_shero_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: nes_txrom_device(mconfig, NES_SACHEN_SHERO, tag, owner, clock)
	, m_jumper(*this, "JUMPER")
	, m_reg(0)
{
}

nes_sachen_zgdh_device::nes_sachen_zgdh_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: nes_txrom_device(mconfig, NES_SACHEN_ZGDH, tag, owner, clock), m_reg(0)
{
}

nes_a9746_device::nes_a9746_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: nes_txrom_device(mconfig, NES_A9746, tag, owner, clock)
	, m_reg{0, 0, 0, 3}
	, m_ext(0)
{
}

nes_a88s1_device::nes_a88s1_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: nes_txrom_device(mconfig, NES_A88S1, tag, owner, clock), m_submapper(0)
{
}

nes_bmc_el86xc_device::nes_bmc_el86xc_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: nes_txrom_device(mconfig, NES_BMC_EL86XC, tag, owner, clock)
{
}

nes_fk23c_device::nes_fk23c_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: nes_txrom_device(mconfig, type, tag, owner, clock), m_mmc_cmd1(0)
{
}

nes_fk23c_device::nes_fk23c_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: nes_fk23c_device(mconfig, NES_FK23C, tag, owner, clock)
{
}

nes_fk23ca_device::nes_fk23ca_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: nes_fk23c_device(mconfig, NES_FK23CA, tag, owner, clock)
{
}

nes_nt639_device::nes_nt639_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: nes_txrom_device(mconfig, NES_NT639, tag, owner, clock)
{
}

nes_resettxrom_device::nes_resettxrom_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: nes_txrom_device(mconfig, NES_RESETTXROM, tag, owner, clock), m_count(-1)
{
}

nes_s24in1sc03_device::nes_s24in1sc03_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: nes_txrom_device(mconfig, NES_S24IN1SC03, tag, owner, clock)
{
}

nes_tech9in1_device::nes_tech9in1_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: nes_txrom_device(mconfig, NES_TECHLINE9IN1, tag, owner, clock)
{
}

nes_bmc_5in1_device::nes_bmc_5in1_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: nes_txrom_device(mconfig, NES_BMC_5IN1, tag, owner, clock)
	, m_jumper(*this, "JUMPER")
{
}

nes_bmc_8in1_device::nes_bmc_8in1_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: nes_txrom_device(mconfig, NES_BMC_8IN1, tag, owner, clock)
{
}

nes_bmc_15in1_device::nes_bmc_15in1_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: nes_txrom_device(mconfig, NES_BMC_15IN1, tag, owner, clock), m_jumper(0)
{
}

nes_bmc_sbig7_device::nes_bmc_sbig7_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: nes_txrom_device(mconfig, NES_BMC_SBIG7, tag, owner, clock)
{
}

nes_bmc_hik8_device::nes_bmc_hik8_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock)
	: nes_txrom_device(mconfig, type, tag, owner, clock), m_count(0)
{
}

nes_bmc_hik8_device::nes_bmc_hik8_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: nes_bmc_hik8_device(mconfig, NES_BMC_HIK8, tag, owner, clock)
{
}

nes_bmc_jy208_device::nes_bmc_jy208_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: nes_bmc_hik8_device(mconfig, NES_BMC_JY208, tag, owner, clock)
{
}

nes_bmc_jy302_device::nes_bmc_jy302_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: nes_bmc_hik8_device(mconfig, NES_BMC_JY302, tag, owner, clock)
{
}

nes_bmc_kc885_device::nes_bmc_kc885_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: nes_bmc_hik8_device(mconfig, NES_BMC_KC885, tag, owner, clock)
	, m_jumper(*this, "JUMPER")
{
}

nes_bmc_sfc12_device::nes_bmc_sfc12_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: nes_bmc_hik8_device(mconfig, NES_BMC_SFC12, tag, owner, clock)
{
}

nes_bmc_hik4_device::nes_bmc_hik4_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: nes_txrom_device(mconfig, NES_BMC_HIK4, tag, owner, clock), m_mmc3_mode(true)
{
}

nes_bmc_f15_device::nes_bmc_f15_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: nes_txrom_device(mconfig, NES_BMC_F15, tag, owner, clock)
{
}

nes_bmc_f600_device::nes_bmc_f600_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: nes_txsrom_device(mconfig, NES_BMC_F600, tag, owner, clock)
	, m_jumper(*this, "JUMPER")
	, m_reg(0)
{
}

nes_bmc_gn45_device::nes_bmc_gn45_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: nes_txrom_device(mconfig, NES_BMC_GN45, tag, owner, clock), m_lock(false)
{
}

nes_bmc_gold7in1_device::nes_bmc_gold7in1_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: nes_txrom_device(mconfig, NES_BMC_GOLD7IN1, tag, owner, clock), m_lock(false)
{
}

nes_bmc_k3006_device::nes_bmc_k3006_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: nes_txrom_device(mconfig, NES_BMC_K3006, tag, owner, clock)
{
}

nes_bmc_k3033_device::nes_bmc_k3033_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: nes_txrom_device(mconfig, NES_BMC_K3033, tag, owner, clock), m_mmc3_mode(false)
{
}

nes_bmc_l6in1_device::nes_bmc_l6in1_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: nes_txrom_device(mconfig, NES_BMC_L6IN1, tag, owner, clock), m_reg(0)
{
}

nes_bmc_00202650_device::nes_bmc_00202650_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: nes_txrom_device(mconfig, NES_BMC_00202650, tag, owner, clock), m_mmc3_mode(false)
{
}

nes_bmc_411120c_device::nes_bmc_411120c_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: nes_txrom_device(mconfig, NES_BMC_411120C, tag, owner, clock), m_reg(0)
{
}

nes_bmc_810305c_device::nes_bmc_810305c_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: nes_txsrom_device(mconfig, NES_BMC_810305C, tag, owner, clock), m_outer(0)
{
}

nes_bmc_820720c_device::nes_bmc_820720c_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: nes_txrom_device(mconfig, NES_BMC_820720C, tag, owner, clock), m_reg(0)
{
}

nes_bmc_830118c_device::nes_bmc_830118c_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: nes_txrom_device(mconfig, NES_BMC_830118C, tag, owner, clock)
{
}

nes_bmc_830832c_device::nes_bmc_830832c_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: nes_txrom_device(mconfig, NES_BMC_830832C, tag, owner, clock)
{
}

nes_bmc_yy841101c_device::nes_bmc_yy841101c_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: nes_txrom_device(mconfig, NES_BMC_YY841101C, tag, owner, clock)
{
}

nes_bmc_yy841155c_device::nes_bmc_yy841155c_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: nes_txrom_device(mconfig, NES_BMC_YY841155C, tag, owner, clock)
{
}

nes_pjoy84_device::nes_pjoy84_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: nes_pjoy84_device(mconfig, NES_PJOY84, tag, owner, clock, 126)
{
}

nes_pjoy84_device::nes_pjoy84_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock, u16 mapper)
	: nes_txrom_device(mconfig, type, tag, owner, clock), m_submapper(0), m_sl0(0xff), m_mapper(mapper)
{
}

nes_bs300_device::nes_bs300_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: nes_pjoy84_device(mconfig, NES_BS300, tag, owner, clock, 422)
{
}

nes_ing022_device::nes_ing022_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: nes_pjoy84_device(mconfig, NES_ING022, tag, owner, clock, 534)
{
}

nes_smd133_device::nes_smd133_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: nes_txrom_device(mconfig, NES_SMD133, tag, owner, clock), m_submapper(0)
{
}




void nes_bmw8544_device::device_start()
{
	mmc3_start();
	save_item(NAME(m_reg));
	save_item(NAME(m_bus_latch));
}

void nes_bmw8544_device::pcb_reset()
{
	std::fill(std::begin(m_reg), std::end(m_reg), 0x00);
	m_bus_latch = 0;
	mmc3_common_initialize(0x0f, 0xff, 0);
}

void nes_family4646_device::device_start()
{
	mmc3_start();
	save_item(NAME(m_reg));
}

void nes_family4646_device::pcb_reset()
{
	std::fill(std::begin(m_reg), std::end(m_reg), 0x00);
	mmc3_common_initialize(0x1f, 0xff, 0);
	set_nt_mirroring(PPU_MIRROR_HORZ); // Space Shuttle on CB-4035 doesn't set mirroring bit. Whether this cart is hard-wired to reset correctly to horizontal mirroring is not clear.
}

void nes_pikay2k_device::device_start()
{
	mmc3_start();
	save_item(NAME(m_reg));
}

void nes_pikay2k_device::pcb_reset()
{
	m_reg[0] = 0xff;
	m_reg[1] = 0;
	mmc3_common_initialize(0xff, 0xff, 0);
}

void nes_8237_device::device_start()
{
	mmc3_start();
	save_item(NAME(m_reg));
}

void nes_8237_device::pcb_reset()
{
	mmc3_common_initialize(0x1f, 0xff, 0);

	m_reg[0] = 0;
	m_reg[1] = 0x0f;
	m_reg[2] = 0;
	update_banks();
}

void nes_158b_device::device_start()
{
	nes_8237_device::device_start();
	save_item(NAME(m_prot));
}

void nes_158b_device::pcb_reset()
{
	nes_8237_device::pcb_reset();
	m_prot = 7;    // Blood of Jurassic needs this row of prot_table
}

void nes_kasing_device::device_start()
{
	mmc3_start();
	save_item(NAME(m_mmc3_mode));
}

void nes_kasing_device::pcb_reset()
{
	m_mmc3_mode = true;
	mmc3_common_initialize(0xff, 0xff, 0);
}

void nes_kay_device::device_start()
{
	mmc3_start();
	save_item(NAME(m_ovr));
	save_item(NAME(m_outer));
	save_item(NAME(m_prot_out));
	save_item(NAME(m_prot_index));
	save_item(NAME(m_prot_latch));
	save_item(NAME(m_prot_lock));
}

void nes_kay_device::pcb_reset()
{
	std::fill(std::begin(m_ovr), std::end(m_ovr), 0x00);
	m_outer = 0x80;     // the outer bank register powers on set
	m_prot_out = 0;
	m_prot_index = 0;
	m_prot_latch = 0;
	m_prot_lock = 0;
	mmc3_common_initialize(0x1f, 0xff, 0);
	m_mmc_prg_bank[0] = 0x00;   // MMC3 registers R6/R7 power on as $00/$01
	m_mmc_prg_bank[1] = 0x01;
	update_outer();
}

void nes_kay_device::update_outer()
{
	m_prg_base = (m_outer & 0x80) >> 2;
	m_chr_base = (m_prg_size == m_vrom_size) ? ((m_outer & 0x80) << 1) : 0;     // only the A9713 (PRG and CHR the same size)
	set_prg(m_prg_base, m_prg_mask);
	set_chr(m_chr_source, m_chr_base, m_chr_mask);
}

void nes_kay_device::prot_sync()
{
	// the protection index decides which banks the latched value goes to
	switch (m_prot_index & 0x3f)
	{
		case 0x20:
		case 0x29:
		case 0x2b:
		case 0x3c:
		case 0x3f:
			m_prot_lock = 1;
			m_ovr[0] = m_prot_latch;
			break;
		case 0x26:
			m_prot_lock = 0;
			m_ovr[0] = m_prot_latch;
			break;
		case 0x2c:
			m_prot_lock = 1;
			if (m_prot_latch)
				m_ovr[0] = m_prot_latch;
			break;
		case 0x28:
			m_prot_lock = 0;
			m_ovr[1] = m_prot_latch;
			break;
		case 0x2a:
			m_prot_lock = 0;
			m_ovr[2] = m_prot_latch;
			break;
		case 0x2f:      // keep the banks previously selected
			break;
		default:        // invalid index: restore normal MMC3 PRG banking
			m_prot_index = 0;
			break;
	}
}


void nes_h2288_device::device_start()
{
	mmc3_start();
	save_item(NAME(m_mmc3_mode));
}

void nes_h2288_device::pcb_reset()
{
	m_mmc3_mode = true;
	mmc3_common_initialize(0x3f, 0xff, 0);
}

void nes_6035052_device::device_start()
{
	mmc3_start();
	save_item(NAME(m_prot));
}

void nes_6035052_device::pcb_reset()
{
	mmc3_common_initialize(0xff, 0xff, 0);
	m_prot = 0;
}

void nes_kof96_device::device_start()
{
	mmc3_start();
	save_item(NAME(m_mmc3_mode));
}

void nes_kof96_device::pcb_reset()
{
	m_mmc3_mode = true;
	mmc3_common_initialize(0xff, 0xff, 0);
}

void nes_cocoma_device::pcb_reset()
{
	mmc3_common_initialize(0x0f, 0x7f, 0);
}

void nes_gouder_device::device_start()
{
	mmc3_start();
	save_item(NAME(m_reg));
	save_item(NAME(m_submapper));
}

void nes_gouder_device::pcb_reset()
{
	std::fill(std::begin(m_reg), std::end(m_reg), 0x00);
	m_reg[5] = 0x11;    // power-on value of the PRG/mirroring register
	mmc3_common_initialize(0xff, 0xff, 0);
	m_mmc_prg_bank[0] = 0x00;   // MMC3 registers R6/R7 power on as $00/$01 (submapper 1 maps R6 >> 2 as its only PRG bank)
	m_mmc_prg_bank[1] = 0x01;
	if (m_submapper)
		set_prg(m_prg_base, m_prg_mask);
	else
	{
		prg32(bitswap<2>(m_reg[5], 4, 0));
		update_mirroring();
	}
}

void nes_sa9602b_device::device_start()
{
	mmc3_start();
	save_item(NAME(m_reg));
	save_item(NAME(m_prg_chip));
}

void nes_sa9602b_device::pcb_reset()
{
	m_reg = 0;
	m_prg_chip = 0;
	mmc3_common_initialize(0x1ff, 0xff, 0);    // 1.5MB of PRG-ROM, no CHR-ROM but 32K CHR-RAM
}

void nes_sachen_shero_device::device_start()
{
	mmc3_start();
	save_item(NAME(m_reg));
}

void nes_sachen_shero_device::pcb_reset()
{
	m_reg = 0;
	mmc3_common_initialize(0xff, 0xff, 0);
}

void nes_sachen_zgdh_device::device_start()
{
	mmc3_start();
	save_item(NAME(m_reg));
}

void nes_sachen_zgdh_device::pcb_reset()
{
	assert(m_vram.size() >= 0x2000);

	m_reg = 0;
	mmc3_common_initialize(0x1f, 0x7f, 0);
}

void nes_a9746_device::device_start()
{
	mmc3_start();
	save_item(NAME(m_reg));
	save_item(NAME(m_ext));
}

void nes_a9746_device::pcb_reset()
{
	m_reg[0] = 0;
	m_reg[1] = 0;
	m_reg[2] = 0;
	m_reg[3] = 3;   // the outer bank register must be $03 for the menu to appear
	m_ext = 0;
	mmc3_common_initialize(0x0f, 0x7f, 0);
	m_mmc_prg_bank[0] = 0x00;   // MMC3 registers R6/R7 power on as $00/$01 (this board never writes them, so $8000-$BFFF would stay on the last banks)
	m_mmc_prg_bank[1] = 0x01;
	update_outer();
}

void nes_a88s1_device::device_start()
{
	mmc3_start();
	save_item(NAME(m_reg));
	save_item(NAME(m_submapper));

	// power-on values for registers; they don't change with soft reset, only the DIP value advances
	m_reg[0] = 0x00;
	m_reg[1] = 0x03;
	m_reg[2] = 0xff;
}

void nes_a88s1_device::pcb_reset()
{
	m_reg[2]++;
	mmc3_common_initialize(0x1f, 0xff, 0);
	update_banks();
}

void nes_bmc_el86xc_device::pcb_reset()
{
	mmc3_common_initialize((m_outer_prg_size >> 3) - 1, 0x7f, 0);
}

void nes_fk23c_device::device_start()
{
	mmc3_start();
	save_item(NAME(m_reg));
	save_item(NAME(m_mmc_cmd1));
}

void nes_fk23c_device::pcb_reset()
{
	m_mmc_cmd1 = 0;
	m_reg[0] = 4;
	m_reg[1] = 0xff;
	m_reg[2] = m_reg[3] = 0;
	m_reg[4] = m_reg[5] = m_reg[6] = m_reg[7] = 0xff;
	mmc3_common_initialize(0xff, 0xff, 0);
	fk23c_set_prg();
	fk23c_set_chr();
}

void nes_fk23ca_device::pcb_reset()
{
	m_mmc_cmd1 = 0;
	m_reg[0] = m_reg[1] = m_reg[2] = m_reg[3] = 0;
	m_reg[4] = m_reg[5] = m_reg[6] = m_reg[7] = 0xff;
	mmc3_common_initialize(0xff, 0xff, 0);
	fk23c_set_prg();
	fk23c_set_chr();
}

void nes_nt639_device::pcb_reset()
{
	mmc3_common_initialize(0x0f, 0xff, 0);
}

// These reset-based PCBs are emulated here with no additional handlers
void nes_resettxrom_device::device_start()
{
	mmc3_start();
	save_item(NAME(m_count));
}

void nes_resettxrom_device::pcb_reset()
{
	assert(std::has_single_bit(m_outer_prg_size));
	assert(std::has_single_bit(m_outer_chr_size));

	mmc3_common_initialize((m_outer_prg_size >> 3) - 1, m_outer_chr_size - 1, 0);

	m_count = (m_count + 1) & 3;
	m_prg_base = m_count << (std::countr_zero(m_outer_prg_size) - 3);
	m_chr_base = m_count << std::countr_zero(m_outer_chr_size);
	set_prg(m_prg_base, m_prg_mask);
	set_chr(m_chr_source, m_chr_base, m_chr_mask);
}

void nes_s24in1sc03_device::device_start()
{
	mmc3_start();
	save_item(NAME(m_reg));
}

void nes_s24in1sc03_device::pcb_reset()
{
	m_reg[0] = 0x24;
	m_reg[1] = 0x9f;
	m_reg[2] = 0;
	mmc3_common_initialize(0xff, 0xff, 0);
}

void nes_tech9in1_device::device_start()
{
	mmc3_start();
	save_item(NAME(m_reg));
}

void nes_tech9in1_device::pcb_reset()
{
	m_reg[0] = m_reg[1] = m_reg[2] = 0;
	mmc3_common_initialize(0x1f, 0xff, 0);
}

void nes_bmc_5in1_device::pcb_reset()
{
	mmc3_common_initialize(0x0f, 0x7f, 0);
	prg32(0);
}

void nes_bmc_8in1_device::pcb_reset()
{
	mmc3_common_initialize(0x0f, 0x7f, 0);
	prg32(0);
}

void nes_bmc_15in1_device::pcb_reset()
{
	mmc3_common_initialize(0x1f, 0xff, 0);
}

void nes_bmc_sbig7_device::pcb_reset()
{
	mmc3_common_initialize(0x0f, 0x7f, 0);
}

void nes_bmc_hik8_device::device_start()
{
	mmc3_start();
	save_item(NAME(m_reg));
	save_item(NAME(m_count));
}

void nes_bmc_hik8_device::pcb_reset()
{
	set_nt_mirroring(PPU_MIRROR_VERT);  // necessary since some boards/games don't reliably set mirroring (Rockman 1 on mc_s13 at least)

	m_count = 0;
	std::fill(std::begin(m_reg), std::end(m_reg), 0x00);
	mmc3_common_initialize(0x3f, 0xff, 0);
}

void nes_bmc_kc885_device::pcb_reset()
{
	nes_bmc_hik8_device::pcb_reset();
	m_prg_mask = 0x1f;
	set_prg(m_prg_base, m_prg_mask);
}

void nes_bmc_hik4_device::device_start()
{
	mmc3_start();
	save_item(NAME(m_mmc3_mode));
}

void nes_bmc_hik4_device::pcb_reset()
{
	m_mmc3_mode = true;
	mmc3_common_initialize(0x0f, 0x7f, 0);
}

void nes_bmc_f15_device::pcb_reset()
{
	mmc3_common_initialize(0x1f, 0xff, 0);
	prg16_89ab(0);
	prg16_cdef(0);
}

void nes_bmc_f600_device::device_start()
{
	mmc3_start();
	save_item(NAME(m_reg));
}

void nes_bmc_f600_device::pcb_reset()
{
	m_reg = 0;
	mmc3_common_initialize(0x1f, 0x7f, 0);
}

void nes_bmc_gn45_device::device_start()
{
	mmc3_start();
	save_item(NAME(m_lock));
}

void nes_bmc_gn45_device::pcb_reset()
{
	m_lock = false;
	mmc3_common_initialize(0x0f, 0x7f, 0);
}

void nes_bmc_gold7in1_device::device_start()
{
	mmc3_start();
	save_item(NAME(m_lock));
}

void nes_bmc_gold7in1_device::pcb_reset()
{
	m_lock = false;
	mmc3_common_initialize(0x1f, 0xff, 0);
}

void nes_bmc_k3006_device::pcb_reset()
{
	mmc3_common_initialize(0x0f, 0x7f, 0);
	prg16_89ab(0);
	prg16_cdef(0);
}

void nes_bmc_k3033_device::device_start()
{
	mmc3_start();
	save_item(NAME(m_mmc3_mode));
}

void nes_bmc_k3033_device::pcb_reset()
{
	m_mmc3_mode = false;
	mmc3_common_initialize(0x0f, 0x7f, 0);
	prg16_89ab(0);
	prg16_cdef(0);
}

void nes_bmc_l6in1_device::device_start()
{
	mmc3_start();
	save_item(NAME(m_reg));
}

void nes_bmc_l6in1_device::pcb_reset()
{
	m_reg = 0;
	mmc3_common_initialize(0x0f, 0x07, 0);
}

void nes_bmc_00202650_device::device_start()
{
	mmc3_start();
	save_item(NAME(m_mmc3_mode));
}

void nes_bmc_00202650_device::pcb_reset()
{
	m_mmc3_mode = false;
	mmc3_common_initialize(0x0f, 0x7f, 0);

	// PCB has an extra 32KB boot menu chip, only the menu uses the CHRRAM
	prg32((m_prg_chunks >> 1) - 1);
	chr8(0, CHRRAM);
}

void nes_bmc_411120c_device::device_start()
{
	mmc3_start();
	save_item(NAME(m_reg));
}

void nes_bmc_411120c_device::pcb_reset()
{
	m_reg = 0;
	mmc3_common_initialize(0x0f, 0x7f, 0);
}

void nes_bmc_810305c_device::device_start()
{
	mmc3_start();
	save_item(NAME(m_outer));
}

void nes_bmc_810305c_device::pcb_reset()
{
	m_outer = 0;
	mmc3_common_initialize(0x1f, 0x7f, 0);
}

void nes_bmc_820720c_device::device_start()
{
	mmc3_start();
	save_item(NAME(m_reg));
}

void nes_bmc_820720c_device::pcb_reset()
{
	m_reg = 0;
	mmc3_common_initialize(0x0f, 0xff, 0);
}

void nes_bmc_830118c_device::pcb_reset()
{
	mmc3_common_initialize(0x0f, 0x7f, 0);
}

void nes_bmc_830832c_device::pcb_reset()
{
	mmc3_common_initialize(0x1f, 0xff, 0);
}

void nes_bmc_yy841101c_device::pcb_reset()
{
	mmc3_common_initialize(0x0f, 0x7f, 0);
}

void nes_bmc_yy841155c_device::device_start()
{
	mmc3_start();
	save_item(NAME(m_reg));
}

void nes_bmc_yy841155c_device::pcb_reset()
{
	m_reg[0] = m_reg[1];
	mmc3_common_initialize(0x0f, 0x7f, 0);
}

void nes_pjoy84_device::device_start()
{
	mmc3_start();
	save_item(NAME(m_reg));
	save_item(NAME(m_submapper));
	save_item(NAME(m_sl0));
}

void nes_pjoy84_device::pcb_reset()
{
	m_sl0++;    // soft resets cycle through the SL0 (menu selection) input values
	std::fill(std::begin(m_reg), std::end(m_reg), 0x00);
	mmc3_common_initialize(0x1f, 0xff, 0);
	m_mmc_prg_bank[0] = 0x00;   // MMC3 registers R6/R7 power on as $00/$01
	m_mmc_prg_bank[1] = 0x01;
	update_banks();
}

void nes_smd133_device::device_start()
{
	mmc3_start();
	save_item(NAME(m_reg));
	save_item(NAME(m_submapper));
}

void nes_smd133_device::pcb_reset()
{
	std::fill(std::begin(m_reg), std::end(m_reg), 0x00);
	mmc3_common_initialize(0xff, 0xff, 0);
	update_banks();
}

void nes_smd133_device::set_smd133_submapper(u8 val)
{
	// submapper bit 0 selects the solder pad: odd = registers at $5000-$5FFF, even = registers overlay WRAM at $6000-$6FFF
	// TODO: FCEUmm decodes the even-submapper registers over the whole of $6000-$7FFF (index = A & 7)
	m_submapper = val;
	m_smd133_addr = BIT(val, 0) ? 0x5000 : 0x6000;
}



/*-------------------------------------------------
 mapper specific handlers
 -------------------------------------------------*/

/*-------------------------------------------------

 Bootleg Board by Nitra

 Games: Queen Bee, Time Diver Avenger

 MMC3 clone with A10 tied to what is normally A0 and
 data instead coming from the lower address lines.

 iNES: mapper 250

 In MAME: Supported.

 -------------------------------------------------*/

void nes_nitra_device::write_h(offs_t offset, u8 data)
{
	LOG("nitra write_h, offset: %04x, data: %02x\n", offset, data);

	txrom_write((offset & 0x6000) | BIT(offset, 10), offset);
}

/*-------------------------------------------------

 Board BMW8544

 Games: Dragon Fighter (Flying Star)

 MMC3 clone with an overridden $8000 PRG bank and custom CHR
 banking (2K XOR bank at PPU $0000, 2K bank at $0800, 4K bank at
 $1000) that is fed by two extra registers. Reading $6000-$6FFF
 (even addresses, PRG-RAM enabled) copies the data of the last CPU
 write to ANY address (including zero page) into one of those
 registers, so the NES driver forwards every CPU write to
 cpu_write_latch().

 NES 2.0: mapper 292 (UNIF UNL-DRAGONFIGHTER)

 In MAME: Supported.

 -------------------------------------------------*/

void nes_bmw8544_device::prg_cb(int start, int bank)
{
	// the $8000 bank is replaced by the board's own register
	prg8_x(start, start == 0 ? (m_reg[0] & 0x1f) : bank);
}

void nes_bmw8544_device::chr_cb(int start, int bank, int source)
{
	if (start == 0)
		chr2_0((bank >> 1) ^ m_reg[1], source);
	else if (start == 2)
		chr2_2((bank >> 1) | ((m_reg[2] & 0x40) << 1), source);
	else if (start == 4)
		chr4_4(m_reg[2] & 0x3f, source);
}

u8 nes_bmw8544_device::read_m(offs_t offset)
{
	LOGMASKED(LOG_HIFREQ, "bmw8544 read_m, offset: %04x\n", offset);

	if (offset < 0x1000 && !BIT(offset, 0))
	{
		if ((m_reg[0] & 0xe0) == 0xc0)
			m_reg[1] = m_bus_latch;
		else
			m_reg[2] = m_bus_latch;
		set_chr(m_chr_source, m_chr_base, m_chr_mask);
	}
	return 0;
}

void nes_bmw8544_device::write_m(offs_t offset, u8 data)
{
	LOG("bmw8544 write_m, offset: %04x, data: %02x\n", offset, data);

	if (offset < 0x1000 && !BIT(offset, 0))
	{
		m_reg[0] = data;
		set_prg(m_prg_base, m_prg_mask);
	}
}

/*-------------------------------------------------

 Board UNL-FS6

 Games: Street Fighter VI / Fight Street VI

 MMC3 clone (identical, but for switched address lines)

 iNES: mapper 196 alt?

 In MAME: Supported.

 -------------------------------------------------*/

void nes_fs6_device::write_h(offs_t offset, u8 data)
{
	LOG("fs6 write_h, offset: %04x, data: %02x\n", offset, data);

	txrom_write(bitswap<2>(offset, 0, 1) | (offset & ~0x03), data);
}

/*-------------------------------------------------

 BTL-SUPERBROS11

 Games: Super Mario Bros. 11, Super Mario Bros. 17

 This acts basically like a MMC3 with different use of write
 address.

 iNES: mapper 196

 In MAME: Supported.

 -------------------------------------------------*/

void nes_sbros11_device::write_h(offs_t offset, u8 data)
{
	LOG("smb11 write_h, offset: %04x, data: %02x\n", offset, data);

	txrom_write((offset & 0x6000) | BIT(offset, 2), data);
}

/*-------------------------------------------------

 UNL-MALISB

 Games: Super Mali Bros Splash Bomb

 This is very similar to mapper 196, but with additional
 data bit swap.

 NES 2.0: mapper 325

 In MAME: Supported.

 -------------------------------------------------*/

void nes_malisb_device::prg_cb(int start, int bank)
{
	bank = bitswap<4>(bank, 2, 3, 1, 0);
	prg8_x(start, bank);
}

void nes_malisb_device::chr_cb(int start, int bank, int source)
{
	bank = bitswap<8>(bank, 7, 6, 1, 4, 3, 2, 5, 0);
	chr1_x(start, bank, source);
}

void nes_malisb_device::write_h(offs_t offset, u8 data)
{
	LOG("malisb write_h, offset: %04x, data: %02x\n", offset, data);

	offset = (offset & 0x6000) | BIT(offset, 3) | (BIT(offset, 14) & BIT(offset, 2));
	txrom_write(offset, data);
}

/*-------------------------------------------------

 BMC-FAMILY-4646B

 Known Boards: Unknown Multigame Bootleg Board (4646B)
 Games: 2 in 1 - Family Kid & Aladdin 4, various multicarts.

 MMC3 clone with banking for multigame menu.

 iNES: mapper 134 (T4A54A / WX-KB4K / BS-5652)

 $6000: mode (CNROM CHR mode, PRG A19, CHR A19, solder pad
 read mode, register lock), $6001: outer banks, $6002: CNROM
 CHR bank, $6003: unknown (only $00 is ever written).
 With $6000 bit 6 set, reads from $8000-$FFFF return the
 solder pad setting in bit 0 (configurable below).

 $6001 bit 2 / bit 6 select 128 KiB outer PRG / CHR banks, in
 which PRG A17 / CHR A17 come from $6001 bit 0 / bit 4; otherwise
 they come from the MMC3. In NROM mode ($6001 bit 7) the MMC3
 sees A13 = A14 = 0, so register 6 (or the fixed bank when PRG A14
 is inverted) applies to the whole range, with A13 from the CPU and
 A14 from the CPU (NROM-256) or the MMC3 ($6001 bit 3, NROM-128).
 In CNROM mode ($6000 bit 3) MMC3 CHR writes do not change the
 8 KiB bank taken from $6002.

 In MAME: Supported.

 TODO: YH-4103 doesn't soft reset cleanly like the other
 carts, but does this happen on hardware?

 -------------------------------------------------*/

void nes_family4646_device::prg_cb(int start, int bank)
{
	if (!BIT(m_reg[1], 7))    // MMC3 mode
		nes_txrom_device::prg_cb(start, bank);
	else if (start == 0)      // NROM mode, only uses MMC3's $8000 bank; PRG A13 and A14 come from the CPU (or A14 from the MMC3)
	{
		if (BIT(m_reg[1], 3))
		{
			prg16_89ab(bank >> 1);
			prg16_cdef(bank >> 1);
		}
		else
			prg32(bank >> 2);
	}
}

void nes_family4646_device::chr_cb(int start, int bank, int source)
{
	if (BIT(m_reg[0], 3))     // CNROM mode: the 8 KiB bank comes from $6002, the MMC3 CHR registers are not used
		bank = m_chr_base | ((m_reg[2] & (m_chr_mask >> 3)) << 3) | start;
	chr1_x(start, bank, source);
}

u8 nes_family4646_device::read_h(offs_t offset)
{
	// $6000 bit 6: CPU $8000-$FFFF reads the solder pad setting instead of PRG-ROM
	if (BIT(m_reg[0], 6))
		return (get_open_bus() & 0xfe) | BIT(m_config.read_safe(0), 0);
	return hi_access_rom(offset);
}

void nes_family4646_device::write_m(offs_t offset, u8 data)
{
	LOG("family4646 write_m, offset: %04x, data: %02x\n", offset, data);

	int reg = offset & 0x03;
	if (!BIT(m_reg[0], 7))    // lock bit
		m_reg[reg] = data;
	else if (reg == 2)        // final two bits of reg 2 respond regardless
		m_reg[reg] = (m_reg[reg] & ~0x03) | (data & 0x03);
	else
		return;

	// outer PRG: A19 = $6000.4, A18 = $6001.1, A17 = $6001.0 only in 128 KiB mode (otherwise A17 comes from the MMC3)
	m_prg_base = ((m_reg[0] & 0x10) << 2) | ((m_reg[1] & 0x02) << 4) | (BIT(m_reg[1], 2) ? ((m_reg[1] & 0x01) << 4) : 0);
	m_prg_mask = 0x1f >> BIT(m_reg[1], 2);
	set_prg(m_prg_base, m_prg_mask);

	// outer CHR: A19 = $6000.5, A18 = $6001.5, A17 = $6001.4 only in 128 KiB mode (otherwise A17 comes from the MMC3)
	m_chr_base = ((m_reg[0] & 0x20) << 4) | ((m_reg[1] & 0x20) << 3) | (BIT(m_reg[1], 6) ? ((m_reg[1] & 0x10) << 3) : 0);
	m_chr_mask = 0xff >> BIT(m_reg[1], 6);
	set_chr(m_chr_source, m_chr_base, m_chr_mask);
}

/*-------------------------------------------------

 BTL-PIKACHUY2K

 Games: Pikachu Y2k

 iNES: mapper 254

 In MAME:

 -------------------------------------------------*/

void nes_pikay2k_device::write_h(offs_t offset, uint8_t data)
{
	LOG("pikay2k write_h, offset: %04x, data: %02x\n", offset, data);

	switch (offset & 0x6001)
	{
		case 0x2001:
			m_reg[1] = data;
			break;

		case 0x2000:
			m_reg[0] = 0;
			[[fallthrough]];
		default:
			txrom_write(offset, data);
			break;
	}
}

// strange WRAM usage: it is protected at start, and gets unprotected after the first write to 0xa000
void nes_pikay2k_device::write_m(offs_t offset, uint8_t data)
{
	LOG("pikay2k write_m, offset: %04x, data: %02x\n", offset, data);

	m_prgram[offset & 0x1fff] = data;
}

uint8_t nes_pikay2k_device::read_m(offs_t offset)
{
	LOG("pikay2k read_m, offset: %04x\n", offset);

	return m_prgram[offset & 0x1fff] ^ (m_reg[0] & m_reg[1]);
}

/*-------------------------------------------------

 Board UNL-8237, UNL-8237A

 Games: Pocahontas 2, MK3, various multicarts

 MMC3 clone with register and address scrambling and
 a few extra banking modes by writing 0x5000-0x5fff.

 NES 2.0: mapper 215, submapper 0 (UNL-8237) and submapper 1 (UNL-8237A)

 In MAME: Preliminary supported.

 TODO: Lion King crashes after the first level in mc_9king.
 Also in mc_9king Lion King, Somari and Aladdin all reset
 to Aladdin instead of the menu.

 -------------------------------------------------*/

void nes_8237_device::update_banks()
{
	int a17 = BIT(m_reg[0], 6);
	m_chr_base = m_board ? (m_reg[1] & 0x0e) << 7 : (m_reg[1] & 0x0c) << 6;
	m_chr_base |= (BIT(m_reg[1], 5) & a17) << 7;
	m_chr_mask = 0xff >> a17;
	set_chr(m_chr_source, m_chr_base, m_chr_mask);

	m_prg_base = m_board ? bitswap<3>(m_reg[1], 3, 1, 0) : m_reg[1] & 0x03;
	m_prg_base = m_prg_base << 5 | (m_reg[1] & (a17 << 4));
	m_prg_mask = 0x1f >> a17;

	if (BIT(m_reg[0], 7))
	{
		u8 bank = m_prg_base >> 1 | (m_reg[0] & (m_prg_mask >> 1));
		u8 mode = BIT(m_reg[0], 5);
		prg16_89ab(bank & ~mode);
		prg16_cdef(bank | mode);
	}
		set_prg(m_prg_base, m_prg_mask);
}

void nes_8237_device::prg_cb(int start, int bank)
{
	if (!BIT(m_reg[0], 7))
		nes_txrom_device::prg_cb(start, bank);
}

void nes_8237_device::write_l(offs_t offset, u8 data)
{
	LOG("unl_8237 write_l, offset: %04x, data: %02x\n", offset, data);
	offset += 0x100;

	switch (offset & 0x1007)
	{
		case 0x1000:
		case 0x1001:
			m_reg[offset & 1] = data;
			update_banks();
			break;
		case 0x1007:
			m_reg[2] = data & 0x07;
			break;
	}
}

void nes_8237_device::write_h(offs_t offset, u8 data)
{
	LOG("unl_8237 write_h, offset: %04x, data: %02x\n", offset, data);

	static constexpr u8 reg_table[8][8] =
	{
		{0, 1, 2, 3, 4, 5, 6, 7},
		{0, 2, 6, 1, 7, 3, 4, 5},
		{0, 5, 4, 1, 7, 2, 6, 3},
		{0, 6, 3, 7, 5, 2, 4, 1},
		{0, 2, 5, 3, 6, 1, 7, 4},
		{0, 1, 2, 3, 4, 5, 6, 7},
		{0, 1, 2, 3, 4, 5, 6, 7},
		{0, 1, 2, 3, 4, 5, 6, 7}
	};

	static constexpr u16 addr_table[8][8] =
	{
		{0x8000, 0x8001, 0xa000, 0xa001, 0xc000, 0xc001, 0xe000, 0xe001},
		{0xa001, 0xa000, 0x8000, 0xc000, 0x8001, 0xc001, 0xe000, 0xe001},
		{0x8000, 0x8001, 0xa000, 0xa001, 0xc000, 0xc001, 0xe000, 0xe001},
		{0xc001, 0x8000, 0x8001, 0xa000, 0xa001, 0xe001, 0xe000, 0xc000},
		{0xa001, 0x8001, 0x8000, 0xc000, 0xa000, 0xc001, 0xe000, 0xe001},
		{0x8000, 0x8001, 0xa000, 0xa001, 0xc000, 0xc001, 0xe000, 0xe001},
		{0x8000, 0x8001, 0xa000, 0xa001, 0xc000, 0xc001, 0xe000, 0xe001},
		{0x8000, 0x8001, 0xa000, 0xa001, 0xc000, 0xc001, 0xe000, 0xe001},
	};

	u16 addr = addr_table[m_reg[2]][bitswap<3>(offset, 14, 13, 0)];
	if (addr == 0x8000)
		data = (data & 0xc0) | reg_table[m_reg[2]][data & 0x07];
	txrom_write(addr & 0x6001, data);
}

/*-------------------------------------------------

 Board UNL-158B

 Games: Blood of Jurassic, Super Hang-On

 MMC3 clone that appears to be the same as the 8237
 boards above with added protection reads at 0x5000.

 NES 2.0: mapper 258

 In MAME: Preliminary supported.

 TODO: Sprites in BoJ can sometimes become a garbled
 mess. Not sure if zapper hit detection is ok either,
 or why it seems you can't shoot at times (reload?).
 Super Hang-On is a hack of Rad Racer/Highway Star so
 it has all the graphics issues those games have.

 -------------------------------------------------*/

u8 nes_158b_device::read_l(offs_t offset)
{
	LOG("unl_158b read_l, offset: %04x\n", offset);

	static constexpr u8 prot_table[8][8] =
	{
		{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
		{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x00},
		{0x00, 0x00, 0x00, 0x00, 0x03, 0x04, 0x00, 0x00},
		{0x00, 0x00, 0x00, 0x01, 0x00, 0x04, 0x05, 0x00},
		{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
		{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
		{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
		{0x00, 0x00, 0x00, 0x01, 0x02, 0x04, 0x0f, 0x00}
	};

	u8 temp = get_open_bus();

	offset += 0x100;
	if (offset >= 0x1000)
		temp = (temp & 0xf0) | prot_table[m_prot][offset & 0x07];

	return temp;
}

void nes_158b_device::write_l(offs_t offset, u8 data)
{
	LOG("unl_158b write_l, offset: %04x, data: %02x\n", offset, data);

	switch ((offset + 0x100) & 0x1003)
	{
		case 0x1002:
			m_prot = data & 0x07;
			break;
		default:
			nes_8237_device::write_l(offset, data);
			break;
	}
}

/*-------------------------------------------------

 Bootleg Board by Kasing

 Games: AV Jiu Ji Mahjong, Bao Qing Tian, Thunderbolt 2,
 Shisen Mahjong 2, Garou Densetsu Special

 MMC3 clone with extra banking modes at 0x6000-0x7fff.

 iNES: mapper 115

 In MAME: Supported.

 -------------------------------------------------*/

void nes_kasing_device::prg_cb(int start, int bank)
{
	if (m_mmc3_mode)
		nes_txrom_device::prg_cb(start, bank);
}

void nes_kasing_device::write_m(offs_t offset, u8 data)
{
	LOG("kasing write_m, offset: %04x, data: %02x\n", offset, data);

	if (BIT(offset, 0))
	{
		m_chr_base = BIT(data, 0) << 8;
		set_chr(m_chr_source, m_chr_base, m_chr_mask);
	}
	else
	{
		m_mmc3_mode = !BIT(data, 7);
		if (m_mmc3_mode)
			set_prg(m_prg_base, m_prg_mask);
		else
		{
			u8 bank = data & 0x0f;
			u8 mode = BIT(data, 5);
			prg16_89ab(bank & ~mode);
			prg16_cdef(bank | mode);
		}
	}
}

/*-------------------------------------------------

 Bootleg Boards by Super Game and Hosenkan

 Games: The Lion King, Aladdin, Boogerman, Pocohon,
 Super Donkey Kong

 MMC3 clone with register and address scrambling and
 a few extra banking modes by writing 0x6000-0x7fff.
 This is the same as mapper 115 with scrambling.

 iNES: mapper 114 (and 182)

 In MAME: Supported.

 -------------------------------------------------*/

void nes_sglionk_device::write_h(offs_t offset, u8 data)
{
	LOG("sglionk write_h, offset: %04x, data: %02x\n", offset, data);

	static constexpr u8 reg_table[2][8] =
	{
		{0, 3, 1, 5, 6, 7, 2, 4},
		{0, 2, 5, 3, 6, 1, 7, 4}
	};

	static constexpr u16 addr_table[2][8] =
	{
		{0xa001, 0xa000, 0x8000, 0xc000, 0x8001, 0xc001, 0xe000, 0xe001},
		{0xa001, 0x8001, 0x8000, 0xc001, 0xa000, 0xc000, 0xe000, 0xe001}
	};

	u16 addr = addr_table[m_board][bitswap<3>(offset, 14, 13, 0)];
	if (addr == 0x8000)
		data = (data & 0xc0) | reg_table[m_board][data & 0x07];
	txrom_write(addr & 0x6001, data);
}

/*-------------------------------------------------

 A9711/A9713 Bootleg Boards by Kay (for Panda Prince)

 Games: The Panda Prince, Sonic 3d Blast 6, SFZ2 '97, YuYu '97
 (and its title hack MK6), UMK3, Super Lion King 2

 MMC3 clone. This is basically KOF96 board + protection

 Writes to $5000-$5FFF select one of four protection values that
 reads of $5000-$5FFF return ($83, $83, $42, $00). $8003 selects a
 protection index and also writes its value to the MMC3 bank select
 register ($8000); $8001 latches the written value with its lower
 six bits reversed, and depending on the index that value replaces
 the $A000/$C000/$E000 banks (see prot_sync()). The A9713 (512 KiB
 PRG and CHR) has an outer bank register at $5180 that powers on
 set. On boards with more CHR than PRG (only Street Fighter Zero 2 '97)
 CHR A18 is set for the pattern table that holds the MMC3's 2 KiB banks,
 as in FCEUmm's 121.c.

 iNES: mapper 121

 In MAME: Supported.

 TODO: Super Lion King 2's title screen is reported as broken.
 NESdev's page instead says CHR A18 follows PPU A12 (the right pattern
 table always uses the second 256 KiB), which did not boot Street Fighter
 Zero 2 '97 here; the two rules only agree while the MMC3 CHR invert bit
 is set.

 -------------------------------------------------*/

void nes_kay_device::write_l(offs_t offset, u8 data)
{
	LOG("kay write_l, offset: %04x, data: %02x\n", offset, data);

	static constexpr u8 prot_table[4] = {0x83, 0x83, 0x42, 0x00};

	offset += 0x100;
	if (offset >= 0x1000)
	{
		m_prot_out = prot_table[data & 0x03];

		if ((offset & 0x1180) == 0x1180)    // A9713 outer bank register
		{
			m_outer = data;
			update_outer();
		}
	}
}

u8 nes_kay_device::read_l(offs_t offset)
{
	LOG("kay read_l, offset: %04x\n", offset);

	offset += 0x100;
	if (offset >= 0x1000)
		return m_prot_out;
	else
		return get_open_bus();
}

void nes_kay_device::chr_cb(int start, int bank, int source)
{
	// boards with more CHR than PRG (the A9713 uses its outer bank register instead): CHR A18 is set for the pattern table
	// that holds the MMC3's 2 KiB banks, i.e. the right one while the CHR invert bit is set and the left one while it is clear
	if (m_prg_size != m_vrom_size && ((start >= 4) == BIT(m_latch, 7)))
		bank |= 0x100;
	chr1_x(start, bank, source);
}

void nes_kay_device::set_prg(int prg_base, int prg_mask)
{
	nes_txrom_device::set_prg(prg_base, prg_mask);

	if (m_prot_index & 0x3f)
	{
		prg8_ab(prg_base | m_ovr[2]);
		prg8_cd(prg_base | m_ovr[1]);
		prg8_ef(prg_base | m_ovr[0]);
	}
}

void nes_kay_device::write_h(offs_t offset, u8 data)
{
	LOG("kay write_h, offset: %04x, data: %02x\n", offset, data);

	if (offset >= 0x2000)       // $A000-$FFFF: plain MMC3 registers
	{
		txrom_write(offset, data);
		return;
	}

	switch (offset & 0x0003)
	{
		case 0x0001:    // protection latch: lower six bits reversed; the MMC3 clone always receives the write as well
			m_prot_latch = bitswap<6>(data, 0, 1, 2, 3, 4, 5);
			if (!m_prot_lock)
				prot_sync();
			txrom_write(offset, data);
			break;

		case 0x0003:    // protection index; the value is also written to the MMC3 bank select register ($8000)
			m_prot_index = data;
			prot_sync();
			txrom_write(offset & ~0x0003, data);
			break;

		case 0x0002:    // 121.c ignores $8002 (NESdev's $E001 mask would make it a second $8000); no known game writes it
			break;

		default:        // $8000: MMC3 bank select
			txrom_write(offset, data);
			break;
	}
	set_prg(m_prg_base, m_prg_mask);
}

/*-------------------------------------------------

 UNL-H2288

 Games: Earthworm Jim 2, Ultimate Mortal Kombat 3

 iNES: mapper 123

 In MAME: Supported.

 -------------------------------------------------*/

void nes_h2288_device::prg_cb(int start, int bank)
{
	if (m_mmc3_mode)
		nes_txrom_device::prg_cb(start, bank);
}

void nes_h2288_device::write_l(offs_t offset, u8 data)
{
	LOG("h2288 write_l, offset: %04x, data: %02x\n", offset, data);

	offset += 0x100;
	if (offset >= 0x1800)
	{
		m_mmc3_mode = !BIT(data, 6);
		if (m_mmc3_mode)
			set_prg(m_prg_base, m_prg_mask);
		else
		{
			u8 bank = bitswap<4>(data, 5, 2, 3, 0);
			u8 mode = BIT(data, 1);
			prg16_89ab(bank & ~mode);
			prg16_cdef(bank | mode);
		}
	}
}

void nes_h2288_device::write_h(offs_t offset, u8 data)
{
	LOG("h2288 write_h, offset: %04x, data: %02x\n", offset, data);

	static constexpr u8 reg_table[8] = {0, 3, 1, 5, 6, 7, 2, 4};

	if (!(offset & 0x6001))
		data = (data & 0xc0) | reg_table[data & 0x07];
	txrom_write(offset, data);
}

/*-------------------------------------------------

 UNL-603-5052

 MMC3 + protection access in 0x4020 - 0x7fff

 in MAME: Partial support

 -------------------------------------------------*/

void nes_6035052_device::write_ex(offs_t offset, uint8_t data)
{
	LOG("6035052 write_ex, offset: %04x, data: %02x\n", offset, data);
	m_prot = data & 0x03;
	if (m_prot == 1)
		m_prot = 2;
}

uint8_t nes_6035052_device::read_ex(offs_t offset)
{
	LOG("6035052 read_ex, offset: %04x\n", offset);
	return m_prot;
}

/*-------------------------------------------------

 Bootleg Board 'Thunder Warrior' by TXC

 Games: Master Fighter II, Master Fighter 3, Thunder Warrior

 MMC3 clone

 iNES: mapper 189

 In MAME: Supported.

 -------------------------------------------------*/

void nes_txc_tw_device::write_l(offs_t offset, uint8_t data)
{
	if (offset < 0x20)   // the register starts at $4120
		return;
	write_m(offset, data);
}

// NESdev INES Mapper 189: [AAAA BBBB] at $4120-$7FFF selects the 32 KiB PRG bank, the two nibbles effectively OR'd
// (a 74175 provides 4 bits); the MMC3's own PRG registers are ignored and there is no WRAM
void nes_txc_tw_device::write_m(offs_t offset, uint8_t data)
{
	LOG("txc_tw write_l, offset: %04x, data: %02x\n", offset, data);

	prg32((data | (data >> 4)) & 0x0f);
}

// writes to 0x8000-0xffff are like MMC3 but no PRG bankswitch (because it is handled by low writes)
void nes_txc_tw_device::prg_cb(int start, int bank)
{
}

/*-------------------------------------------------

 Board UNL-KOF97

 Games: King of Fighters 97 (Rex Soft), Boogerman II

 MMC3 clone

 In MAME: Supported.

 -------------------------------------------------*/

void nes_kof97_device::write_h(offs_t offset, u8 data)
{
	LOG("kof97 write_h, offset: %04x, data: %02x\n", offset, data);

	// unscramble address and data
	offset = (offset & 0x6000) | BIT(offset, 12);
	data = bitswap<8>(data, 7, 6, 2, 4, 3, 0, 5, 1);

	txrom_write(offset, data);
}

/*-------------------------------------------------

 Bootleg Board for KOF96

 Games: The King of Fighters 96, Street Fighter Zero 2

 MMC3 clone

 iNES: mapper 187

 In MAME: Supported.

 -------------------------------------------------*/

void nes_kof96_device::prg_cb(int start, int bank)
{
	if (m_mmc3_mode)
		nes_txrom_device::prg_cb(start, bank);
}

void nes_kof96_device::chr_cb(int start, int bank, int source)
{
	bank |= (m_latch << 1) & (start << 6) & 0x100;
	chr1_x(start, bank, source);
}

void nes_kof96_device::write_l(offs_t offset, u8 data)
{
	LOG("kof96 write_l, offset: %04x, data: %02x\n", offset, data);

	offset += 0x100;
	if ((offset & 0x1001) == 0x1000)
	{
		m_mmc3_mode = !BIT(data, 7);
		if (m_mmc3_mode)
			set_prg(m_prg_base, m_prg_mask);
		else
		{
			u8 bank = BIT(data, 1, 4);
			u8 mode = BIT(data, 5);
			prg16_89ab(bank & ~mode);
			prg16_cdef(bank | mode);
		}
	}
}

u8 nes_kof96_device::read_l(offs_t offset)
{
	LOG("kof96 read_l, offset: %04x\n", offset);

	offset += 0x100;
	if (offset >= 0x1000)
		return 0x80;  // unknown protection read, kof96 expects that MSB is set
	else
		return 0;
}

/*-------------------------------------------------

 Bootleg Board for Super Fighter III

 MMC3 clone

 iNES: mapper 197

 In MAME: Supported.

 -------------------------------------------------*/

void nes_sf3_device::set_chr(uint8_t chr_source, int chr_base, int chr_mask)
{
	chr4_0(chr_base | ((m_mmc_vrom_bank[0] >> 1) & chr_mask), chr_source);
	chr2_4(chr_base | (m_mmc_vrom_bank[1] & chr_mask), chr_source);
	chr2_6(chr_base | (m_mmc_vrom_bank[2] & chr_mask), chr_source);
}

void nes_sf3_device::write_h(offs_t offset, uint8_t data)
{
	uint8_t cmd;
	LOG("sf3 write_h, offset: %04x, data: %02x\n", offset, data);

	switch (offset & 0x6001)
	{
		case 0x0001:
			cmd = m_latch & 0x07;
			switch (cmd)
			{
				case 0: case 2: case 4:
					m_mmc_vrom_bank[cmd >> 1] = data;
					set_chr(m_chr_source, m_chr_base, m_chr_mask);
					break;
				case 6:
				case 7:
					m_mmc_prg_bank[cmd - 6] = data;
					set_prg(m_prg_base, m_prg_mask);
					break;
			}
			break;

		default:
			txrom_write(offset, data);
			break;
	}
}

/*-------------------------------------------------

 Cocoma Core Pro Board

 Games: BrilliantCom Cocoma Pack 1 and 2

 MMC3 clone with an extra outer bank latch.

 NES 2.0: mapper 516

 In MAME: Supported.

 -------------------------------------------------*/

void nes_cocoma_device::write_h(offs_t offset, u8 data)
{
	LOG("cocoma write_h, offset: %04x, data: %02x\n", offset, data);

	if (BIT(offset, 4))
	{
		m_prg_base = (offset & 0x03) << 4;
		set_prg(m_prg_base, m_prg_mask);
		m_chr_base = (offset & 0x0c) << 5;
		set_chr(m_chr_source, m_chr_base, m_chr_mask);
	}
	else
		txrom_write(offset, data);
}

/*-------------------------------------------------

 Bootleg Board 37017 (?) by Gouder

 Games: Street Fighter IV

 MMC3 clone. It also requires reads from 0x5000-0x7fff.

 NES 2.0: mapper 208, submapper 0 (Gouder: $4800-$4FFF/$6800-$6FFF
 select a 32 KiB PRG bank and the mirroring, $5000-$5FFF is the XOR
 protection) and submapper 1 (Mortal Kombat JJ-01 without the Gouder
 copyright: no extra registers, 32 KiB PRG bank is MMC3 register 6 / 4).

 In MAME: Supported.

 -------------------------------------------------*/

void nes_gouder_device::write_l(offs_t offset, u8 data)
{
	static constexpr u8 conv_table[256] =
	{
		0x59,0x59,0x59,0x59,0x59,0x59,0x59,0x59,0x59,0x49,0x19,0x09,0x59,0x49,0x19,0x09,
		0x59,0x59,0x59,0x59,0x59,0x59,0x59,0x59,0x51,0x41,0x11,0x01,0x51,0x41,0x11,0x01,
		0x59,0x59,0x59,0x59,0x59,0x59,0x59,0x59,0x59,0x49,0x19,0x09,0x59,0x49,0x19,0x09,
		0x59,0x59,0x59,0x59,0x59,0x59,0x59,0x59,0x51,0x41,0x11,0x01,0x51,0x41,0x11,0x01,
		0x00,0x10,0x40,0x50,0x00,0x10,0x40,0x50,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
		0x08,0x18,0x48,0x58,0x08,0x18,0x48,0x58,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
		0x00,0x10,0x40,0x50,0x00,0x10,0x40,0x50,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
		0x08,0x18,0x48,0x58,0x08,0x18,0x48,0x58,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
		0x59,0x59,0x59,0x59,0x59,0x59,0x59,0x59,0x58,0x48,0x18,0x08,0x58,0x48,0x18,0x08,
		0x59,0x59,0x59,0x59,0x59,0x59,0x59,0x59,0x50,0x40,0x10,0x00,0x50,0x40,0x10,0x00,
		0x59,0x59,0x59,0x59,0x59,0x59,0x59,0x59,0x58,0x48,0x18,0x08,0x58,0x48,0x18,0x08,
		0x59,0x59,0x59,0x59,0x59,0x59,0x59,0x59,0x50,0x40,0x10,0x00,0x50,0x40,0x10,0x00,
		0x01,0x11,0x41,0x51,0x01,0x11,0x41,0x51,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
		0x09,0x19,0x49,0x59,0x09,0x19,0x49,0x59,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
		0x01,0x11,0x41,0x51,0x01,0x11,0x41,0x51,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
		0x09,0x19,0x49,0x59,0x09,0x19,0x49,0x59,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00
	};

	LOG("gouder write_l, offset: %04x, data: %02x\n", offset, data);

	if (m_submapper)    // submapper 1 has no registers here
		return;

	offset += 0x100;
	switch (offset & 0x1800)
	{
		case 0x0800:
			m_reg[5] = data;
			prg32(bitswap<2>(m_reg[5], 4, 0));
			update_mirroring();
			break;
		case 0x1000:
			m_reg[4] = data;
			break;
		case 0x1800:
			m_reg[offset & 0x03] = data ^ conv_table[m_reg[4]];
			break;
	}
}

u8 nes_gouder_device::read_l(offs_t offset)
{
	LOG("gouder read_l, offset: %04x\n", offset);

	offset += 0x100;
	if (!m_submapper && offset >= 0x1800)
		return m_reg[offset & 0x03];

	return get_open_bus();
}

void nes_gouder_device::update_mirroring()
{
	// submapper 0: the MMC3 mirroring register is ignored, bit 5 of the PRG register selects mirroring
	set_nt_mirroring(BIT(m_reg[5], 5) ? PPU_MIRROR_HORZ : PPU_MIRROR_VERT);
}

void nes_gouder_device::write_m(offs_t offset, u8 data)
{
	LOG("gouder write_m, offset: %04x, data: %02x\n", offset, data);

	// $6800-$6FFF mirrors the $4800-$4FFF PRG/mirroring register
	if (!m_submapper && (offset & 0x1800) == 0x0800)
	{
		m_reg[5] = data;
		prg32(bitswap<2>(m_reg[5], 4, 0));
		update_mirroring();
	}
}

void nes_gouder_device::write_h(offs_t offset, u8 data)
{
	txrom_write(offset, data);
	if (!m_submapper)
		update_mirroring();
}

// submapper 0: writes to 0x8000-0xffff are like MMC3 but no PRG bankswitch (it is handled by the PRG register)
// submapper 1: one 32 KiB PRG bank at $8000 selected by MMC3 register 6 / 4
void nes_gouder_device::prg_cb(int start, int bank)
{
	if (m_submapper && start == 0)
		prg32(m_mmc_prg_bank[0] >> 2);
}


/*-------------------------------------------------

 UNL-SA-9602B

 Sachen boards used for a chinese port of Princess Maker (?)

 in MAME: Very Preliminary support, based on Cah4e3
 code in FCEUMM

 -------------------------------------------------*/

void nes_sa9602b_device::prg_cb(int start, int bank)
{
	if (m_prg_chip == 3)
	{
		printf("Accessing PRG beyond end of ROM!\n");
		m_prg_chip = 2; // assume that the higher PRG chip is mirrored twice
	}
	prg8_x(start, (m_prg_chip * 0x40) | (bank & 0x3f));

	if (m_latch & 0x40)
		prg8_89(0x3e);
	else
		prg8_cd(0x3e);

	prg8_ef(0x3f);
//  printf("start %d, bank %d\n", start, bank);
}


void nes_sa9602b_device::write_h(offs_t offset, uint8_t data)
{
	LOG("sa9602b write_h, offset: %04x, data: %02x\n", offset, data);

	switch (offset & 0x6001)
	{
		case 0x0000:
			m_reg = data;
			break;
		case 0x0001:
			if ((m_reg & 7) < 6)
				m_prg_chip = BIT(data, 6, 2);
			set_prg(0, m_prg_mask);
			break;
	}

	txrom_write(offset, data);
}

/*-------------------------------------------------

 UNL-SHERO

 Sachen boards used for Street Heroes

 NES 2.0: mapper 262

 In MAME: Supported.

 -------------------------------------------------*/

void nes_sachen_shero_device::chr_cb(int start, int bank, int source)
{
	static constexpr u8 shift[4] = {5, 6, 8, 7};

	if (!BIT(m_reg, 6))
		chr1_x(start, ((m_reg << shift[start >> 1]) & 0x100) | bank, source);
}

void nes_sachen_shero_device::write_l(offs_t offset, u8 data)
{
	LOG("shero write_l, offset: %04x, data: %02x\n", offset, data);
	offset += 0x4100;

	if (offset == 0x4100)
	{
		m_reg = data;
		if (BIT(m_reg, 6))
			chr8(0, CHRRAM);
		else
			set_chr(m_chr_source, m_chr_base, m_chr_mask);
	}
}

u8 nes_sachen_shero_device::read_l(offs_t offset)
{
	LOG("shero read_l, offset: %04x\n", offset);
	offset += 0x4100;

	if (offset == 0x4100)
		return m_jumper->read();
	else
		return get_open_bus();
}

/*-------------------------------------------------

 SACHEN-ZGDH

 Sachen board used for Zhongguo Daheng

 NES 2.0: mapper 512

 In MAME: Supported.

 -------------------------------------------------*/

u8 nes_sachen_zgdh_device::nt_r(offs_t offset)
{
	if (m_reg == 1)
		return m_vram[0x1000 + (offset & 0x0fff)];
	else
		return device_nes_cart_interface::nt_r(offset);
}

void nes_sachen_zgdh_device::nt_w(offs_t offset, u8 data)
{
	if (m_reg == 1)
		m_vram[0x1000 + (offset & 0x0fff)] = data;
	else
		device_nes_cart_interface::nt_w(offset, data);
}

void nes_sachen_zgdh_device::set_chr(u8 chr, int chr_base, int chr_mask)
{
	if (m_reg <= 1)
	{
		chr = CHRROM;
		m_chr_mask = 0x7f;
	}
	else
	{
		chr = CHRRAM;
		m_chr_mask = 0x03;
	}
	nes_txrom_device::set_chr(chr, chr_base, m_chr_mask);
}

void nes_sachen_zgdh_device::write_l(offs_t offset, u8 data)
{
	LOG("zgdh write_l, offset: %04x, data: %02x\n", offset, data);

	offset += 0x100;
	if ((offset & 0x1100) == 0x0100)
	{
		m_reg = data & 0x03;
		set_chr(m_chr_source, m_chr_base, m_chr_mask);
	}
}

/*-------------------------------------------------

 UNL-A9746

 Games: Toy Story, Super 1997 4 in 1 (NT-8029)

 MMC3 clone

 NES 2.0: mapper 219 (Kasheng A9461)

 MMC3 mode (banks masked to 128 KiB) and an extended mode with
 reversed-bit PRG bank registers $23-$26 and CHR latch registers,
 plus a two-bit 128 KiB outer PRG/CHR bank written at $5002/$5003
 (power-on value $03, which makes the menu appear).

 $8002 bit 5 selects the mode: with it clear $8000/$8001 are the
 normal MMC3 bank select/data registers, with it set a write to
 $8000 or $8002 selects one of the extended PRG ($23-$26) or CHR
 ($08-$1F) registers that $8001 then writes. $A000-$FFFF are always
 the regular MMC3 registers.

 NOTE: FCEUX (a9746.c) and Nestopia (NstBoardUnlA9746) never pass
 $8000/$8001 on to the MMC3 and drop $A000/$A001; the mode bit and
 the MMC3 passthrough follow NESdev. If Toy Story or the 4-in-1
 misbehave, remove the txrom_write() calls for $8000-$8002 below.

 -------------------------------------------------*/

void nes_a9746_device::update_outer()
{
	m_prg_base = m_reg[3] << 4;
	m_chr_base = m_reg[3] << 7;
	set_prg(m_prg_base, m_prg_mask);
	set_chr(m_chr_source, m_chr_base, m_chr_mask);
}

void nes_a9746_device::prg_cb(int start, int bank)
{
	// MMC3 mode banks are masked to 128 KiB, the outer register selects the 128 KiB block
	prg8_x(start, m_prg_base | (bank & 0x0f));
}

void nes_a9746_device::chr_cb(int start, int bank, int source)
{
	chr1_x(start, m_chr_base | (bank & 0x7f), source);
}

void nes_a9746_device::write_l(offs_t offset, u8 data)
{
	LOG("unl_a9746 write_l, offset: %04x, data: %02x\n", offset, data);

	offset += 0x100;
	switch (offset & 0x1003)
	{
		case 0x1002:    // $5002: outer bank bit 0
			m_reg[3] = (m_reg[3] & 0x02) | BIT(data, 0);
			update_outer();
			break;
		case 0x1003:    // $5003: outer bank bit 1
			m_reg[3] = (m_reg[3] & 0x01) | (BIT(data, 5) << 1);
			update_outer();
			break;
	}
}

void nes_a9746_device::update_banks(uint8_t value)
{
	// extended PRG registers: index $23-$26 ($2B-$2E too, bit 3 is not decoded) selects the 8 KiB bank at $E000/$C000/$A000/$8000;
	// the bank number is made of bits 5-2 of the data, in reversed order
	const u8 sel = m_reg[0];
	const u8 prg_sel = sel;
	if ((prg_sel & 0x30) == 0x20 && (prg_sel & 0x07) >= 3 && (prg_sel & 0x07) <= 6)
		prg8_x(6 - (prg_sel & 0x07), m_prg_base | bitswap<4>(value, 2, 3, 4, 5));

	// extended CHR registers
	const u8 chr_sel = sel;
	if (!BIT(chr_sel, 0) && ((chr_sel & 0x38) == 0x08 || (chr_sel & 0x30) == 0x10))
		m_reg[2] = (value & 0x07) << 4;                                                 // bank number latch: CHR A16-A14 of the next bank
	else if ((chr_sel & 0x39) == 0x09)                                                  // 1 KiB banks at $0000-$0FFF, pairs share bank bit 0
		chr1_x(BIT(chr_sel, 1, 2), m_chr_base | m_reg[2] | (value >> 1) | BIT(chr_sel, 1), m_chr_source);
	else if ((chr_sel & 0x31) == 0x11)                                                  // 1 KiB banks at $1000-$1FFF
		chr1_x(4 + BIT(chr_sel, 2, 2), m_chr_base | m_reg[2] | (value >> 1), m_chr_source);
}

void nes_a9746_device::write_h(offs_t offset, uint8_t data)
{
	LOG("unl_a9746 write_h, offset: %04x, data: %02x\n", offset, data);

	switch (offset & 0x6003)
	{
		case 0x0000:    // MMC3 bank select, or index of the extended register
			m_reg[0] = data;
			if (!m_ext)
				txrom_write(offset, data);
			break;

		case 0x0001:    // MMC3 bank data, or data of the selected extended register
			if (m_ext)
				update_banks(data);
			else
				txrom_write(offset, data);
			break;

		case 0x0002:    // mode register: bit 5 selects extended mode, otherwise the same as the MMC3 bank select
			m_ext = BIT(data, 5);
			m_reg[0] = data;
			if (!m_ext)
				txrom_write(offset & ~0x0002, data);
			break;

		case 0x0003:
			break;

		default:        // $A000-$FFFF are regular MMC3 registers
			txrom_write(offset, data);
			break;
	}
}



/*-------------------------------------------------

 MULTIGAME CARTS BASED ON MMC3

 -------------------------------------------------*/

/*-------------------------------------------------

 Board BMC-FK23C / BMC-FK23CA

 MMC3 clone

 8025 enhanced MMC3 (FK-xxx/BS-xxx, NES 2.0 mapper 176 submapper 1).
 NES 2.0 images are emulated by nes_8025_device, which follows the
 NESdev register description; this older board is only used for UNIF
 files and software list entries.

 In MAME: partially supported (still to sort initial banking
 for many games)

 TODO: the extended MMC3 registers $8-$B are only partly
 emulated, and the solder pad address decode ($5013-$5803)
 is fixed.

 -------------------------------------------------*/

#if 0
// alt version...
void nes_fk23c_device::prg_cb(int start, int bank)
{
	if (((m_reg[0] & 0x07) - 3) > 1 && (!(m_reg[3] & 0x02) || start < 2))
	{
		if (m_reg[0] & 0x03)
			bank = (bank & (0x3f >> (m_reg[0] & 0x03))) | (m_reg[1] << 1);

		prg8_x(start, bank);
	}
}

void nes_fk23c_device::chr_cb(int start, int bank, int source)
{
	if (!(m_reg[0] & 0x40) && (!(m_reg[3] & 0x02) || (start != 1 && start != 3)))
		chr1_x(start, ((m_reg[2] & 0x7f) << 3) | bank, source);
}

#endif

void nes_fk23c_device::prg_cb(int start, int bank)
{
	uint8_t mask = (0x3f >> (m_reg[0] & 0x03));

	if ((m_reg[0] & 0x07) < 3)
	{
		if (!(m_reg[0] & 0x03))
			bank = (bank & mask) | ((m_reg[1]  & (0x7f ^ mask)) << 1);

		prg8_x(start, bank);
	}
}

void nes_fk23c_device::chr_cb(int start, int bank, int source)
{
	if (!(m_reg[0] & 0x40) && (!(m_reg[3] & 0x02) || (start != 1 && start != 3)))
		chr1_x(start, ((m_reg[2] & 0x7f) << 3) | bank, source);
}

void nes_fk23c_device::fk23c_set_prg()
{
	if ((m_reg[0] & 0x07) == 4)
		prg32(BIT(m_reg[1], 1, 6));
	else if ((m_reg[0] & 0x07) == 3)
	{
		prg16_89ab(m_reg[1] & 0x7f);
		prg16_cdef(m_reg[1] & 0x7f);
	}
	else
	{
		if (m_reg[3] & 0x02)
		{
			prg8_cd(m_reg[4]);
			prg8_ef(m_reg[5]);
		}
		else
			set_prg(m_prg_base, m_prg_mask);
	}
}

void nes_fk23c_device::fk23c_set_chr()
{
	if (m_reg[0] & 0x40)
		chr8(m_reg[2] | m_mmc_cmd1, m_chr_source);
	else
	{
		if (m_reg[3] & 0x02)
		{
			int base = (m_reg[2] & 0x7f) << 3;
			chr1_x(1, base | m_reg[6], m_chr_source);
			chr1_x(3, base | m_reg[7], m_chr_source);
		}
		else
			set_chr(m_chr_source, m_chr_base, m_chr_mask);
	}
}

void nes_fk23c_device::write_l(offs_t offset, uint8_t data)
{
	LOG("fk23c write_l, offset: %04x, data: %02x\n", offset, data);
	offset += 0x100;

	if (offset >= 0x1000)
	{
		if (offset & (1 << 4))  // here it should be (4 + m_mmc_dipsetting)
		{
			m_reg[offset & 0x03] = data;

			fk23c_set_prg();
			fk23c_set_chr();
		}
	}
}

void nes_fk23c_device::write_h(offs_t offset, uint8_t data)
{
	LOG("fk23c write_h, offset: %04x, data: %02x\n", offset, data);

	if (m_reg[0] & 0x40)
	{
		if (m_reg[0] & 0x30)
			m_mmc_cmd1 = 0;
		else
		{
			m_mmc_cmd1 = data & 0x03;
			fk23c_set_chr();
		}
	}
	else
	{
		switch (offset & 0x6001)
		{
			case 0x0001:
				if ((m_reg[3] & 0x02) && (m_latch & 0x08))
				{
					m_reg[4 | (m_latch & 0x03)] = data;
					fk23c_set_prg();
					fk23c_set_chr();
				}
				else
					txrom_write(offset, data);
				break;

			case 0x2000:
				set_nt_mirroring(data ? PPU_MIRROR_HORZ : PPU_MIRROR_VERT);
				break;

			default:
				txrom_write(offset, data);
				break;
		}
	}
}

/*-------------------------------------------------

 BMC-NT-639

 Unknown Bootleg Multigame Board
 Games: 2 in 1 Mortal Kombat VI/VII

 NES 2.0: mapper 291

 In MAME: Supported.

 -------------------------------------------------*/

void nes_nt639_device::write_m(offs_t offset, u8 data)
{
	LOG("nt639 write_m, offset: %04x, data: %02x\n", offset, data);

	if (BIT(data, 5))
		prg32(bitswap<3>(data, 6, 2, 1));
	else
	{
		m_prg_base = (data & 0x40) >> 2;
		set_prg(m_prg_base, m_prg_mask);
	}

	m_chr_base = (data & 0x40) << 2;
	set_chr(m_chr_source, m_chr_base, m_chr_mask);
}

/*-------------------------------------------------

 Board BMC-SUPER24IN1SC03

 Games: Super 24-in-1

 NES 2.0 mapper 176 submapper 0 images (LP-8002KB, SFC-12B) are
 emulated by nes_8025_device; this older board is only used for UNIF
 files and software list entries.

 In MAME: Partially Supported

 -------------------------------------------------*/

void nes_s24in1sc03_device::prg_cb(int start, int bank)
{
	static const uint8_t masks[8] = {0x3f, 0x1f, 0x0f, 0x01, 0x03, 0x00, 0x00, 0x00};
	int prg_base = m_reg[1] << 1;
	int prg_mask = masks[m_reg[0] & 0x07];

	bank = prg_base | (bank & prg_mask);
	prg8_x(start, bank);
}

void nes_s24in1sc03_device::chr_cb(int start, int bank, int source)
{
	uint8_t chr = BIT(m_reg[0], 5) ? CHRRAM : CHRROM;
	int chr_base = (m_reg[2] << 3) & 0xf00;

	chr1_x(start, chr_base | bank, chr);
}

void nes_s24in1sc03_device::write_l(offs_t offset, uint8_t data)
{
	LOG("s24in1sc03 write_l, offset: %04x, data: %02x\n", offset, data);
	offset += 0x100;

	if (offset == 0x1ff0)
	{
		m_reg[0] = data;
		set_chr(m_chr_source, m_chr_base, m_chr_mask);
		set_prg(m_prg_base, m_prg_mask);
	}

	if (offset == 0x1ff1)
	{
		m_reg[1] = data;
		set_prg(m_prg_base, m_prg_mask);
	}

	if (offset == 0x1ff2)
	{
		m_reg[2] = data;
		set_chr(m_chr_source, m_chr_base, m_chr_mask);
	}
}

/*-------------------------------------------------

 BMC-TECHLINE9IN1

 Unknown Bootleg Multigame Board
 Games: 9 in 1

 NES 2.0: mapper 351

 In MAME: Supported.

 -------------------------------------------------*/

void nes_tech9in1_device::update_banks()
{
	if (BIT(m_reg[2], 7))    // NROM mode
	{
		u8 bank = m_reg[1] >> 2;
		u8 mode = !BIT(m_reg[2], 6);
		prg16_89ab(bank & ~mode);
		prg16_cdef(bank | mode);
	}
	else                     // MMC3 mode
	{
		m_prg_base = (m_reg[1] & 0xfc) >> 1;
		m_prg_mask = 0x1f >> BIT(m_reg[2], 5);
		set_prg(m_prg_base, m_prg_mask);
	}

	m_chr_base = (m_reg[0] & 0xfc) << 1;
	m_chr_mask = BIT(m_reg[2], 7) ? 0x1f : 0xff >> BIT(m_reg[2], 5);
	set_chr(m_chr_source, m_chr_base, m_chr_mask);
}

void nes_tech9in1_device::write_l(offs_t offset, u8 data)
{
	LOG("tech9in1 write_l, offset: %04x, data: %02x\n", offset, data);
	offset += 0x100;

	switch (offset & 0x1003)    // writes $5000-$5002, mask is a best guess
	{
		case 0x1000: case 0x1001: case 0x1002:
			m_reg[offset & 3] = data;
			update_banks();
			break;
	}
}

/*-------------------------------------------------

 BMC-5IN1

 Unknown Bootleg Multigame Board
 Games: 5 in 1 1993 Copyright

 NES 2.0: mapper 334

 In MAME: Supported.

 -------------------------------------------------*/

u8 nes_bmc_5in1_device::read_m(offs_t offset)
{
	LOG("bmc_5in1 read_m, offset: %04x\n", offset);

	if ((offset & 0x03) == 0x02)
		return (get_open_bus() & 0xfe) | m_jumper->read();
	else
		return get_open_bus();
}

void nes_bmc_5in1_device::write_m(offs_t offset, u8 data)
{
	LOG("bmc_5in1 write_m, offset: %04x, data: %02x\n", offset, data);

	if ((m_wram_protect & 0xc0) == 0x80)
	{
		if (!(offset & 0x03))
			prg32(BIT(data, 1, 2));
	}
}

/*-------------------------------------------------

 BMC-NEWSTAR-GRM070-8IN1

 Unknown Bootleg Multigame Board
 Games: New Star Super 8 in 1

 NES 2.0: mapper 333

 In MAME: Supported.

 -------------------------------------------------*/

void nes_bmc_8in1_device::write_h(offs_t offset, u8 data)
{
	LOG("bmc_8in1 write_h, offset: %04x, data: %02x\n", offset, data);

	if (BIT(offset, 12))
	{
		if (BIT(data, 4))
		{
			m_prg_base = (data & 0x0c) << 2;
			set_prg(m_prg_base, m_prg_mask);
			m_chr_base = m_prg_base << 3;
			set_chr(m_chr_source, m_chr_base, m_chr_mask);
		}
		else
			prg32(data & 0x0f);
	}
	else
		txrom_write(offset, data);
}

/*-------------------------------------------------

 BMC-15IN1 (PCB JC-016-2?)

 Games: 3 in 1, 15 in 1

 MMC3 clone with banking for multigame menu.

 iNES: mapper 205

 In MAME: Supported.

 -------------------------------------------------*/

void nes_bmc_15in1_device::write_m(offs_t offset, u8 data)
{
	LOG("bmc_15in1 write_m, offset: %04x, data: %02x\n", offset, data);

	if (data & 1)
		data |= m_jumper;    // TODO: add jumper settings, m_jumper is 0 for now

	m_prg_base = (data & 0x03) << 4;
	m_prg_mask = 0x1f >> BIT(data, 1);
	set_prg(m_prg_base, m_prg_mask);

	m_chr_base = m_prg_base << 3;
	m_chr_mask = 0xff >> BIT(data, 1);
	set_chr(m_chr_source, m_chr_base, m_chr_mask);
}

/*-------------------------------------------------

 BMC-SUPERBIG-7IN1

 Known Boards: Unknown Multigame Bootleg Board
 Games: Kunio 8 in 1, Super Big 7 in 1

 iNES: mapper 44

 In MAME: Supported. It also uses mmc3_irq.

 -------------------------------------------------*/

void nes_bmc_sbig7_device::write_h(offs_t offset, uint8_t data)
{
	uint8_t page;
	LOG("bmc_sbig7 write_h, offset: %04x, data: %02x\n", offset, data);

	switch (offset & 0x6001)
	{
		case 0x2001: // $a001 - Select 128K ROM/VROM base (0..5) or last 256K ROM/VRAM base (6)
			page = (data & 0x07);
			if (page > 6)
				page = 6;

			m_prg_base = page << 4;
			m_prg_mask = (page > 5) ? 0x1f : 0x0f;
			m_chr_base = page << 7;
			m_chr_mask = (page > 5) ? 0xff : 0x7f;
			set_prg(m_prg_base, m_prg_mask);
			set_chr(m_chr_source, m_chr_base, m_chr_mask);
			break;

		default:
			txrom_write(offset, data);
			break;
	}
}

/*-------------------------------------------------

 BMC-HIK8IN1

 Known Boards: Unknown Multigame Bootleg Board
 Games: Many, many multigame carts

 iNES: mapper 45

 In MAME: Supported. It also uses mmc3_irq.

 -------------------------------------------------*/

void nes_bmc_hik8_device::write_m(offs_t offset, u8 data)
{
	LOG("bmc_hik8 write_m, offset: %04x, data: %02x\n", offset, data);

	nes_txrom_device::write_m(offset, data);  // registers overlay WRAM

	if (!BIT(m_reg[3], 6))  // outer register lock bit
	{
		m_reg[m_count] = data;
		m_count = (m_count + 1) & 0x03;

		m_prg_base = (m_reg[2] & 0xc0) << 2 | m_reg[1];
		m_prg_mask = ~m_reg[3] & 0x3f;
		set_prg(m_prg_base, m_prg_mask);

		m_chr_base = (m_reg[2] & 0xf0) << 4 | m_reg[0];
		m_chr_mask = 0xff >> (~m_reg[2] & 0x0f);
		set_chr(m_chr_source, m_chr_base, m_chr_mask);
	}
}

/*-------------------------------------------------

 BMC-JY-208

 Games: Rockman 7 in 1

 Variant of the HIK8IN1 boards that adds 8K of VRAM
 to be used presumably by Rockman 4/6 and a 4 screen
 mirroring mode for Sachen's Rocman X.

 NES 2.0: mapper 356

 In MAME: Supported.

 -------------------------------------------------*/

void nes_bmc_jy208_device::set_chr(u8 chr, int chr_base, int chr_mask)
{
	if (BIT(m_reg[2], 5))
		nes_txrom_device::set_chr(chr, chr_base, chr_mask);
	else
		chr8(0, CHRRAM);
}

void nes_bmc_jy208_device::write_m(offs_t offset, u8 data)
{
	nes_bmc_hik8_device::write_m(offset, data);

	if (BIT(m_reg[2], 6))
		m_mirroring = PPU_MIRROR_4SCREEN;  // prevent MMC3 mirror switching
	else
		m_mirroring = BIT(m_mmc_mirror, 0) ? PPU_MIRROR_HORZ : PPU_MIRROR_VERT;  // allow MMC3 mirror switching and get its current setting

	set_nt_mirroring(m_mirroring);  // actually change nametable pointers
}

/*-------------------------------------------------

 BMC-JY-302

 Games: Super 8 in 1

 Variant of the HIK8IN1 boards that adds 8K of VRAM
 which seems the same as the SFC-12 board below,
 other than the line which selects VRAM/VROM.

 NES 2.0: mapper 410

 In MAME: Supported.

 -------------------------------------------------*/

void nes_bmc_jy302_device::set_chr(u8 chr, int chr_base, int chr_mask)
{
	if (BIT(m_reg[2], 6))
		chr8(0, CHRRAM);
	else
		nes_txrom_device::set_chr(chr, chr_base, chr_mask);
}

/*-------------------------------------------------

 BMC-KC885

 Games: Super 8 in 1 VIP19

 Variant of the HIK8IN1 boards that has different
 upper address lines for PRG banking. Exactly how
 these lines are selected is determined by three
 jumper/solder pads.

 NES 2.0: mapper 401

 In MAME: Supported.

 -------------------------------------------------*/

u8 nes_bmc_kc885_device::read_h(offs_t offset)
{
	LOGMASKED(LOG_HIFREQ, "bmc_kc885 read_h, offset: %04x\n", offset);

	if (BIT(m_reg[1], 7) & BIT(m_jumper->read(), 2))
		return get_open_bus();
	else
		return hi_access_rom(offset);
}

void nes_bmc_kc885_device::write_m(offs_t offset, u8 data)
{
	LOG("bmc_kc885 write_m, offset: %04x, data: %02x\n", offset, data);

	if (!BIT(m_reg[3], 6))  // outer register lock bit
	{
		if (m_count == 1)
			data = (data & ~0x60) | bitswap<2>(data, 5, 6) << 5;
		m_reg[m_count] = data;
		m_count = (m_count + 1) & 0x03;

		u8 mask = 0x80 | (m_jumper->read() & 0x03) << 5;
		m_prg_base = (m_reg[2] & mask) | (m_reg[1] & ~mask);
		m_prg_mask = ~m_reg[3] & 0x1f;
		set_prg(m_prg_base, m_prg_mask);

		m_chr_base = (m_reg[2] & 0xf0) << 4 | m_reg[0];
		m_chr_mask = 0xff >> (~m_reg[2] & 0x0f);
		set_chr(m_chr_source, m_chr_base, m_chr_mask);
	}
}

/*-------------------------------------------------

 BMC-SFC-12

 Games: Rockman 6 in 1 (alt 2)

 Variant of the HIK8IN1 boards that adds 8K of VRAM
 to be used presumably by Rockman 4/6.

 NES 2.0: mapper 372

 In MAME: Supported.

 -------------------------------------------------*/

void nes_bmc_sfc12_device::set_chr(u8 chr, int chr_base, int chr_mask)
{
	if (BIT(m_reg[2], 5))
		chr8(0, CHRRAM);
	else
		nes_txrom_device::set_chr(chr, chr_base, chr_mask);
}

/*-------------------------------------------------

 BMC-SUPERHIK-4IN1

 Known Boards: Unknown Multigame Bootleg Board
 Games: Super HIK 4 in 1

 iNES: mapper 49

 In MAME: Supported. It also uses mmc3_irq.

 -------------------------------------------------*/

void nes_bmc_hik4_device::prg_cb(int start, int bank)
{
	if (m_mmc3_mode)    // all games use MMC3 PRG banking except Master Fighter III
		nes_txrom_device::prg_cb(start, bank);
}

void nes_bmc_hik4_device::write_m(offs_t offset, u8 data)
{
	LOG("bmc_hik4 write_m, offset: %04x, data: %02x\n", offset, data);

	// mid writes only work when WRAM is enabled and writable
	if ((m_wram_protect & 0xc0) == 0x80)
	{
		m_mmc3_mode = data & 0x01;
		if (m_mmc3_mode)
		{
			m_prg_base = (data & 0xc0) >> 2;
			set_prg(m_prg_base, m_prg_mask);
		}
		else                // Master Fighter III only
			prg32(BIT(data, 4, 2));

		m_chr_base = (data & 0xc0) << 1;
		set_chr(m_chr_source, m_chr_base, m_chr_mask);
	}
}

/*-------------------------------------------------

 BMC-A88S-1

 Games: 1997 Super 7 in 1, 6 in 1 (JY-201 to JY-206)

 MMC3 clone with banking for multigame menu. $5000-$5FFF holds
 the outer registers; reading it returns a DIP value that advances
 on every soft reset.

 NES 2.0: mapper 411, submapper 0 (JY-201..204), submapper 1 (CHR
 A18 taken from $5001 only) and submapper 2 (registers written at
 any $5000-$5FFF address, 256 KiB PRG size bit moved to bit 0)

 In MAME: Supported.

 TODO: Investigate...
 - Hokuto no Ken has broken sound on each cart it's on.
 - Bird Fighter boss fights have corrupt graphics.
 - whether not being able to reset from within certain
   games is correct. Examples are Samurai (title scroll
   only) and Bomber Man (in game only) on JY-202, or
   SD Gunman in JY-204.

 -------------------------------------------------*/

void nes_a88s1_device::update_banks()
{
	set_prg(0, 0xff);
	set_chr(m_chr_source, 0, 0xff);
}

void nes_a88s1_device::prg_cb(int start, int bank)
{
	const int prg_or = ((m_reg[1] << 1) & 0x10) | ((m_reg[1] >> 1) & 0x60);
	const int prg_and = (m_submapper == 2 ? BIT(m_reg[1], 0) : BIT(m_reg[1], 1)) ? 0x1f : 0x0f;

	if (BIT(m_reg[0], 6))   // NROM mode
	{
		const int nrom_bank = (m_reg[0] & 0x05) | ((m_reg[0] >> 2) & 0x02) | (prg_or >> 1);
		if (BIT(m_reg[0], 1))
			prg32(nrom_bank >> 1);
		else
		{
			prg16_89ab(nrom_bank);
			prg16_cdef(nrom_bank);
		}
	}
	else                    // MMC3 mode
		prg8_x(start, (bank & prg_and) | (prg_or & ~prg_and));
}

void nes_a88s1_device::chr_cb(int start, int bank, int source)
{
	const int chr_and = BIT(m_reg[1], 1) ? 0xff : 0x7f;
	int chr_or = (m_reg[1] << 5) & 0x080;
	if (m_submapper == 1)
		chr_or |= (m_reg[1] << 2) & 0x100;
	else
		chr_or |= ((m_reg[0] << 4) & 0x100) | ((m_reg[1] << 2) & 0x200);

	chr1_x(start, (bank & chr_and) | (chr_or & ~chr_and), source);
}

u8 nes_a88s1_device::read_l(offs_t offset)
{
	LOGMASKED(LOG_HIFREQ, "a88s1 read_l, offset: %04x\n", offset);

	// $5000-$5FFF returns the DIP value
	return (offset >= 0x0f00) ? m_reg[2] : get_open_bus();
}

void nes_a88s1_device::write_l(offs_t offset, u8 data)
{
	LOG("a88s1 write_l, offset: %04x, data: %02x\n", offset, data);

	offset += 0x100;
	if (offset >= 0x1000 && (m_submapper == 2 || BIT(offset, 11)))
	{
		m_reg[offset & 1] = data;
		update_banks();
	}
}

/*-------------------------------------------------

 BMC-F-15

 Unknown Multigame Bootleg Board
 Games: 150 in 1 Unchained Melody

 MMC3 clone with banking for multigame menu.

 NES 2.0: mapper 259

 In MAME: Supported.

 -------------------------------------------------*/

void nes_bmc_f15_device::write_m(offs_t offset, u8 data)
{
	LOG("bmc_f15 write_m, offset: %04x, data: %02x\n", offset, data);

	if (BIT(m_wram_protect, 7))
	{
		u8 bank = data & 0x0f;
		u8 mode = BIT(data, 3);
		prg16_89ab(bank & ~mode);
		prg16_cdef(bank | mode);
	}
}

void nes_bmc_f15_device::prg_cb(int start, int bank)
{
	// Ignore MMC3 PRG bank switching. Master Fighter II (game #150) uses the bank switching above.
}

/*-------------------------------------------------

 BMC-F600

 Games: Golden Mario Party II 6 in 1

 MMC3 clone with banking for multigame menu. Note,
 this cart has one TxSROM game (SMB4) and so it uses
 the nonstandard MMC3 mirroring of those boards.

 NES 2.0: mapper 370

 In MAME: Supported.

 -------------------------------------------------*/

u8 nes_bmc_f600_device::read_l(offs_t offset)
{
	LOG("bmc_f600 read_l, offset: %04x\n", offset);

	offset += 0x100;
	if (offset >= 0x1000)
		return (get_open_bus() & 0x7f) | m_jumper->read();
	else
		return get_open_bus();
}

void nes_bmc_f600_device::write_l(offs_t offset, u8 data)
{
	LOG("bmc_f600 write_l, offset: %04x, data: %02x\n", offset, data);

	offset += 0x100;
	if (offset >= 0x1000)
	{
		m_reg = offset;

		m_prg_base = (m_reg & 0x38) << 1;
		m_prg_mask = 0x1f >> BIT(m_reg, 5);
		set_prg(m_prg_base, m_prg_mask);

		m_chr_base = (m_reg & 0x07) << 7;
		m_chr_mask = 0xff >> BIT(~m_reg, 2);
		set_chr(m_chr_source, m_chr_base, m_chr_mask);
	}
}

void nes_bmc_f600_device::write_h(offs_t offset, u8 data)
{
	LOG("bmc_f600 write_h, offset: %04x, data: %02x\n", offset, data);
	if ((m_reg & 0x07) == 1)
		nes_txsrom_device::write_h(offset, data);
	else
		nes_txrom_device::write_h(offset, data);
}

void nes_bmc_f600_device::set_chr(u8 chr, int chr_base, int chr_mask)
{
	if ((m_reg & 0x07) == 1)
		nes_txsrom_device::set_chr(chr, chr_base, chr_mask);
	else
		nes_txrom_device::set_chr(chr, chr_base, chr_mask);
}

/*-------------------------------------------------

 BMC EL860947C and EL861121C boards

 Games: 8 in 1 (JY-111, JY-112, and JY-119)

 MMC3 clone with banking for multigame menu.

 NES 2.0: mapper 267 and mapper 377

 In MAME: Supported.

 -------------------------------------------------*/

void nes_bmc_el86xc_device::write_m(offs_t offset, u8 data)
{
	LOG("bmc_el86xc write_m, offset: %04x, data: %02x\n", offset, data);
	if (!(offset & 0x1000))    // game only banks via 0x6000, this mask is a guess
	{
		assert(std::has_single_bit(m_outer_prg_size));

		int outer = bitswap<3>(data, 5, 2, 1);
		m_prg_base = outer << (std::countr_zero(m_outer_prg_size) - 3);
		set_prg(m_prg_base, m_prg_mask);
		m_chr_base = outer << 7;
		set_chr(m_chr_source, m_chr_base, m_chr_mask);
	}
}

/*-------------------------------------------------

 BMC-GN-45

 Games: 4 in 1 (K-3131GS and K-3131SS)

 MMC3 clone with banking for multigame menu.

 NES 2.0: mapper 366

 In MAME: Supported.

 -------------------------------------------------*/

void nes_bmc_gn45_device::write_m(offs_t offset, u8 data)
{
	LOG("bmc_gn45 write_m, offset: %04x, data: %02x\n", offset, data);

	nes_txrom_device::write_m(offset, data);  // registers overlay WRAM

	if (!m_lock)
	{
		m_prg_base = offset & 0x70;
		set_prg(m_prg_base, m_prg_mask);
		m_chr_base = m_prg_base << 3;
		set_chr(m_chr_source, m_chr_base, m_chr_mask);

		m_lock = BIT(offset, 7);
	}
}

/*-------------------------------------------------

 BMC-GOLD-7IN1

 Known Boards: Unknown Multigame Bootleg Board
 Games: Super HIK Gold 7 in 1, Golden 7 in 1 and many more

 MMC3 clone with banking for multigame menu.

 iNES: mapper 52

 In MAME: Supported.

 -------------------------------------------------*/

void nes_bmc_gold7in1_device::write_m(offs_t offset, u8 data)
{
	LOG("bmc_gold7in1 write_m, offset: %04x, data: %02x\n", offset, data);

	if ((m_wram_protect & 0xc0) == 0x80 && !m_lock)
	{
		u8 prg128k = BIT(data, 3);
		u8 chr128k = BIT(data, 6);

		m_prg_base = (data & (0x06 | prg128k)) << 4;
		m_prg_mask = 0x1f >> prg128k;
		m_chr_base = (bitswap<3>(data, 2, 5, 4) & (0x06 | chr128k)) << 7;
		m_chr_mask = 0xff >> chr128k;
		set_prg(m_prg_base, m_prg_mask);
		set_chr(m_chr_source, m_chr_base, m_chr_mask);

		m_lock = BIT(data, 7); // mc_2hikg & mc_s3nt3 write here multiple time
	}
	else
		nes_txrom_device::write_m(offset, data);  // write WRAM
}

/*-------------------------------------------------

 BMC-K-3006

 Games: 21 in 1

 MMC3 clone with banking for multigame menu.

 NES 2.0: mapper 339

 In MAME: Supported.

 -------------------------------------------------*/

void nes_bmc_k3006_device::write_m(offs_t offset, u8 data)
{
	LOG("bmc_k3006 write_m, offset: %04x, data: %02x\n", offset, data);
	if ((m_wram_protect & 0xc0) == 0x80)
	{
		if (BIT(offset, 5))    // MMC3 mode
		{
			m_prg_base = (offset & 0x18) << 1;
			set_prg(m_prg_base, m_prg_mask);
			m_chr_base = m_prg_base << 3;
			set_chr(m_chr_source, m_chr_base, m_chr_mask);
		}
		else                   // NROM mode
		{
			u8 bank = offset & 0x1f;
			u8 mode = (offset & 0x06) == 0x06;
			prg16_89ab(bank & ~mode);
			prg16_cdef(bank | mode);
		}
	}
}

/*-------------------------------------------------

 BMC-K-3033

 Games: 35 in 1

 MMC3 clone with banking for multigame menu.

 NES 2.0: mapper 322

 In MAME: Supported.

 -------------------------------------------------*/

void nes_bmc_k3033_device::prg_cb(int start, int bank)
{
	if (m_mmc3_mode)
		nes_txrom_device::prg_cb(start, bank);
}

void nes_bmc_k3033_device::write_m(offs_t offset, u8 data)
{
	LOG("bmc_k3033 write_m, offset: %04x, data: %02x\n", offset, data);

	if ((m_wram_protect & 0xc0) == 0x80)
	{
		m_mmc3_mode = BIT(offset, 5);

		u8 mode_128k = !(BIT(offset, 7) && m_mmc3_mode);
		m_chr_base = bitswap<3>(offset, 6, 4, 3) << (8 - mode_128k);
		m_chr_mask = 0xff >> mode_128k;
		set_chr(m_chr_source, m_chr_base, m_chr_mask);

		if (m_mmc3_mode)
		{
			m_prg_base = m_chr_base >> 3;
			m_prg_mask = m_chr_mask >> 3;
			set_prg(m_prg_base, m_prg_mask);
		}
		else                // NROM mode
		{
			u8 bank = (offset & 0x40) >> 1 | (offset & 0x1f);
			u8 mode = !!(offset & 0x03);
			prg16_89ab(bank & ~mode);
			prg16_cdef(bank | mode);
		}
	}
}

/*-------------------------------------------------

 BMC-L6IN1

 Games: New Star 6 in 1

 MMC3 clone with banking for multigame menu.

 NES 2.0: mapper 345

 Note: Cabal leaves the APU in a bad state so that
 after soft resetting and loading Track & Field there
 is a constant drone pitch. This is likely a game bug.

 -------------------------------------------------*/

void nes_bmc_l6in1_device::set_prg(int prg_base, int prg_mask)
{
	if (m_reg & 0x0c)    // MMC3 mode
		nes_txrom_device::set_prg(prg_base, prg_mask);
	else                // AxROM mode
		prg32(prg_base >> 2 | (m_reg & 0x03));
}

void nes_bmc_l6in1_device::set_mirror()
{
	if (BIT(m_reg, 5))
		set_nt_mirroring(BIT(m_reg, 4) ? PPU_MIRROR_HIGH : PPU_MIRROR_LOW);
	else
		set_nt_mirroring(m_mmc_mirror ? PPU_MIRROR_HORZ : PPU_MIRROR_VERT);
}

void nes_bmc_l6in1_device::write_m(offs_t offset, u8 data)
{
	LOG("bmc_l6in1 write_m, offset: %04x, data: %02x\n", offset, data);

	if ((m_wram_protect & 0xc0) == 0x80)
	{
		m_reg = data;
		m_prg_base = (m_reg & 0xc0) >> 2;
		set_prg(m_prg_base, m_prg_mask);
		set_mirror();
	}
}

void nes_bmc_l6in1_device::write_h(offs_t offset, u8 data)
{
	LOG("bmc_l6in1 write_h, offset: %04x, data: %02x\n", offset, data);

	txrom_write(offset, data);
	if ((offset & 0x6001) == 0x2000)
		set_mirror();
}

/*-------------------------------------------------

 BMC-00202650

 Games: 8 in 1

 MMC3 clone with banking for multigame menu.

 NES 2.0: mapper 392

 In MAME: Supported.

 TODO: Soft reset doesn't work for some games some of
 the time. Seems to be caused by main RAM contents.
 Does this happen on hardware?

 -------------------------------------------------*/

void nes_bmc_00202650_device::set_prg(int prg_base, int prg_mask)
{
	if (m_mmc3_mode)
		nes_txrom_device::set_prg(prg_base, prg_mask);
}

void nes_bmc_00202650_device::set_chr(u8 chr, int chr_base, int chr_mask)
{
	if (m_mmc3_mode)
		nes_txrom_device::set_chr(chr, chr_base, chr_mask);
}

void nes_bmc_00202650_device::write_m(offs_t offset, u8 data)
{
	LOG("bmc_00202650 write_m, offset: %04x, data: %02x\n", offset, data);

	if (!m_mmc3_mode)
	{
		m_mmc3_mode = BIT(data, 4);

		m_prg_base = (data & 0x07) << 4;
		set_prg(m_prg_base, m_prg_mask);

		m_chr_base = m_prg_base << 3;
		set_chr(m_chr_source, m_chr_base, m_chr_mask);
	}
}

/*-------------------------------------------------

 BMC-411120C, BMC-810849-C

 Games: 4 in 1, 19 in 1

 MMC3 clone with banking for multigame menu.

 NES 2.0: mapper 287

 In MAME: Supported.

 -------------------------------------------------*/

void nes_bmc_411120c_device::prg_cb(int start, int bank)
{
	if (!BIT(m_reg, 3))    // all games use MMC3 PRG banking except Master Fighter II
		nes_txrom_device::prg_cb(start, bank);
}

void nes_bmc_411120c_device::write_m(offs_t offset, u8 data)
{
	LOG("bmc_411120c write_m, offset: %04x, data: %02x\n", offset, data);

	if (BIT(m_wram_protect, 7))
	{
		m_reg = offset;
		if (BIT(m_reg, 3))
			prg32(bitswap<5>(m_reg, 2, 1, 0, 5, 4));
		else
		{
			m_prg_base = (m_reg & 0x07) << 4;
			set_prg(m_prg_base, m_prg_mask);
		}
		m_chr_base = (m_reg & 0x07) << 7;
		set_chr(m_chr_source, m_chr_base, m_chr_mask);
	}
}

/*-------------------------------------------------

 BMC-81-03-05-C

 Games: 92' Super Mario Family

 MMC3 clone with banking for multigame menu. Note,
 this cart has one TxSROM game (SMB4) and so it uses
 the nonstandard MMC3 mirroring of those boards.

 NES 2.0: mapper 353

 Outer register at $8080-$E080 (bits 13-14): bank 0 uses TxSROM
 mirroring (CHR A17), bank 2 is a 128 KiB PRG bank with CHR A17
 serving as PRG A17 (and unbanked CHR-RAM when CHR A17 is high),
 bank 3 replaces PRG A17 with CPU A14 while CHR A17 is low.

 In MAME: Supported.

 TODO: on the real board PRG A17 follows the live CHR A17 line,
 which changes with every PPU fetch; here bit 7 of MMC3 register
 R0 stands in for it.

 -------------------------------------------------*/

void nes_bmc_810305c_device::prg_cb(int start, int bank)
{
	const bool chr_a17 = BIT(m_mmc_vrom_bank[0], 7);

	if (m_outer == 2 && chr_a17)
		bank |= 0x10;                                   // CHR A17 serves as PRG A17
	else if (m_outer == 3 && !chr_a17)
		bank = (bank & ~0x10) | ((start & 2) << 3);     // CPU A14 serves as PRG A17

	prg8_x(start, bank);
}

void nes_bmc_810305c_device::set_chr(u8 chr, int chr_base, int chr_mask)
{
	if (m_outer == 2 && BIT(m_mmc_vrom_bank[0], 7))
		chr8(0, CHRRAM);
	else if (m_outer)
		nes_txrom_device::set_chr(chr, chr_base, chr_mask);
	else
		nes_txsrom_device::set_chr(chr, chr_base, chr_mask);
}

void nes_bmc_810305c_device::write_h(offs_t offset, u8 data)
{
	LOG("bmc_810305c write_h, offset: %04x, data: %02x\n", offset, data);

	if (BIT(offset, 7))    // outer register
	{
		m_outer = BIT(offset, 13, 2);

		m_prg_base = m_outer << 5;
		m_prg_mask = 0x1f >> ((m_outer == 2) ? 1 : 0);
		set_prg(m_prg_base, m_prg_mask);

		m_chr_base = m_outer << 7;
		set_chr(m_chr_source, m_chr_base, m_chr_mask);
	}
	else if (m_outer)      // standard MMC3 registers
		nes_txrom_device::write_h(offset, data);
	else                   // TxSROM MMC3 registers without mirroring bit
		nes_txsrom_device::write_h(offset, data);
}

/*-------------------------------------------------

 BMC-820720C

 Games: 1993 Super HiK 8 in 1 (G-002)

 MMC3 clone with banking for multigame menu.

 NES 2.0: mapper 393

 In MAME: Supported.

 -------------------------------------------------*/

void nes_bmc_820720c_device::set_prg(int prg_base, int prg_mask)
{
	switch (m_reg >> 4)
	{
		case 0:            // MMC3 mode
		case 1:
			nes_txrom_device::set_prg(prg_base, prg_mask);
			break;
		case 2:            // BNROM mode
			prg32((prg_base | (m_mmc_prg_bank[BIT(m_latch, 6) << 1] & prg_mask)) >> 2);
			break;
		case 3:            // UNROM mode
			prg16_89ab(m_prg_base >> 1);
			prg16_cdef(m_prg_base >> 1 | 0x07);
			break;
	}
}

void nes_bmc_820720c_device::set_chr(u8 chr, int chr_base, int chr_mask)
{
	if (BIT(m_reg, 3))
		chr8(0, CHRRAM);
	else
		nes_txrom_device::set_chr(chr, chr_base, chr_mask);
}

void nes_bmc_820720c_device::write_m(offs_t offset, u8 data)
{
	LOG("bmc_820720c write_m, offset: %04x, data: %02x\n", offset, data);

	if ((m_wram_protect & 0xc0) == 0x80)
	{
		m_reg = offset & 0x3f;

		m_prg_base = (m_reg & 0x07) << 4;
		set_prg(m_prg_base, m_prg_mask);

		m_chr_base = (m_reg & 0x01) << 8;
		set_chr(m_chr_source, m_chr_base, m_chr_mask);
	}
}

void nes_bmc_820720c_device::write_h(offs_t offset, u8 data)
{
	LOG("bmc_820720c write_h, offset: %04x, data: %02x\n", offset, data);

	txrom_write(offset, data);     // MMC3 regs always written (for mirroring in non-MMC3 modes)

	if ((m_reg & 0x30) == 0x30)    // UNROM mode
	{
		prg16_89ab(m_prg_base >> 1 | (data & 0x07));
		prg16_cdef(m_prg_base >> 1 | 0x07);
	}
}

/*-------------------------------------------------

 BMC-830118C

 Games: 7 in 1 (EW-002, M-022, M-026, M-027)

 MMC3 clone with banking for multigame menu.

 NES 2.0: mapper 348

 The outer register at $6800 only responds when PRG-RAM is
 enabled and writable ($A001 = $80-$BF). Outer bank 3 selects
 the GNROM-like mode: MMC3 banks for $8000-$BFFF with bit 1
 cleared, and for $C000-$FFFF with bit 1 set.

 In MAME: Preliminary supported.

 TODO: Only M-022 seems to reliably soft reset to the
 menu for all games. The other three carts are hit or
 miss. MMC3 games in particular are not likely to
 reset to the menu. Investigate further...

 -------------------------------------------------*/

void nes_bmc_830118c_device::prg_cb(int start, int bank)
{
	if (m_prg_base == 0x30)
	{
		if (start <= 1)
		{
			prg8_x(start, bank & ~2);
			prg8_x(start + 2, bank | 2);
		}
	}
	else
		nes_txrom_device::prg_cb(start, bank);
}

void nes_bmc_830118c_device::write_m(offs_t offset, u8 data)
{
	LOG("bmc_830118c write_m, offset: %04x, data: %02x\n", offset, data);

	if ((m_wram_protect & 0xc0) == 0x80)
	{
		m_prg_base = (data & 0x0c) << 2;
		set_prg(m_prg_base, m_prg_mask);
		m_chr_base = m_prg_base << 3;
		set_chr(m_chr_source, m_chr_base, m_chr_mask);
	}
}

/*-------------------------------------------------

 BMC-830832C

 Games: 1994 Super HiK 3 in 1 (JY-007)

 MMC3 clone with banking for multigame menu.

 NES 2.0: mapper 364

 In MAME: Supported.

 -------------------------------------------------*/

void nes_bmc_830832c_device::write_m(offs_t offset, u8 data)
{
	LOG("bmc_830832c write_m, offset: %04x, data: %02x\n", offset, data);
	if (offset & 0x1000)    // game only writes 0x7000, this mask is a guess
	{
		m_prg_base = (data & 0x40) >> 1;
		m_prg_mask = 0x1f >> BIT(data, 5);
		set_prg(m_prg_base, m_prg_mask);

		m_chr_base = (data & 0x10) << 4;
		m_chr_mask = 0xff >> BIT(data, 5);
		set_chr(m_chr_source, m_chr_base, m_chr_mask);
	}
}

/*-------------------------------------------------

 BMC-YY841101C

 Games: 4 in 1 (OK-411, JY-009, JY-018, JY-019, JY-020)

 MMC3 clone with banking for multigame menu.

 NES 2.0: mapper 361

 In MAME: Supported.

 -------------------------------------------------*/

void nes_bmc_yy841101c_device::write_m(offs_t offset, u8 data)
{
	LOG("bmc_yy841101c write_m, offset: %04x, data: %02x\n", offset, data);
	if (offset & 0x1000)    // games only write 0x7000, this mask is a guess
	{
		m_prg_base = data & 0xf0;
		set_prg(m_prg_base, m_prg_mask);
		m_chr_base = (data & 0xf0) << 3;
		set_chr(m_chr_source, m_chr_base, m_chr_mask);
	}
}

/*-------------------------------------------------

 BMC-YY841155C

 Games: Donkey Kong 8 in 1 (JY-041)

 MMC3 clone with banking for multigame menu.

 NES 2.0: mapper 376

 In MAME: Supported.

 -------------------------------------------------*/

void nes_bmc_yy841155c_device::write_m(offs_t offset, u8 data)
{
	LOG("bmc_yy841155c write_m, offset: %04x, data: %02x\n", offset, data);

	if (offset >= 0x1000)
	{
		m_reg[offset & 1] = data;

		m_prg_base = (m_reg[1] & 0x01) << 5 | (m_reg[0] & 0x40) >> 2;
		if (BIT(m_reg[0], 7))    // NROM mode
		{
			u8 bank = m_prg_base >> 1 | (m_reg[0] & 0x07);
			u8 mode = BIT(m_reg[0], 5);
			prg16_89ab(bank & ~mode);
			prg16_cdef(bank | mode);
		}
		else                     // MMC3 mode
			set_prg(m_prg_base, m_prg_mask);

		m_chr_base = m_prg_base << 3;
		set_chr(m_chr_source, m_chr_base, m_chr_mask);
	}
}

/*-------------------------------------------------

 TEC9719 (mapper 126), BS-300 / ING003C family (mapper 422) and
 ING-022 (mapper 534) enhanced MMC3 ASICs

 Games: PowerJoy 84-in-1 (126); many multicarts (422); Atari
 Flashback Mini 7800, Intellivision 25-in-1 and others (534)

 Four outer registers overlay WRAM at $6000-$7FFF (writable only
 while $A001 bit 7 is set): $6000 PRG/CHR outer banks and sizes,
 $6001 SL0 menu input / mirroring mode / PRG chip select,
 $6002 CNROM CHR bank and bit locks, $6003 banking mode (MMC3,
 NROM-128/256, UNROM, ANROM) and register lock. The ASIC addresses
 up to 4 MiB of PRG-ROM and repurposes high PRG address lines as
 CHR lines: mapper 126 swaps CHR A18/A19 compared to 422 and 534.
 Mapper 534 also inverts the data written to the MMC3 IRQ latch
 at $C000. NES 2.0 submapper 1 uses PRG A21 as chip select
 between two 1 MiB PRG chips, submapper 2 uses $6001 bit 2, submapper 3
 substitutes PRG A14 and CHR A14 with $6000 bit 5 (when $6000 bit 2 is
 set) and submapper 4 (LD822 PCB) wires CHR A20..A18 to PRG A20..A18.
 The banking follows FCEUmm's 126_422_534.c, which also lists some old
 dumps that need their PRG/CHR halves swapped (not handled here).

 NES 2.0: mappers 126, 422 and 534

 In MAME: Supported.

 -------------------------------------------------*/

u8 nes_pjoy84_device::get_mmc3_bank(int bank) const
{
	if (!(bank & 1) && BIT(m_latch, 6))
		bank ^= 2;
	return (bank & 2) ? (0xfe | (bank & 1)) : u8(m_mmc_prg_bank[bank & 1]);
}

void nes_pjoy84_device::update_prg()
{
	int prg_and = BIT(m_reg[0], 6) ? 0x0f : 0x1f;     // 128 or 256 KiB inner PRG bank
	int prg_or = (((m_reg[0] << 4) & 0x070) | (((m_reg[0] ^ 0x20) << 3) & 0x180)) & ~prg_and;     // outer PRG bank
	if (m_submapper == 1)
		prg_or = (prg_or & 0x7f) | ((prg_or >> 1) & 0x80);      // PRG A21 selects between two 1 MiB chips
	if (m_submapper == 2)
		prg_or = (prg_or & 0x7f) | ((m_reg[1] << 5) & 0x80);    // $6001 bit 2 selects between two 1 MiB chips
	if (m_submapper == 3 && BIT(m_reg[0], 2))
	{
		// PRG A14 is replaced by PRG A21
		prg_and &= ~0x02;
		prg_or |= BIT(m_reg[0], 5) ? 0x00 : 0x02;
	}

	// In UNROM-like mode (CT3=1, CT2=1, CT0=1) the MMC3 sees A13=0 and A14=CPU A14 during reads, making register 6 apply
	// from $8000-$BFFF and the fixed bank from $C000-$FFFF. In NROM-128, NROM-256, ANROM and UNROM modes (CT0=1) it sees
	// A13=0 and A14=0, making register 6 apply from $8000-$FFFF.
	const int page_mask = ((m_reg[3] & 0x0d) == 0x0d) ? 2 : BIT(m_reg[3], 0) ? 0 : 3;

	for (int page = 0; page < 4; page++)
	{
		u8 v = get_mmc3_bank(page & page_mask);

		// UNROM and ANROM modes mean that MMC3 register 6 selects 16 and 32 KiB rather than 8 KiB banks
		if (BIT(m_reg[3], 3))
		{
			switch (m_reg[3] & 0x03)
			{
				case 0: v = (v & 3) | ((v << 1) & ~3); break;       // PRG A14 appears twice
				case 1: v = (page & 3) | ((v << 1) & ~1); break;    // 16 KiB mode, bit 0 OR'd with CPU A14
				case 2: v = (v & 3) | ((v << 2) & ~3); break;       // PRG A13 and PRG A14 appear twice
				case 3: v = (page & 3) | ((v << 2) & ~3); break;    // 32 KiB mode
			}
		}
		else if (BIT(m_reg[3], 0))  // regular NROM modes
		{
			v = (page & 1) | (v & ~1);
			if (BIT(m_reg[3], 1))
				v = (page & 2) | (v & ~2);                          // NROM-256
		}

		prg8_x(page, (v & prg_and) | (prg_or & ~prg_and));
	}
}

void nes_pjoy84_device::update_chr()
{
	int chr_and = BIT(m_reg[0], 7) ? 0x7f : 0xff;     // 128 or 256 KiB inner CHR bank
	int chr_or;                                         // outer CHR bank
	if (m_mapper == 126)    // mapper 126 swaps CHR A18 and A19
		chr_or = (((m_reg[0] << 4) & 0x080) | (((m_reg[0] ^ 0x20) << 3) & 0x100) | ((m_reg[0] << 5) & 0x200)) & ~chr_and;
	else if (m_submapper == 4)  // LD822 PCB: CHR A20..A18 = PRG A20..A18
		chr_or = (((m_reg[0] << 4) & 0x080) | ((m_reg[0] << 7) & 0x100) | ((~m_reg[0] << 4) & 0x200) | ((m_reg[0] << 6) & 0x400)) & ~chr_and;
	else
		chr_or = ((((m_reg[0] ^ 0x20) << 4) & 0x380) | ((m_reg[0] << 8) & 0x400)) & ~chr_and;
	if (m_submapper == 3 && BIT(m_reg[0], 2))
	{
		// CHR A14 is replaced by PRG A21
		chr_and &= ~0x10;
		chr_or |= BIT(m_reg[0], 5) ? 0x00 : 0x10;
	}

	if (BIT(m_reg[3], 4))   // CNROM mode: the 8 KiB inner CHR bank comes from outer bank register #2
	{
		chr8((m_reg[2] & (chr_and >> 3)) | ((chr_or & ~chr_and) >> 3), m_chr_source);
		return;
	}

	const u8 chr_page = (m_latch & 0x80) >> 5;
	chr1_x(chr_page ^ 0, ((m_mmc_vrom_bank[0] & ~0x01) & chr_and) | chr_or, m_chr_source);
	chr1_x(chr_page ^ 1, ((m_mmc_vrom_bank[0] |  0x01) & chr_and) | chr_or, m_chr_source);
	chr1_x(chr_page ^ 2, ((m_mmc_vrom_bank[1] & ~0x01) & chr_and) | chr_or, m_chr_source);
	chr1_x(chr_page ^ 3, ((m_mmc_vrom_bank[1] |  0x01) & chr_and) | chr_or, m_chr_source);
	chr1_x(chr_page ^ 4, (u8(m_mmc_vrom_bank[2]) & chr_and) | chr_or, m_chr_source);
	chr1_x(chr_page ^ 5, (u8(m_mmc_vrom_bank[3]) & chr_and) | chr_or, m_chr_source);
	chr1_x(chr_page ^ 6, (u8(m_mmc_vrom_bank[4]) & chr_and) | chr_or, m_chr_source);
	chr1_x(chr_page ^ 7, (u8(m_mmc_vrom_bank[5]) & chr_and) | chr_or, m_chr_source);
}

void nes_pjoy84_device::update_mirroring()
{
	if (BIT(m_reg[3], 5))   // ANROM mirroring from MMC3 register 6 bit 4
		set_nt_mirroring(BIT(m_mmc_prg_bank[0], 4) ? PPU_MIRROR_HIGH : PPU_MIRROR_LOW);
	else if (BIT(m_reg[1], 1))  // extended mirroring
	{
		switch (m_mmc_mirror & 3)
		{
			case 0: set_nt_mirroring(PPU_MIRROR_VERT); break;
			case 1: set_nt_mirroring(PPU_MIRROR_HORZ); break;
			case 2: set_nt_mirroring(PPU_MIRROR_LOW); break;
			case 3: set_nt_mirroring(PPU_MIRROR_HIGH); break;
		}
	}
	else
		set_nt_mirroring(BIT(m_mmc_mirror, 0) ? PPU_MIRROR_HORZ : PPU_MIRROR_VERT);
}

void nes_pjoy84_device::update_banks()
{
	update_prg();
	update_chr();
	update_mirroring();
}

void nes_pjoy84_device::prg_cb(int start, int bank)
{
	update_prg();
}

void nes_pjoy84_device::chr_cb(int start, int bank, int source)
{
	update_chr();
}

u8 nes_pjoy84_device::read_m(offs_t offset)
{
	// WRAM stays connected regardless of the MMC3 WRAM enable bit
	return device_nes_cart_interface::read_m(offset);
}

u8 nes_pjoy84_device::read_h(offs_t offset)
{
	// $6001 bit 0: PRG A0 comes from the SL0 input (menu selection)
	if (BIT(m_reg[1], 0))
		offset = (offset & ~1) | BIT(m_sl0, 0);
	return hi_access_rom(offset);
}

void nes_pjoy84_device::write_m(offs_t offset, u8 data)
{
	LOG("pjoy84 write_m, offset: %04x, data: %02x\n", offset, data);

	device_nes_cart_interface::write_m(offset, data);   // clearing the MMC3 WRAM bit does not disable WRAM itself

	// the outer registers can only be written while the MMC3 WRAM bit is set
	if (!BIT(m_wram_protect, 7))
		return;

	const u8 reg = offset & 0x03;
	if (reg == 2)
	{
		// CNROM bank (D0-D3), bank enable (D4-D6) and bank enable lock (D7)
		const int latch_mask = 0xff & ~(BIT(m_reg[2], 7) ? 0xf0 : 0x00) & ~((m_reg[2] >> 3) & 0x0e);
		m_reg[2] = (m_reg[2] & ~latch_mask) | (data & latch_mask);
		update_chr();
	}
	else if (!BIT(m_reg[3], 7))
	{
		// lock bit clear: update any other outer bank register
		m_reg[reg] = data;
		update_banks();
	}
}

void nes_pjoy84_device::write_h(offs_t offset, u8 data)
{
	LOG("pjoy84 write_h, offset: %04x, data: %02x\n", offset, data);

	if ((m_reg[3] & 0x09) == 0x09)
		txrom_write(1, data);       // UNROM and ANROM modes treat all writes to $8000-$FFFF as if they were going to $8001
	else
	{
		if (offset >= 0x4000 && m_mapper == 534)
			data ^= 0xff;           // mapper 534 inverts the MMC3 scanline counter reload value
		txrom_write(offset, data);
	}
	update_banks();
}

/*-------------------------------------------------

 COOLBOY, MINDKIDS, and other AA6023 / SMD132 / SMD133 boards

 Games: many multicarts, Chinese games, and some
 modern re-releases such as Metal Storm and Holy Diver

 MMC3 clone with six outer registers. NES 2.0 submapper bit 0
 selects the solder pad (odd: registers at $5000-$5FFF, even: registers
 overlay WRAM at $6000-$6FFF); submapper bits 1-3 select the revision /
 PCB wiring: 0/1 original implementation (CHR-RAM only, or mixed
 CHR-ROM/RAM selected by register 4), 2/3 later revision with a different
 arrangement of register 1, 4/5 LD622D (PRG A20-21 moved to register 0),
 6/7 J-852C (CHR A17 selects between two PRG chips), 8/9 2 MiB regular plus
 an extra latch and write-protectable CHR-RAM, 10/11 single-screen mirroring
 controlled by register 0.

 NES 2.0: mapper 268, submappers 0-11

 In MAME: Supported.

 -------------------------------------------------*/

void nes_smd133_device::prg_cb(int start, int bank)
{
	const int prg_mask_mmc3 = (BIT(m_reg[3], 4) ? 0x00 : 0x0f)
			| (BIT(m_reg[0], 6) ? 0x00 : 0x10)
			| (BIT(m_reg[1], 7) ? 0x00 : 0x20)
			| (BIT(m_reg[1], 6) ? 0x40 : 0x00)
			| (BIT(m_reg[1], 5) ? 0x80 : 0x00);
	int prg_mask_gnrom = 0;
	int prg_offset = 0;

	switch (m_submapper & ~1)
	{
		default:    // original AA6023
			prg_mask_gnrom = BIT(m_reg[3], 4) ? (BIT(m_reg[1], 1) ? 0x03 : 0x01) : 0;
			prg_offset = (m_reg[3] & 0x0e) | ((m_reg[0] << 4) & 0x070) | ((m_reg[1] << 3) & 0x080)
					| ((m_reg[1] << 6) & 0x300) | ((m_reg[0] << 6) & 0xc00) | ((~m_reg[1] << 12) & 0x1000);
			break;

		case 2:     // later revision with a different arrangement of register 1
			prg_mask_gnrom = BIT(m_reg[3], 4) ? (BIT(m_reg[1], 4) ? 0x01 : 0x03) : 0;
			prg_offset = (m_reg[3] & 0x0e) | ((m_reg[0] << 4) & 0x070) | ((m_reg[1] << 4) & 0x080)
					| ((m_reg[1] << 6) & 0x100) | ((m_reg[1] << 8) & 0x200) | ((m_reg[0] << 6) & 0xc00);
			break;

		case 4:     // LD622D: PRG A20-21 moved to register 0
			prg_mask_gnrom = BIT(m_reg[3], 4) ? (BIT(m_reg[1], 1) ? 0x03 : 0x01) : 0;
			prg_offset = (m_reg[3] & 0x0e) | ((m_reg[0] << 4) & 0x070) | ((m_reg[0] << 3) & 0x180);
			break;

		case 6:     // J-852C: CHR A17 (or bit 3 of register 0) selects one of two PRG chips
		{
			prg_mask_gnrom = BIT(m_reg[3], 4) ? (BIT(m_reg[1], 1) ? 0x03 : 0x01) : 0;
			prg_offset = (m_reg[3] & 0x0e) | ((m_reg[0] << 4) & 0x070) | ((m_reg[1] << 3) & 0x080)
					| ((m_reg[1] << 6) & 0x300) | ((m_reg[0] << 6) & 0xc00);
			const int chip_size = (m_prg_size / 0x2000) / 2;
			prg_offset &= chip_size - 1;
			if (BIT(m_reg[0], 7) ? BIT(m_reg[0], 3) : BIT(m_mmc_vrom_bank[0], 7))
				prg_offset |= chip_size;
			break;
		}

		case 8:     // 2 MiB regular plus extra latch (register 6)
			prg_mask_gnrom = BIT(m_reg[3], 4) ? (BIT(m_reg[1], 1) ? 0x03 : 0x01) : 0;
			prg_offset = (m_reg[3] & 0x0e) | ((m_reg[0] << 4) & 0x070) | ((m_reg[1] << 3) & 0x080)
					| ((m_reg[6] << 8) & 0x300) | ((m_reg[6] << 6) & 0xc00);
			break;
	}

	prg_offset &= ~(prg_mask_mmc3 | prg_mask_gnrom);
	prg8_x(start, (bank & prg_mask_mmc3) | prg_offset | (start & prg_mask_gnrom));
}

void nes_smd133_device::chr_cb(int start, int bank, int source)
{
	const int chr_mask_mmc3 = BIT(m_reg[3], 4) ? 0x00 : BIT(m_reg[0], 7) ? 0x7f : 0xff;
	const int chr_mask_gnrom = BIT(m_reg[3], 4) ? 0x07 : 0x00;
	int chr_offset = ((m_reg[0] << 9) & 0xc00) | ((m_reg[0] << 4) & 0x380) | ((m_reg[2] << 3) & 0x078);
	chr_offset &= ~(chr_mask_mmc3 | chr_mask_gnrom);

	// mixed CHR-ROM/RAM boards: a bank matching register 4 selects CHR-RAM
	const bool mixed_ram = !m_vram.empty() && m_vrom && BIT(m_reg[4], 0) && ((bank & 0xfe) == (m_reg[4] & 0xfe));
	chr1_x(start, (bank & chr_mask_mmc3) | chr_offset | (start & chr_mask_gnrom), mixed_ram ? CHRRAM : source);
}

void nes_smd133_device::update_mirroring()
{
	if ((m_submapper & ~1) == 10 && !BIT(m_reg[0], 5))
		set_nt_mirroring(BIT(m_reg[0], 4) ? PPU_MIRROR_HIGH : PPU_MIRROR_LOW);   // single-screen mirroring
	else
		set_nt_mirroring(BIT(m_mmc_mirror, 0) ? PPU_MIRROR_HORZ : PPU_MIRROR_VERT);
}

void nes_smd133_device::update_banks()
{
	set_prg(0, 0xff);
	set_chr(m_chr_source, 0, 0xff);
	update_mirroring();
}

u8 nes_smd133_device::read_m(offs_t offset)
{
	return (m_wram_protect & 0xa0) ? device_nes_cart_interface::read_m(offset) : get_open_bus();
}

void nes_smd133_device::smd133_write(offs_t offset, u8 data)
{
	const int index = offset & 0x07;
	if (index > 5)
		return;
	if (BIT(m_reg[3], 7) && index != 2)     // register lock
		return;

	if (index == 2)
	{
		// bit 7 locks the upper nibble, bits 4-6 lock individual lower bits
		if (BIT(m_reg[2], 7))
			data = (data & 0x0f) | (m_reg[2] & 0xf0);
		data &= ((~m_reg[2] >> 3) & 0x0e) | 0xf1;
	}

	if ((m_submapper & ~1) == 8 && index == 1 && (data & 0x0c) == 0x0c)
		m_reg[6] = data;

	m_reg[index] = data;
	update_banks();
}

void nes_smd133_device::write_l(offs_t offset, u8 data)
{
	LOG("smd133 write_l, offset: %04x, data: %02x\n", offset, data);

	if (m_smd133_addr == 0x5000 && offset >= 0x0f00)
		smd133_write(offset, data);
}

void nes_smd133_device::write_m(offs_t offset, u8 data)
{
	LOG("smd133 write_m, offset: %04x, data: %02x\n", offset, data);

	if ((BIT(m_wram_protect, 7) && !BIT(m_wram_protect, 6)) || BIT(m_wram_protect, 5))
		device_nes_cart_interface::write_m(offset, data);   // registers overlay, but are independent of, WRAM

	if (!BIT(m_submapper, 0) && offset < 0x1000)
		smd133_write(offset, data);
}

void nes_smd133_device::write_h(offs_t offset, u8 data)
{
	txrom_write(offset, data);
	update_mirroring();     // submappers 10/11 override the MMC3 mirroring register
}

void nes_smd133_device::chr_w(offs_t offset, u8 data)
{
	// submapper 8/9: bit 4 of register 0 write-protects the main CHR-RAM (not the extra CHR-RAM of mixed CHR-ROM/RAM boards)
	if ((m_submapper & ~1) == 8 && BIT(m_reg[0], 4) && !m_vrom)
		return;
	device_nes_cart_interface::chr_w(offset, data);
}

/*-------------------------------------------------

 8025 enhanced MMC3 ASIC

 Games: Waixing 2005+ re-releases and 245-in-1 Real Game (FS005/FS006),
 Super Mario 160-in-1 Funny Time (JX9003B), GameStar Smart Genius Deluxe,
 Game 500-in-1 (HST-162)

 MMC3 clone with outer registers at $5xx0-$5xx3 (mask selected by a solder
 pad, see the config port) for PRG/CHR banking modes and outer banks, up to
 32 MiB of PRG, optional extended MMC3 mode (RAMBO-1 style 1 KiB CHR banks and
 four selectable PRG banks), UNROM/CNROM latches and several submapper
 specific upper address lines:
 - submapper 0 (LP-8002KB, SFC-12B): 6 MMC3 PRG bits, CHR-ROM/CHR-RAM selected by $5xx0.5
 - submapper 1 (FK-xxx/BS-xxx): 8 MMC3 PRG bits, extended MMC3 mode, CNROM CHR latch
 - submapper 2 (FS005): PRG A21-A25 from $5xx0/$5xx2, RAM configuration
   register at $A001 (32 KiB WRAM, mixed CHR-ROM/RAM, single-screen
   mirroring, outer registers can be disabled; used for copy protection)
 - submapper 3 (JX9003B): PRG/CHR MSB registers at $5xx5/$5xx6
 - submapper 4: PRG A21 from $5xx2 bit 7
 - submapper 5 (HST-162): PRG MSB at $4800, $5xx1 only supplies A14-A18

 NES 2.0: mapper 176, submappers 0-5. The older BMC-SUPER24IN1SC03 and
 BMC-FK23C/FK23CA boards above are only used for UNIF files and software
 list entries.

 In MAME: Supported (checked against the NESdev register description).

 TODO: PRG-RAM write protect and the FS005 PRG-RAM bank switching are
 only roughly emulated.

 -------------------------------------------------*/

nes_8025_device::nes_8025_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: nes_txrom_device(mconfig, NES_8025, tag, owner, clock)
	, m_config(*this, "CONFIG")
	, m_submapper(0)
	, m_mode(0)
	, m_prg_lsb(0)
	, m_chr_lsb(0)
	, m_ext(0)
	, m_prg_msb(0)
	, m_chr_msb(0)
	, m_latch_data(0)
{
}

void nes_8025_device::device_start()
{
	mmc3_start();
	save_item(NAME(m_submapper));
	save_item(NAME(m_mode));
	save_item(NAME(m_prg_lsb));
	save_item(NAME(m_chr_lsb));
	save_item(NAME(m_ext));
	save_item(NAME(m_prg_msb));
	save_item(NAME(m_chr_msb));
	save_item(NAME(m_latch_data));
}

void nes_8025_device::pcb_start(running_machine &machine, u8 *ciram_ptr, bool cart_mounted)
{
	if (m_submapper == 2)
	{
		// FS005: 32 KiB WRAM (four 8 KiB banks) and 8 KiB CHR-RAM
		if (m_prgram.size() < 0x8000)
			prgram_alloc(0x8000);
		if (m_vram.size() < 0x2000)
			vram_alloc(0x2000);
	}
	nes_txrom_device::pcb_start(machine, ciram_ptr, cart_mounted);
}

void nes_8025_device::pcb_reset()
{
	m_mode = m_prg_lsb = m_chr_lsb = m_ext = m_prg_msb = m_chr_msb = m_latch_data = 0;
	mmc3_common_initialize(0xff, 0xff, 0);

	// power-on values of the MMC3 registers: R6 = $00, R7 = $01, R8 = $FE, R9 = $FF, R10 = R11 = $FF
	m_mmc_prg_bank[0] = 0x00;
	m_mmc_prg_bank[1] = 0x01;
	m_mmc_vrom_bank[6] = 0xff;
	m_mmc_vrom_bank[7] = 0xff;

	if (m_submapper == 2)
		m_wram_protect = 0x00;  // RAM configuration register powers on cleared

	update_banks();
}

int nes_8025_device::mmc3_prg_bank(int page, bool ext) const
{
	switch ((page & 1) ? page : (page ^ (BIT(m_latch, 6) ? 2 : 0)))     // PRG A14 inversion only swaps $8000 and $C000
	{
		case 0:  return m_mmc_prg_bank[0] & 0xff;
		case 1:  return m_mmc_prg_bank[1] & 0xff;
		case 2:  return ext ? (m_mmc_prg_bank[2] & 0xff) : 0xfe;
		default: return ext ? (m_mmc_prg_bank[3] & 0xff) : 0xff;
	}
}

int nes_8025_device::prg_bank(int page) const
{
	const bool ext = ext_mode();
	u8 mode = ext ? 0 : (m_mode & 0x07);    // the PRG banking mode is ignored in extended MMC3 mode
	if (mode > 5)
		mode = 0;                           // modes 6 and 7 are never used

	// outer PRG bank in 8 KiB units (bit 0 = A13); the PRG base register supplies A14 and up
	int outer = (m_prg_lsb & 0x7f) << 1;
	switch (m_submapper)
	{
		case 2:
			// A21 = $5xx0.3, A22 = $5xx0.7, A23 = $5xx2.6, A24 = $5xx2.7, A25 = $5xx2.5
			outer |= (BIT(m_mode, 3) << 8) | (BIT(m_mode, 7) << 9) | (BIT(m_chr_lsb, 6) << 10) | (BIT(m_chr_lsb, 7) << 11) | (BIT(m_chr_lsb, 5) << 12);
			break;
		case 3:
			outer |= (m_prg_msb & 0x0f) << 8;               // A24-A21
			break;
		case 4:
			outer |= BIT(m_chr_lsb, 7) << 8;                // A21
			break;
		case 5:
			outer = ((m_prg_lsb & 0x1f) << 1) | ((m_prg_msb & 0x3f) << 6);     // $5xx1 = A18-A14, $4800 = A24-A19
			break;
		default:
			break;
	}

	switch (mode)
	{
		default:
		case 0:
		case 1:
		case 2:
		{
			// MMC3 PRG modes: 2 MiB (submappers 1 and 3 only) / 512 KiB, 256 KiB or 128 KiB inner banks
			int mask;
			if (ext)
				mask = 0xff;
			else if (mode == 0)
				mask = (m_submapper == 1 || m_submapper == 3) ? 0xff : 0x3f;
			else
				mask = (mode == 1) ? 0x1f : 0x0f;
			return (mmc3_prg_bank(page, ext) & mask) | (outer & ~mask);
		}

		case 3:     // NROM-128: 16 KiB at $8000-$BFFF mirrored at $C000-$FFFF
			return (outer & ~1) | (page & 1);

		case 4:     // NROM-256: 32 KiB at $8000-$FFFF
			return (outer & ~3) | page;

		case 5:     // UNROM: 16 KiB at $8000-$BFFF from the latch, inner bank #7 at $C000-$FFFF
		{
			const int latch = (page < 2) ? (m_latch_data & 0x07) : 0x07;
			return ((((outer >> 1) & ~0x07) | latch) << 1) | (page & 1);
		}
	}
}

void nes_8025_device::set_prg(int prg_base, int prg_mask)
{
	for (int page = 0; page < 4; page++)
		prg8_x(page, prg_bank(page));
}

void nes_8025_device::set_chr(u8 chr, int chr_base, int chr_mask)
{
	u8 source = chr;
	if (m_submapper <= 1 && BIT(m_mode, 5) && !m_vram.empty() && m_vrom_chunks)
		source = CHRRAM;    // boards with both CHR-ROM and CHR-RAM (e.g. SFC-12B) select between them with $5xx0.5
	if (source == CHRRAM && m_vram.empty())
		source = CHRROM;
	if (source == CHRROM && !m_vrom_chunks)
		source = CHRRAM;

	// FS005: the RAM configuration register can make the first 8 KiB of the CHR address space CHR-RAM (like mapper 195)
	const bool ram_low = ram_cfg() && BIT(m_wram_protect, 2) && !m_vram.empty();
	auto put = [this, source, ram_low] (int slot, int bank)
	{
		chr1_x(slot, bank, (ram_low && bank < 8) ? CHRRAM : source);
	};

	if (!BIT(m_mode, 6))
	{
		// MMC3 CHR mode: 256 KiB ($5xx0.4 = 0) or 128 KiB inner banks, CHR base register supplies the upper bits
		const int chr_and = BIT(m_mode, 4) ? 0x7f : 0xff;
		int outer = m_chr_lsb << 3;                         // CHR A20-A13 in 1 KiB units
		if (m_submapper == 3)
			outer |= (m_chr_msb & 0x0f) << 11;              // CHR A24-A21

		const bool ext = ext_mode();
		const int page = (m_latch & 0x80) >> 5;
		int bank[8];
		if (ext)
		{
			// extended MMC3 mode: six 1 KiB banks plus R10 and R11
			bank[0] = m_mmc_vrom_bank[0]; bank[1] = m_mmc_vrom_bank[6];
			bank[2] = m_mmc_vrom_bank[1]; bank[3] = m_mmc_vrom_bank[7];
		}
		else
		{
			bank[0] = m_mmc_vrom_bank[0] & ~0x01; bank[1] = m_mmc_vrom_bank[0] | 0x01;
			bank[2] = m_mmc_vrom_bank[1] & ~0x01; bank[3] = m_mmc_vrom_bank[1] | 0x01;
		}
		bank[4] = m_mmc_vrom_bank[2];
		bank[5] = m_mmc_vrom_bank[3];
		bank[6] = m_mmc_vrom_bank[4];
		bank[7] = m_mmc_vrom_bank[5];

		for (int i = 0; i < 8; i++)
			put(page ^ i, (bank[i] & chr_and) | (outer & ~chr_and));
	}
	else
	{
		// CHR A10-A12 come from the PPU: an 8 KiB bank selected by the CHR base register (and the CNROM latch on submapper 1)
		int bank8 = m_chr_lsb;
		if (m_submapper == 1 && !BIT(m_mode, 5))
			bank8 = BIT(m_mode, 4) ? ((m_chr_lsb & ~0x01) | (m_latch_data & 0x01)) : ((m_chr_lsb & ~0x03) | (m_latch_data & 0x03));
		if (m_submapper == 3)
			bank8 |= (m_chr_msb & 0x0f) << 8;

		for (int i = 0; i < 8; i++)
			put(i, (bank8 << 3) | i);
	}
}

void nes_8025_device::update_mirroring()
{
	if (ram_cfg() && BIT(m_mmc_mirror, 1))      // single-screen mirroring is only available when the RAM configuration register is enabled
		set_nt_mirroring(BIT(m_mmc_mirror, 0) ? PPU_MIRROR_HIGH : PPU_MIRROR_LOW);
	else
		set_nt_mirroring(BIT(m_mmc_mirror, 0) ? PPU_MIRROR_HORZ : PPU_MIRROR_VERT);
}

void nes_8025_device::update_banks()
{
	set_prg(0, 0);
	set_chr(m_chr_source, 0, 0);
	update_mirroring();
}

u8 nes_8025_device::read_l(offs_t offset)
{
	const u16 addr = offset + 0x4100;
	if (addr >= 0x5000 && !regs_enabled() && !m_prgram.empty())
		return m_prgram[(0x4000 + 0x1000 + (addr & 0x0fff)) & (m_prgram.size() - 1)];
	return get_open_bus();
}

void nes_8025_device::write_l(offs_t offset, u8 data)
{
	LOG("8025 write_l, offset: %04x, data: %02x\n", offset, data);

	const u16 addr = offset + 0x4100;

	// submapper 5: PRG base MSB at $4800-$4FFF (mask $F800)
	if (m_submapper == 5 && (addr & 0xf800) == 0x4800)
	{
		m_prg_msb = data & 0x3f;
		update_banks();
		return;
	}

	if (addr < 0x5000)
		return;

	if (!regs_enabled())
	{
		// $5000-$5FFF is the second half of WRAM bank 2 while the outer registers are disabled (FS005 copy protection)
		if (!m_prgram.empty())
			m_prgram[(0x4000 + 0x1000 + (addr & 0x0fff)) & (m_prgram.size() - 1)] = data;
		return;
	}

	// the solder pad selects which of the address lines A4-A11 must be set to access the registers
	if (!BIT(addr, 4 + (m_config.read_safe(0) & 0x07)))
		return;

	switch (addr & (m_submapper == 3 ? 0x07 : 0x03))
	{
		case 0:     // mode register
			m_mode = data;
			break;
		case 1:     // PRG base register (LSB)
			m_prg_lsb = data;
			break;
		case 2:     // CHR base register (LSB)
			m_chr_lsb = data;
			break;
		case 3:     // extended mode register (submappers 1 and 2)
			if (m_submapper != 1 && m_submapper != 2)
				return;
			m_ext = data;
			break;
		case 5:     // PRG base register (MSB, submapper 3)
			if (m_submapper != 3)
				return;
			m_prg_msb = data;
			break;
		case 6:     // CHR base register (MSB, submapper 3)
			if (m_submapper != 3)
				return;
			m_chr_msb = data;
			break;
		default:
			return;
	}
	update_banks();
}

u8 nes_8025_device::read_m(offs_t offset)
{
	if (ram_cfg())
	{
		if (!BIT(m_wram_protect, 7) || m_prgram.empty())
			return get_open_bus();
		return m_prgram[(((m_wram_protect & 0x03) * 0x2000) + offset) & (m_prgram.size() - 1)];
	}
	return nes_txrom_device::read_m(offset);
}

void nes_8025_device::write_m(offs_t offset, u8 data)
{
	LOG("8025 write_m, offset: %04x, data: %02x\n", offset, data);

	if (ram_cfg())
	{
		if (BIT(m_wram_protect, 7) && !m_prgram.empty())
			m_prgram[(((m_wram_protect & 0x03) * 0x2000) + offset) & (m_prgram.size() - 1)] = data;
		return;
	}
	nes_txrom_device::write_m(offset, data);
}

void nes_8025_device::write_h(offs_t offset, u8 data)
{
	LOG("8025 write_h, offset: %04x, data: %02x\n", offset, data);

	m_latch_data = data;    // UNROM / CNROM latch: any write to $8000-$FFFF, no bus conflicts

	// the MMC3 registers decode $E003: $x002 and $x003 (e.g. $9FFF) do nothing
	if (!BIT(offset, 1))
	{
		switch (offset & 0x6001)
		{
			case 0x0000:    // bank select
				m_latch = data;
				break;

			case 0x0001:    // bank data
			{
				const int cmd = m_latch & (ext_mode() ? 0x0f : 0x07);
				switch (cmd)
				{
					case 0: case 1: case 2: case 3: case 4: case 5:
						m_mmc_vrom_bank[cmd] = data;
						break;
					case 6: case 7: case 8: case 9:
						m_mmc_prg_bank[cmd - 6] = data;
						break;
					case 10: case 11:
						m_mmc_vrom_bank[cmd - 4] = data;
						break;
					default:
						break;
				}
				break;
			}

			case 0x2000:    // mirroring
				m_mmc_mirror = data & 0x03;
				break;

			case 0x2001:    // WRAM protect, or the RAM configuration register on submapper 2
				m_wram_protect = data;
				break;

			default:        // IRQ registers
				txrom_write(offset, data);
				break;
		}
	}
	update_banks();
}
