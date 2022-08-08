/*
 * mux.c
 *
 * Copyright (C) 2011 Texas Instruments Incorporated - http://www.ti.com/
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation version 2.
 *
 * This program is distributed "as is" WITHOUT ANY WARRANTY of any
 * kind, whether express or implied; without even the implied warranty
 * of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 */

#include <common.h>
#include <asm/arch/sys_proto.h>
#include <asm/arch/hardware.h>
#include <asm/arch/mux.h>
#include <asm/io.h>
#include <i2c.h>
#include "board.h"

/* UART pinmuxes */
static struct module_pin_mux uart0_pin_mux[] = {
	{OFFSET(uart0_rxd), (MODE(0) | PULLUP_EN | RXACTIVE)},	/* UART0_RXD */
	{OFFSET(uart0_txd), (MODE(0) | PULLUDEN)},		        /* UART0_TXD */
	{-1},
};

/* Keyboard UART2 module will be in future */

/* I2C pinmuxes */
static struct module_pin_mux i2c0_pin_mux[] = {
	{OFFSET(i2c0_sda), (MODE(0) | RXACTIVE | PULLUDEN | SLEWCTRL)}, /* I2C_DATA */
	{OFFSET(i2c0_scl), (MODE(0) | RXACTIVE | PULLUDEN | SLEWCTRL)}, /* I2C_SCLK */
	{-1},
};

/* LED pinmux */
static struct module_pin_mux led_pin_mux[] = {
        {OFFSET(gpmc_csn2), (MODE(7) | PULLUDEN)}, /* I2C_DATA */
        {-1},
};

static struct module_pin_mux spi0_pin_mux[] = {
	{OFFSET(spi0_sclk), (MODE(0) | RXACTIVE | PULLUDEN)},               /* SPI0_SCLK */
	{OFFSET(spi0_d0),   (MODE(0) | RXACTIVE | PULLUDEN | PULLUP_EN)},   /* SPI0_D0 */
	{OFFSET(spi0_d1),   (MODE(0) | RXACTIVE | PULLUDEN)},               /* SPI0_D1 */
	{OFFSET(spi0_cs0),  (MODE(0) | RXACTIVE | PULLUDEN | PULLUP_EN)},   /* SPI0_CS0 */
    {OFFSET(spi0_cs1),  (MODE(7) | PULLUDEN | PULLUP_EN)},              /* SPI0_CS1 - nWP (gpio0_6) */
	{-1},
};

/* Using RGMII1 */
static struct module_pin_mux rgmii1_pin_mux[] = {
    /* TX pins */
	{OFFSET(mii1_txd3),     MODE(2)},                          /* RGMII1_TD3 */
	{OFFSET(mii1_txd2),     MODE(2)},                          /* RGMII1_TD2 */
	{OFFSET(mii1_txd1),     MODE(2)},                          /* RGMII1_TD1 */
	{OFFSET(mii1_txd0),     MODE(2)},                          /* RGMII1_TD0 */
	{OFFSET(mii1_txclk),    MODE(2)},                          /* RGMII1_TCLK */
    {OFFSET(mii1_txen),     MODE(2)},                          /* RGMII1_TCTL/EN */
    /* RX pins */
	{OFFSET(mii1_rxclk),    MODE(2) | RXACTIVE},               /* RGMII1_RCLK */
	{OFFSET(mii1_rxd3),     MODE(2) | RXACTIVE},               /* RGMII1_RD3 */
	{OFFSET(mii1_rxd2),     MODE(2) | RXACTIVE},               /* RGMII1_RD2 */
	{OFFSET(mii1_rxd1),     MODE(2) | RXACTIVE},               /* RGMII1_RD1 */
	{OFFSET(mii1_rxd0),     MODE(2) | RXACTIVE},               /* RGMII1_RD0 */
    {OFFSET(mii1_rxdv),     MODE(2) | RXACTIVE},               /* RGMII1_RCTL */
    /* MDIO */
	{OFFSET(mdio_data),     MODE(0) | RXACTIVE | PULLUP_EN},   /* MDIO_DATA */
	{OFFSET(mdio_clk),      MODE(0) | PULLUP_EN},              /* MDIO_CLK */
    /* For Debug purposes */
    {OFFSET(rmii1_refclk),  MODE(0) | PULLUDEN},               /* RMII1_REF_CLK for debug purpose */
	/* PHY reset PIN */
	{OFFSET(usb1_drvvbus),  MODE(7) | PULLUDEN | PULLUP_EN},   /* Ethernet PHY reset pin with pull up mode */
	{-1},
};

static struct module_pin_mux ampsd_pin_mux[] = {
    {OFFSET(usb0_drvvbus),  MODE(7) | PULLDOWN_EN},            /* Audio amplifier reset */
};

static struct module_pin_mux nand_pin_mux[] = {
	{OFFSET(gpmc_ad0),	    (MODE(0) | PULLUDDIS | RXACTIVE)}, /* AD0  */
	{OFFSET(gpmc_ad1),	    (MODE(0) | PULLUDDIS | RXACTIVE)}, /* AD1  */
	{OFFSET(gpmc_ad2),	    (MODE(0) | PULLUDDIS | RXACTIVE)}, /* AD2  */
	{OFFSET(gpmc_ad3),	    (MODE(0) | PULLUDDIS | RXACTIVE)}, /* AD3  */
	{OFFSET(gpmc_ad4),	    (MODE(0) | PULLUDDIS | RXACTIVE)}, /* AD4  */
	{OFFSET(gpmc_ad5),	    (MODE(0) | PULLUDDIS | RXACTIVE)}, /* AD5  */
	{OFFSET(gpmc_ad6),	    (MODE(0) | PULLUDDIS | RXACTIVE)}, /* AD6  */
	{OFFSET(gpmc_ad7),	    (MODE(0) | PULLUDDIS | RXACTIVE)}, /* AD7  */
	{OFFSET(gpmc_wait0),    (MODE(0) | PULLUP_EN | RXACTIVE)}, /* nWAIT */
	{OFFSET(gpmc_wpn),	    (MODE(7) | PULLUP_EN)},		       /* nWP */
	{OFFSET(gpmc_csn0),	    (MODE(0) | PULLUP_EN)},		       /* nCS */
	{OFFSET(gpmc_wen),	    (MODE(0) | PULLDOWN_EN)},	       /* WEN */
	{OFFSET(gpmc_oen_ren),	(MODE(0) | PULLDOWN_EN)},	       /* OE */
	{OFFSET(gpmc_advn_ale),	(MODE(0) | PULLDOWN_EN)},	       /* ADV_ALE */
	{OFFSET(gpmc_be0n_cle),	(MODE(0) | PULLDOWN_EN)},	       /* BE_CLE */
	{-1},
};

void enable_uart0_pin_mux(void)
{
	configure_module_pin_mux(uart0_pin_mux);
}

void enable_i2c0_pin_mux(void)
{
	configure_module_pin_mux(i2c0_pin_mux);
}

void enable_board_pin_mux(void)
{
    /* Enable LED */
    configure_module_pin_mux(led_pin_mux);

    /* Enable NAND pinmux */
    configure_module_pin_mux(nand_pin_mux);

    /* Enable SPI pinmux */
    configure_module_pin_mux(spi0_pin_mux);

    /* Enable ethernet pinmux */
    configure_module_pin_mux(rgmii1_pin_mux);

    configure_module_pin_mux(ampsd_pin_mux);
}
