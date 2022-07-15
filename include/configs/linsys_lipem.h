/*
 * am335x_evm.h
 *
 * Copyright (C) 2011 Texas Instruments Incorporated - http://www.ti.com/
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation version 2.
 *
 * This program is distributed "as is" WITHOUT ANY WARRANTY of any
 * kind, whether express or implied; without even the implied warranty
 * of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#ifndef __CONFIG_LINSYS_LIPEM_H
#define __CONFIG_LINSYS_LIPEM_H

#include <configs/ti_am335x_common.h>
#include <linux/sizes.h>

#ifndef CONFIG_SPL_BUILD
# define CONFIG_TIMESTAMP
#endif

#define CONFIG_SYS_BOOTM_LEN		SZ_16M

//#define CONFIG_MACH_TYPE		MACH_TYPE_AM335XEVM

/* Clock Defines */
#define V_OSCK				25000000  /* Clock output from T2 */
#define V_SCLK				(V_OSCK)

#ifdef CONFIG_MTD_RAW_NAND
#define NANDARGS \
	"mtdids=" CONFIG_MTDIDS_DEFAULT "\0" \
	"mtdparts=" CONFIG_MTDPARTS_DEFAULT "\0" \
	"nandargs=setenv bootargs console=${console} " \
		"${optargs} " \
		"root=${nandroot} " \
		"rootfstype=${nandrootfstype}\0" \
	"nandroot=ubi0:rootfs rw ubi.mtd=NAND.file-system,2048\0" \
	"nandrootfstype=ubifs rootwait=1\0" \
	"nandboot=echo Booting from nand ...; " \
		"run nandargs; " \
		"nand read ${fdtaddr} NAND.u-boot-spl-os; " \
		"nand read ${loadaddr} NAND.kernel; " \
		"bootz ${loadaddr} - ${fdtaddr}\0"
#else
#define NANDARGS ""
#endif

#define BOOTENV_DEV_NAND(devtypeu, devtypel, instance) \
	"bootcmd_" #devtypel "=" \
	"run nandboot\0"

#define BOOTENV_DEV_NAME_NAND(devtypeu, devtypel, instance) \
	#devtypel #instance " "

#if CONFIG_IS_ENABLED(CMD_PXE)
# define BOOT_TARGET_PXE(func) func(PXE, pxe, na)
#else
# define BOOT_TARGET_PXE(func)
#endif

#if CONFIG_IS_ENABLED(CMD_DHCP)
# define BOOT_TARGET_DHCP(func) func(DHCP, dhcp, na)
#else
# define BOOT_TARGET_DHCP(func)
#endif

#define BOOT_TARGET_DEVICES(func) \
	func(NAND, nand, 0) \
	BOOT_TARGET_PXE(func) \
	BOOT_TARGET_DHCP(func)

#include <config_distro_bootcmd.h>

#ifndef CONFIG_SPL_BUILD

#define CONFIG_EXTRA_ENV_SETTINGS \
	"loadaddr=0x82000000\0" \
	"fdtaddr=0x87010000\0" \
	"scriptaddr=0x80000000\0" \
	"bootm_size=0x8000000\0" \
	"boot_fdt=try\0" \
	"bootpart=0:2\0" \
	"bootdir=/boot\0" \
	"bootfile=zImage\0" \
	"fdtfile=undefined\0" \
	"console=ttyO0,115200n8\0" \
	"optargs=\0" \
	"spisrcaddr=0xf0000\0" \
    "spifdtaddr=0xe0000\0" \
	"spiimgsize=0x600000\0" \
    "fdtimgsize=0x10000\0" \
	"spibusno=0\0" \
	"spiboot=echo Booting from spi ...; " \
		"sf probe ${spibusno}:0; "      \
        "sf read ${fdtaddr} ${spifdtaddr} ${fdtimgsize};" \
		"sf read ${loadaddr} ${spisrcaddr} ${spiimgsize}; " \
		"bootz ${loadaddr}\0" \
	"init_console=" \
			"setenv console ttyO0,115200n8;" \
	NANDARGS \
	BOOTENV
#endif

/* NS16550 Configuration */
#if !defined(CONFIG_SYS_NS16550_COM1)
#define CONFIG_SYS_NS16550_COM1		    0x44e09000	/* Base EVM has UART0 */
#endif

/* SPL */
#if 0
#ifndef CONFIG_NOR_BOOT
/* Bootcount using the RTC block */
#define CONFIG_SYS_BOOTCOUNT_BE

/* USB gadget RNDIS */
#endif
#endif /* No bootcount at all */

#ifdef CONFIG_MTD_RAW_NAND
/* NAND: device related configs */
#define CONFIG_SYS_NAND_5_ADDR_CYCLE		/* MT29F2G08ABAGAH4 has 5 cycle address */
#define CONFIG_SYS_NAND_PAGE_COUNT	(CONFIG_SYS_NAND_BLOCK_SIZE / \
					 CONFIG_SYS_NAND_PAGE_SIZE)
#define CONFIG_SYS_NAND_PAGE_SIZE	2048		/* 2K MT29F2G08ABAGAH4-IT */
#define CONFIG_SYS_NAND_OOBSIZE		128			/* 128B MT29F2G08ABAGAH4-IT */
#define CONFIG_SYS_NAND_BLOCK_SIZE	(64*2048)	/* 64 Pages MT29F2G08ABAGAH4-IT */
/* NAND: driver related configs */
#define CONFIG_SYS_NAND_BAD_BLOCK_POS	NAND_LARGE_BADBLOCK_POS
#define CONFIG_SYS_NAND_ECCPOS		{ 2, 3, 4, 5, 6, 7, 8, 9, \
					 10, 11, 12, 13, 14, 15, 16, 17, \
					 18, 19, 20, 21, 22, 23, 24, 25, \
					 26, 27, 28, 29, 30, 31, 32, 33, \
					 34, 35, 36, 37, 38, 39, 40, 41, \
					 42, 43, 44, 45, 46, 47, 48, 49, \
					 50, 51, 52, 53, 54, 55, 56, 57, }

#define CONFIG_SYS_NAND_ECCSIZE		512
#define CONFIG_SYS_NAND_ECCBYTES	14
#define CONFIG_SYS_NAND_ONFI_DETECTION	/* ONFI 2.0 MT29F2G08ABAGAH4-IT */
#define CONFIG_NAND_OMAP_ECCSCHEME	OMAP_ECC_BCH8_CODE_HW
#define CONFIG_SYS_NAND_U_BOOT_OFFS	0x000c0000
/* NAND: SPL related configs */
#ifdef CONFIG_SPL_OS_BOOT
#define CONFIG_SYS_NAND_SPL_KERNEL_OFFS	0x00200000 /* kernel offset */
#endif
#endif /* !CONFIG_MTD_RAW_NAND */

/*
 * Disable MMC DM for SPL build and can be re-enabled after adding
 * DM support in SPL
 */
#ifdef CONFIG_SPL_BUILD
#undef CONFIG_DM_MMC
#undef CONFIG_TIMER
#endif

#if defined(CONFIG_SPL_BUILD) && defined(CONFIG_SPL_USB_ETHER)
/* Remove other SPL modes. */
/* disable host part of MUSB in SPL */
/* disable EFI partitions and partition UUID support */
#endif

/* USB Device Firmware Update support */
#ifndef CONFIG_SPL_BUILD
#define DFUARGS \
	DFU_ALT_INFO_RAM \
	DFU_ALT_INFO_NAND
#endif

/*
 * Default to using SPI for environment, etc.
 * 0x000000 - 0x020000 : SPL (128KiB)
 * 0x020000 - 0x0A0000 : U-Boot (512KiB)
 * 0x0A0000 - 0x0BFFFF : First copy of U-Boot Environment (128KiB)
 * 0x0C0000 - 0x0DFFFF : Second copy of U-Boot Environment (128KiB)
 * 0x0E0000 - 0x0EFFFF : DTB for Linux kernel (64KiB)
 * 0x0F0000 - 0x800000 : Linux Kernel (7+ MiB)
 */
#if defined(CONFIG_SPI_BOOT)
/* SPL related */
#define CONFIG_SYS_SPI_U_BOOT_SIZE	0x80000
#elif defined(CONFIG_ENV_IS_IN_NAND)
#define CONFIG_SYS_ENV_SECT_SIZE	CONFIG_SYS_NAND_BLOCK_SIZE
#endif

/* SPI flash. */

/* Network. */
#define CONFIG_PHY_SMSC

#ifdef CONFIG_DRIVER_TI_CPSW
/* Lipem has fixed  */
#undef CONFIG_CLOCK_SYNTHESIZER
#undef CLK_SYNTHESIZER_I2C_ADDR
#ifndef CONFIG_DM_ETH
#define CONFIG_DM_ETH
#endif	/* should be defined */
#endif

#endif	/* ! __CONFIG_LINSYS_LIPEM_H */
