/* SPDX-License-Identifier: GPL-2.0-only */
/* Copyright (C) 2020 Texas Instruments Incorporated - http://www.ti.com/ */
/* Copyright (C) 2021 XiaoMi, Inc. */

#include <linux/bitops.h>

#define BQ25980_MANUFACTURER "Texas Instruments"

#define BQ25980_BATOVP			0x0
#define BQ25980_BATOVP_ALM		0x1
#define BQ25980_BATOCP			0x2
#define BQ25980_BATOCP_ALM		0x3
#define BQ25980_CHRGR_CFG_1		0x4
#define BQ25980_CHRGR_CTRL_1		0x5
#define BQ25980_BUSOVP			0x6
#define BQ25980_BUSOVP_ALM		0x7
#define BQ25980_BUSOCP			0x8
#define BQ25980_REG_09			0x9
#define BQ25980_TEMP_CONTROL		0xA
#define BQ25980_TDIE_ALM		0xB
#define BQ25980_TSBUS_FLT		0xC
#define BQ25980_TSBAT_FLG		0xD
#define BQ25980_VAC_CONTROL		0xE
#define BQ25980_CHRGR_CTRL_2		0xF
#define BQ25980_CHRGR_CTRL_3		0x10
#define BQ25980_CHRGR_CTRL_4		0x11
#define BQ25980_CHRGR_CTRL_5		0x12
#define BQ25980_STAT1			0x13
#define BQ25980_STAT2			0x14
#define BQ25980_STAT3			0x15
#define BQ25980_STAT4			0x16
#define BQ25980_STAT5			0x17
#define BQ25980_FLAG1			0x18
#define BQ25980_FLAG2			0x19
#define BQ25980_FLAG3			0x1A
#define BQ25980_FLAG4			0x1B
#define BQ25980_FLAG5			0x1C
#define BQ25980_MASK1			0x1D
#define BQ25980_MASK2			0x1E
#define BQ25980_MASK3			0x1F
#define BQ25980_MASK4			0x20
#define BQ25980_MASK5			0x21
#define BQ25980_DEVICE_INFO		0x22
#define BQ25980_ADC_CONTROL1		0x23
#define BQ25980_ADC_CONTROL2		0x24
#define BQ25980_IBUS_ADC_MSB		0x25
#define BQ25980_IBUS_ADC_LSB		0x26
#define BQ25980_VBUS_ADC_MSB		0x27
#define BQ25980_VBUS_ADC_LSB		0x28
#define BQ25980_VAC1_ADC_MSB		0x29
#define BQ25980_VAC1_ADC_LSB		0x2A
#define BQ25980_VAC2_ADC_MSB		0x2B
#define BQ25980_VAC2_ADC_LSB		0x2C
#define BQ25980_VOUT_ADC_MSB		0x2D
#define BQ25980_VOUT_ADC_LSB		0x2E
#define BQ25980_VBAT_ADC_MSB		0x2F
#define BQ25980_VBAT_ADC_LSB		0x30
#define BQ25980_IBAT_ADC_MSB		0x31
#define BQ25980_IBAT_ADC_LSB		0x32
#define BQ25980_TSBUS_ADC_MSB		0x33
#define BQ25980_TSBUS_ADC_LSB		0x34
#define BQ25980_TSBAT_ADC_MSB		0x35
#define BQ25980_TSBAT_ADC_LSB		0x36
#define BQ25980_TDIE_ADC_MSB		0x37
#define BQ25980_TDIE_ADC_LSB		0x38
#define BQ25980_DEGLITCH_TIME		0x39
#define BQ25980_CHRGR_CTRL_6		0x3A
#define CPCHIPX_INTERNAL_1		0x40
#define CPCHIPX_POR_FLAG			BIT(7)

#define BQ25980_BUSOCP_STEP_uA		262500
#define BQ25980_BUSOCP_OFFSET_uA	1050000
#define BQ25960_BUSOCP_STEP_uA		250000
#define BQ25960_BUSOCP_OFFSET_uA	1000000

#define BQ25980_DEV_ID_MASK				0x0F
#define BQ25980_DEV_ID_SHIFT			0

#define BQ25980_DEVICE_ID				0x12
#define BQ25970_DEVICE_ID				0x10
#define CPCHIPX_DEVICE_ID				0x41


#define BQ25980_BUSOCP_SC_DFLT_uA	4200000
#define BQ25980_BUSOCP_BYP_DFLT_uA	8000000
#define BQ25975_BUSOCP_DFLT_uA		4250000
#define BQ25960_BUSOCP_SC_DFLT_uA	4000000
#define BQ25960_BUSOCP_BYP_DFLT_uA	5000000

#define BQ25980_BUSOCP_MIN_uA		1050000
#define BQ25960_BUSOCP_MIN_uA		1000000

#define BQ25980_BUSOCP_SC_MAX_uA	6037500
#define BQ25975_BUSOCP_SC_MAX_uA	5750000
#define BQ25960_BUSOCP_SC_MAX_uA	4500000

#define BQ25980_BUSOCP_BYP_MAX_uA	8925000
#define BQ25975_BUSOCP_BYP_MAX_uA	8500000
#define BQ25960_BUSOCP_BYP_MAX_uA	6500000

#define BQ25980_BUSOVP_SC_STEP_uV	100000
#define BQ25975_BUSOVP_SC_STEP_uV	50000
#define BQ25960_BUSOVP_SC_STEP_uV	50000
#define BQ25980_BUSOVP_SC_OFFSET_uV	14000000
#define BQ25975_BUSOVP_SC_OFFSET_uV	7000000
#define BQ25960_BUSOVP_SC_OFFSET_uV	7000000

#define BQ25980_BUSOVP_BYP_STEP_uV	50000
#define BQ25975_BUSOVP_BYP_STEP_uV	25000
#define BQ25960_BUSOVP_BYP_STEP_uV	25000
#define BQ25980_BUSOVP_BYP_OFFSET_uV	7000000
#define BQ25975_BUSOVP_BYP_OFFSET_uV	3500000
#define BQ25960_BUSOVP_BYP_OFFSET_uV	3500000

#define BQ25980_BUSOVP_DFLT_uV		17800000
#define BQ25980_BUSOVP_BYPASS_DFLT_uV	8900000
#define BQ25975_BUSOVP_DFLT_uV		8900000
#define BQ25975_BUSOVP_BYPASS_DFLT_uV	4450000
#define BQ25960_BUSOVP_DFLT_uV		8900000

#define BQ25980_BUSOVP_SC_MIN_uV	14000000
#define BQ25975_BUSOVP_SC_MIN_uV	7000000
#define BQ25960_BUSOVP_SC_MIN_uV	7000000
#define BQ25980_BUSOVP_BYP_MIN_uV	7000000
#define BQ25975_BUSOVP_BYP_MIN_uV	3500000
#define BQ25960_BUSOVP_BYP_MIN_uV	3500000

#define BQ25980_BUSOVP_SC_MAX_uV	22000000
#define BQ25975_BUSOVP_SC_MAX_uV	12750000
#define BQ25960_BUSOVP_SC_MAX_uV	12750000

#define BQ25980_BUSOVP_BYP_MAX_uV	12750000
#define BQ25975_BUSOVP_BYP_MAX_uV	6500000
#define BQ25960_BUSOVP_BYP_MAX_uV	6500000

#define BQ25980_AC_OVP_6500MV		0x00
#define BQ25980_AC_OVP_10500MV		0x09
#define BQ25980_AC_OVP_12000MV		0x12
#define BQ25980_AC_OVP_14000MV		0x1B
#define BQ25980_AC_OVP_16000MV		0x24
#define BQ25980_AC_OVP_18000MV		0x2D
#define BQ25980_AC_OVP_22000MV		0x36
#define BQ25980_AC_OVP_24000MV		0x3F
#define BQ25980_AC_OVP_SHIFT		2
#define BQ25980_AC_OVP_MASK		GENMASK(7, 2)

#define BQ25980_BATOVP_STEP_uV		20000
#define BQ25975_BATOVP_STEP_uV		10000
#define BQ25960_BATOVP_STEP_uV		10000

#define BQ25980_BATOVP_OFFSET_uV	7000000
#define BQ25975_BATOVP_OFFSET_uV	3500000
#define BQ25960_BATOVP_OFFSET_uV	3500000

#define BQ25980_BATOVP_DFLT_uV		14000000
#define BQ25975_BATOVP_DFLT_uV		8900000
#define BQ25960_BATOVP_DFLT_uV		8900000

#define BQ25980_BATOVP_MIN_uV		7000000
#define BQ25975_BATOVP_MIN_uV		3500000
#define BQ25960_BATOVP_MIN_uV		3500000

#define BQ25980_BATOVP_MAX_uV		9540000
#define BQ25975_BATOVP_MAX_uV		4770000
#define BQ25960_BATOVP_MAX_uV		4770000

#define BQ25980_BATOCP_STEP_uA		100000

#define BQ25980_BATOCP_MASK		GENMASK(6, 0)

#define BQ25980_BATOCP_DFLT_uA		8100000
#define BQ25960_BATOCP_DFLT_uA		6100000

#define BQ25980_BATOCP_MIN_uA		2000000

#define BQ25980_BATOCP_MAX_uA		11000000
#define BQ25975_BATOCP_MAX_uA		11000000
#define BQ25960_BATOCP_MAX_uA		8500000

#define BQ25980_ENABLE_HIZ		0xff
#define BQ25980_DISABLE_HIZ		0x0
#define BQ25980_EN_BYPASS		BIT(3)
#define BQ25980_STAT1_OVP_MASK		(BIT(7) | BIT(5) | BIT(1))
#define BQ25980_STAT3_OVP_MASK		(BIT(7) | BIT(6))
#define BQ25980_STAT1_OCP_MASK		BIT(4)
#define BQ25980_STAT2_OCP_MASK		BIT(7)
#define BQ25980_STAT4_TFLT_MASK		GENMASK(4, 2)

#define BQ25980_WD_STAT			BIT(0)
#define BQ25980_PRESENT_MASK		GENMASK(4, 2)
#define BQ25980_VBAT_PRESENT_MASK	BIT(5)
#define BQ25980_VBUS_PRESENT_MASK	BIT(2)
#define BQ25980_ACDRV2_EN		BIT(0)
#define BQ25980_ACDRV1_EN		BIT(1)
#define BQ25980_DIS_ACDRV_BOTH		BIT(2)
#define BQ25980_EN_OTG			BIT(5)
#define BQ25980_CHG_EN			BIT(4)
#define BQ25980_EN_HIZ			BIT(6)
#define BQ25980_ADC_EN			BIT(7)
#define BQ25980_MS_MASK			GENMASK(1, 0)
#define BQ25980_DEVICE_ID_MASK		GENMASK(3, 0)

#define BQ25980_ADC_CURR_STEP_IBUS_uA		1070
#define CPCHIPX_ADC_CURR_STEP_IBUS_uA		2500

#define BQ25980_ADC_VOLT_STEP_VBAT_deciuV	10055
#define CPCHIPX_ADC_VOLT_STEP_VBAT_deciuV	25000

#define BQ25980_ADC_VOLT_STEP_VBUS_deciuV	10066
#define CPCHIPX_ADC_VOLT_STEP_VBUS_deciuV	62500

#define BQ25980_ADC_VOLT_OFFSET_VBUS		-60000
#define BQ25960_ADC_VOLT_STEP_deciuV		10000
#define BQ25960_ADC_VOLT_STEP_uV		1000
#define BQ25960_ADC_CURR_STEP_uA		1000
#define BQ25980_ADC_TCHG_STEP			5
#define BQ25980_ADC_POLARITY_BIT	        BIT(7)

#define BQ25980_WATCHDOG_MASK	        GENMASK(4, 3)
#define BQ25980_WATCHDOG_DIS	        BIT(2)
#define BQ25980_WATCHDOG_MAX	        300000
#define BQ25980_WATCHDOG_MIN	        0
#define BQ25980_NUM_WD_VAL	        4

#define BQ25980_VAC1_CONTROL_MASK	GENMASK(7, 5)
#define BQ25980_VAC1_CONTROL_SHIFT	5
#define BQ25980_VAC2_CONTROL_MASK	GENMASK(4, 2)
#define BQ25980_VAC2_CONTROL_SHIFT	2

#define BQ25980_STAT1_BAT_OVP_MASK	        BIT(7)
#define BQ25980_STAT1_BAT_OVP_ALM_MASK	        BIT(6)
#define BQ25980_STAT1_BAT_OCP_MASK	        BIT(4)
#define BQ25980_STAT1_BAT_OCP_ALM_MASK	        BIT(3)
#define BQ25980_STAT1_BUS_OVP_MASK	        BIT(1)
#define BQ25980_STAT1_BUS_OVP_ALM_MASK  	BIT(0)
#define BQ25980_STAT2_BUS_OCP_MASK	        BIT(7)
#define BQ25980_STAT2_BUS_OCP_ALM_MASK  	BIT(6)
#define BQ25980_STAT4_TSBAT_FLT_MASK	        BIT(3)
#define BQ25980_STAT4_TSBUS_TSBAT_ALM_MASK	BIT(5)
#define BQ25980_STAT4_TDIE_ALM_MASK	        BIT(1)

#define TS_BAT_FAULT		BIT(4)
#define TS_BUS_FAULT		BIT(3)
#define TS_DIE_FAULT		BIT(2)
#define BAT_THERM_FAULT_SHIFT	4
#define BUS_THERM_FAULT_SHIFT	5
#define DIE_THERM_FAULT_SHIFT	6

#define VBUS_ERROR_HI		BIT(4)
