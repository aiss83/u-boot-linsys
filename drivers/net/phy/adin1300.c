// SPDX-License-Identifier: GPL-2.0+
/*
 * ADIN1300 PHY drivers
 *
 *   Copyright 2022 Lin-sys LLC.
 *   author Aleksandr Shaikhrazeev
 *
 * Some code copied from linux kernel
 * Copyright (c) 2006 Herbert Valerio Riedel <hvr@gnu.org>
 */
#include <common.h>
#include <miiphy.h>
#include <phy.h>

#define ADIN1300_SW_PD      (1 << 11)   /* Software power down mode */

static int adin1300_startup(struct phy_device *phydev)
{
	int ret;

	ret = genphy_update_link(phydev);
	if (ret)
		return ret;

	return genphy_parse_link(phydev);   /* exit success */
}

static struct phy_driver adin1300_driver = {
	.name = "ADIN 1300",
    .uid = 0x0283bc30,  /* ADIN 1300 ID */
	.mask = 0xffff0,
	.features = (PHY_GBIT_FEATURES | SUPPORTED_MII | SUPPORTED_AUI),  /* 1GBIT PHY */
	.config = &genphy_config,
	.startup = &adin1300_startup,
	.shutdown = &genphy_shutdown,
};

int phy_adin1300_init(void)
{
	phy_register(&adin1300_driver);

	return 0;
}
