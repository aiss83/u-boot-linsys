/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * board.h
 *
 * LINSYS LIP-EM AM335x based board information header
 *
 * Copyright (C) 2022, Linsys Ltd, http://lin-sys.ru/
 */

#ifndef _BOARD_H_
#define _BOARD_H_

/**
 * AM335X (EMIF_4D) EMIF REG_COS_COUNT_1, REG_COS_COUNT_2, and
 * REG_PR_OLD_COUNT values to avoid LCDC DMA FIFO underflows and Frame
 * Synchronization Lost errors. The values are the biggest that work
 * reliably with offered video modes and the memory subsystem on the
 * boards. These register have are briefly documented in "7.3.3.5.2
 * Command Starvation" section of AM335x TRM. The REG_COS_COUNT_1 and
 * REG_COS_COUNT_2 do not have any effect on current versions of
 * AM335x.
 */
#define EMIF_OCP_CONFIG_AM335X_LIPEM             0x003d3d3d
#define CONFIG_MAX_RAM_BANK_SIZE	(1024 << 20)	/* 1 GiB */

/**
 * Enable UART0 pinmux mode
 */
void enable_uart0_pin_mux(void);

/**
 * I2C pinmux enabled to get connection to audio codec
 */
void enable_i2c0_pin_mux(void);

/**
 * Ethernet, NAND, SPI0 pinmuxes also required
 */
void enable_board_pin_mux(void);

#endif
