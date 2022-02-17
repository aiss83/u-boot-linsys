// SPDX-License-Identifier: GPL-2.0+
/*
 * board.c
 *
 * LIP-EM board functions for TI AM335X based boards
 *
 * Copyright (C) 2022, Linsys Ltd, http://lin-sys.ru/
 */

#include <common.h>
#include <dm.h>
#include <env.h>
#include <errno.h>
#include <init.h>
#include <malloc.h>
#include <spl.h>
#include <serial.h>
#include <asm/arch/cpu.h>
#include <asm/arch/hardware.h>
#include <asm/arch/omap.h>
#include <asm/arch/ddr_defs.h>
#include <asm/arch/clock.h>
#include <asm/arch/clk_synthesizer.h>
#include <asm/arch/gpio.h>
#include <asm/arch/sys_proto.h>
#include <asm/arch/mem.h>
#include <asm/io.h>
#include <asm/emif.h>
#include <asm/gpio.h>
#include <asm/omap_common.h>
#include <asm/omap_sec_common.h>
#include <linux/types.h>
#include <i2c.h>
#include <miiphy.h>
#include <cpsw.h>
#include <env_internal.h>
#include <watchdog.h>
#include "board.h"

DECLARE_GLOBAL_DATA_PTR;

/* GPIO that controls power to DDR on LIP-EM */
#define GPIO_TO_PIN(bank, gpio)		(32 * (bank) + (gpio))
#define GPIO_DDR_VTT_EN		GPIO_TO_PIN(0, 7)
#define ICE_GPIO_DDR_VTT_EN	GPIO_TO_PIN(0, 18)
#define GPIO_PR1_MII_CTRL	GPIO_TO_PIN(3, 4)
#define GPIO_MUX_MII_CTRL	GPIO_TO_PIN(3, 10)
#define GPIO_FET_SWITCH_CTRL	GPIO_TO_PIN(0, 7)
#define GPIO_PHY_RESET		GPIO_TO_PIN(2, 5)
#define GPIO_ETH0_MODE		GPIO_TO_PIN(0, 11)
#define GPIO_ETH1_MODE		GPIO_TO_PIN(1, 26)

# define LED1_GPIO   GPIO_TO_PIN(1, 31)

static struct ctrl_dev *cdev = (struct ctrl_dev *)CTRL_DEVICE_BASE;

#define GPIO0_RISINGDETECT	(AM33XX_GPIO0_BASE + OMAP_GPIO_RISINGDETECT)
#define GPIO1_RISINGDETECT	(AM33XX_GPIO1_BASE + OMAP_GPIO_RISINGDETECT)

#define GPIO0_IRQSTATUS1	(AM33XX_GPIO0_BASE + OMAP_GPIO_IRQSTATUS1)
#define GPIO1_IRQSTATUS1	(AM33XX_GPIO1_BASE + OMAP_GPIO_IRQSTATUS1)

#define GPIO0_IRQSTATUSRAW	(AM33XX_GPIO0_BASE + 0x024)
#define GPIO1_IRQSTATUSRAW	(AM33XX_GPIO1_BASE + 0x024)

#ifndef CONFIG_SKIP_LOWLEVEL_INIT
static const struct ddr_data ddr2_data = {
	.datardsratio0 = MT47H64M16NF25E_RD_DQS,
	.datafwsratio0 = MT47H64M16NF25E_PHY_FIFO_WE,
	.datawrsratio0 = MT47H64M16NF25E_PHY_WR_DATA,
};

static const struct cmd_control ddr2_cmd_ctrl_data = {
	.cmd0csratio = MT47H64M16NF25E_RATIO,

	.cmd1csratio = MT47H64M16NF25E_RATIO,

	.cmd2csratio = MT47H64M16NF25E_RATIO,
};

#ifdef LIPEM_DONT_USE_OCP
static const struct emif_regs ddr2_emif_reg_data = {
	.sdram_config = MT47H64M16NF25E_EMIF_SDCFG,
	.ref_ctrl = MT47H64M16NF25E_EMIF_SDREF,
	.sdram_tim1 = MT47H64M16NF25E_EMIF_TIM1,
	.sdram_tim2 = MT47H64M16NF25E_EMIF_TIM2,
	.sdram_tim3 = MT47H64M16NF25E_EMIF_TIM3,
	.emif_ddr_phy_ctlr_1 = MT47H64M16NF25E_EMIF_READ_LATENCY,
};
#else
static const struct emif_regs ddr2_lipem_emif_reg_data = {
	.sdram_config = MT47H64M16NF25E_EMIF_SDCFG,
	.ref_ctrl = MT47H64M16NF25E_EMIF_SDREF,
	.sdram_tim1 = MT47H64M16NF25E_EMIF_TIM1,
	.sdram_tim2 = MT47H64M16NF25E_EMIF_TIM2,
	.sdram_tim3 = MT47H64M16NF25E_EMIF_TIM3,
	.ocp_config = EMIF_OCP_CONFIG_AM335X_LIPEM,
	.emif_ddr_phy_ctlr_1 = MT47H64M16NF25E_EMIF_READ_LATENCY,
};
#endif

#ifdef CONFIG_SPL_OS_BOOT
int spl_start_uboot(void)
{
#ifdef CONFIG_SPL_SERIAL_SUPPORT
	/* break into full u-boot on 'c' */
	if (serial_tstc() && serial_getc() == 'c')
		return 1;
#endif

#ifdef CONFIG_SPL_ENV_SUPPORT
	env_init();
	env_load();
	if (env_get_yesno("boot_os") != 1)
		return 1;
#endif

	return 0;
}
#endif

const struct dpll_params *get_dpll_ddr_params(void)
{
	int ind = get_sys_clk_index();

	return &dpll_ddr2_266MHz[ind];
}

const struct dpll_params *get_dpll_mpu_params(void)
{
	int ind = get_sys_clk_index();
	int freq = am335x_get_efuse_mpu_max_freq(cdev);

	switch (freq) {
		case MPUPLL_M_800:
			return &dpll_mpu_opp[ind][4];
		case MPUPLL_M_720:
			return &dpll_mpu_opp[ind][3];
		case MPUPLL_M_600:
			return &dpll_mpu_opp[ind][2];
		case MPUPLL_M_500:
			return &dpll_mpu_opp100;
		case MPUPLL_M_300:
			return &dpll_mpu_opp[ind][0];
	}

	return &dpll_mpu_opp[ind][0];
}

void set_uart_mux_conf(void)
{
	enable_uart0_pin_mux();
}

void set_mux_conf_regs(void)
{
	enable_board_pin_mux();
}

const struct ctrl_ioregs ioregs = {
	.cm0ioctl		= MT47H64M16NF25E_IOCTRL_VALUE,
	.cm1ioctl		= MT47H64M16NF25E_IOCTRL_VALUE,
	.cm2ioctl		= MT47H64M16NF25E_IOCTRL_VALUE,
	.dt0ioctl		= MT47H64M16NF25E_IOCTRL_VALUE,
	.dt1ioctl		= MT47H64M16NF25E_IOCTRL_VALUE,
};

/* Watchdog initialize dumb */
void hw_watchdog_init(void) {
    return;
}

void sdram_init(void)
{
	config_ddr(266, &ioregs, &ddr2_data, &ddr2_cmd_ctrl_data, &ddr2_lipem_emif_reg_data, 0);
	/*
		config_ddr(266, &ioregs, &ddr2_data,
			   &ddr2_cmd_ctrl_data, &ddr2_emif_reg_data, 0);
			   */
}
#endif

/*
 * Basic board specific setup.  Pinmux has been handled already.
 */
int board_init(void)
{
#if defined(CONFIG_HW_WATCHDOG)
	hw_watchdog_init();
#endif

	gd->bd->bi_boot_params = CONFIG_SYS_SDRAM_BASE + 0x100;
#if defined(CONFIG_NOR) || defined(CONFIG_MTD_RAW_NAND)
	gpmc_init();
#endif

	return 0;
}

#ifdef CONFIG_BOARD_LATE_INIT
int board_late_init(void)
{
	struct udevice *dev;
#if !defined(CONFIG_SPL_BUILD)
	uint8_t mac_addr[6];
	uint32_t mac_hi, mac_lo;
#endif

#if !defined(CONFIG_SPL_BUILD)
	/* try reading mac address from efuse */
	mac_lo = readl(&cdev->macid0l);
	mac_hi = readl(&cdev->macid0h);
	mac_addr[0] = mac_hi & 0xFF;
	mac_addr[1] = (mac_hi & 0xFF00) >> 8;
	mac_addr[2] = (mac_hi & 0xFF0000) >> 16;
	mac_addr[3] = (mac_hi & 0xFF000000) >> 24;
	mac_addr[4] = mac_lo & 0xFF;
	mac_addr[5] = (mac_lo & 0xFF00) >> 8;

	if (!env_get("ethaddr")) {
		printf("<ethaddr> not set. Validating first E-fuse MAC\n");

		if (is_valid_ethaddr(mac_addr))
			eth_env_set_enetaddr("ethaddr", mac_addr);
	}

	mac_lo = readl(&cdev->macid1l);
	mac_hi = readl(&cdev->macid1h);
	mac_addr[0] = mac_hi & 0xFF;
	mac_addr[1] = (mac_hi & 0xFF00) >> 8;
	mac_addr[2] = (mac_hi & 0xFF0000) >> 16;
	mac_addr[3] = (mac_hi & 0xFF000000) >> 24;
	mac_addr[4] = mac_lo & 0xFF;
	mac_addr[5] = (mac_lo & 0xFF00) >> 8;

	if (!env_get("eth1addr")) {
		if (is_valid_ethaddr(mac_addr))
			eth_env_set_enetaddr("eth1addr", mac_addr);
	}
#endif

	if (!env_get("serial#")) {
		char *board_serial = env_get("board_serial");
		char *ethaddr = env_get("ethaddr");

		if (!board_serial || !strncmp(board_serial, "unknown", 7))
			env_set("serial#", ethaddr);
		else
			env_set("serial#", board_serial);
	}

	return 0;
}
#endif
#ifdef ENABLE_CPSW
/* CPSW platdata */
#if !CONFIG_IS_ENABLED(OF_CONTROL)
struct cpsw_slave_data slave_data[] = {
	{
		.slave_reg_ofs  = CPSW_SLAVE0_OFFSET,
		.sliver_reg_ofs = CPSW_SLIVER0_OFFSET,
		.phy_addr       = 0,
	},
	{
		.slave_reg_ofs  = CPSW_SLAVE1_OFFSET,
		.sliver_reg_ofs = CPSW_SLIVER1_OFFSET,
		.phy_addr       = 1,
	},
};

struct cpsw_platform_data am335_eth_data = {
	.cpsw_base		= CPSW_BASE,
	.version		= CPSW_CTRL_VERSION_2,
	.bd_ram_ofs		= CPSW_BD_OFFSET,
	.ale_reg_ofs		= CPSW_ALE_OFFSET,
	.cpdma_reg_ofs		= CPSW_CPDMA_OFFSET,
	.mdio_div		= CPSW_MDIO_DIV,
	.host_port_reg_ofs	= CPSW_HOST_PORT_OFFSET,
	.channels		= 8,
	.slaves			= 2,
	.slave_data		= slave_data,
	.ale_entries		= 1024,
	.bd_ram_ofs		= 0x2000,
	.mac_control		= 0x20,
	.active_slave		= 0,
	.mdio_base		= 0x4a101000,
	.gmii_sel		= 0x44e10650,
	.phy_sel_compat		= "ti,am3352-cpsw-phy-sel",
	.syscon_addr		= 0x44e10630,
	.macid_sel_compat	= "cpsw,am33xx",
};

struct eth_pdata cpsw_pdata = {
	.iobase = 0x4a100000,
	.phy_interface = 0,
	.priv_pdata = &am335_eth_data,
};

U_BOOT_DEVICE(am335x_eth) = {
	.name = "eth_cpsw",
	.platdata = &cpsw_pdata,
};
#endif
#endif

#ifdef CONFIG_SPL_LOAD_FIT
int board_fit_config_name_match(const char *name)
{
	return -1;
}
#endif

#define CONFIG_LIPEM_SDRAM_TEST	
// #define SYS_STR_RAM_ADDR	0x80000000
#define SYS_STR_RAM_ADDR	0x82000000


#if defined(CONFIG_LIPEM_SDRAM_TEST)
uint32_t test_ram()
{
	volatile uint32_t* start = (uint32_t*)SYS_STR_RAM_ADDR;
	volatile uint32_t* end   = (uint32_t*)(SYS_STR_RAM_ADDR + (16 * 1024 * 1024)); // 8 MB SDRAM

	//test data bus

	puts("Checking DATA bus\r\n");
	for (int j = 0; j < 32; j++)
	{
		//writing single bit, and checking than 1 and only 1 bit is read back
		*((uint32_t*) start) = (1 << j);
		uint32_t val = *((uint32_t*) start);

		if (*((uint32_t*) start) != (1 << j))
		{
			printf("DB ERROR: on bit %d got back 0x%08x\n\r", j, *((uint32_t*) start));

			return 0;
		}

	}

	volatile uint32_t* max = end;

	//filling ram cels whith it own addreses
	printf("Filling RAM: 0x%x 0x%x\r\n", start, max);
	volatile uint32_t* i;
	for (i = start; (i < (uint32_t*) max); i++)
	{
		*i = (uint32_t) i;
		// if ((uint32_t)i % 1024 == 0) {printf("0x%x\n", i);}
		if (*i != (uint32_t) i)
		{
			printf("Memory fill failed at address %08x \r\n", i);
			return 0;
		}
	}

	puts("Checking usable RAM size\r\n");
	int err=0, stop=0;
	volatile uint32_t* loop=0, oldMax = max;
	printf("Old max 0x%x\n", oldMax);
	while (!stop)
	{
		//re-filling RAM if nececery (second and later pass)
		if (max != end)
		{
			oldMax = max;
			for (i = (volatile uint32_t*) start; i < (volatile uint32_t*) max;
					i++)
			{
				*i = (volatile uint32_t) i;
				//if written wrong value - emergency exit as memory not reliable
				if (*i != (volatile uint32_t) i)
				{
					max = start;
					puts("refill write error\n\r");
					return 0;
				}

			}
		}


		for (i = start;	((i < max) && (err == 0)); i++)
		{
			if (*i != (volatile uint32_t) i)
			{
				//here will be overlapped address
				//if we don't get same address as in previous cycle
				if (max != (volatile uint32_t*) *i)
				{
					printf("Shift detected on %08x to %08x\n\r", (uint32_t)i, *i);
					if ((volatile uint32_t*) i != start)
					{ //overlap start not on boundary address - possible address bus fail
						//determining rouge address bits
						uint32_t rouge = *i - (uint32_t) start;
						uint32_t valid = (uint32_t) i - (uint32_t)start;

						puts("Bad address bus\n\r");
					} else
					{ //definite address overlap
						max = (volatile uint32_t*) *i;
					}
				} else
				{
					//if we got the same - memory is really very bad
					max = start; //will result 0 capacity size
				}
				err = 1;
			}

			if (err)
			{
				//determining loop size
				loop = ((uint32_t)oldMax - (uint32_t)max);
				max = ((uint32_t)start + (uint32_t)loop);
			}
		}

		if (err)
		{
			stop = 0;
			err = 0;
		} else
		{
			stop = 1;
		}
	}

	//calculating good ram size
	printf("Start: 0x%x End: 0x%x\n", start, max);
	uint32_t result = 0;
	if (max > start)
	{
		result = (uint32_t) max - (uint32_t) start; //we can't get end before start
	} else
	{
		result = 0;
	}

	printf("Effective RAM size %ld bytes\r\n", result);

	return result;
}

#endif
