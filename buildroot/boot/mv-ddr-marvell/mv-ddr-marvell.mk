################################################################################
#
# mv-ddr-marvell
#
################################################################################

ifeq ($(BR2_TARGET_MV_DDR_MARVELL_CUSTOM_GIT),y)
MV_DDR_MARVELL_VERSION = $(call qstrip,$(BR2_TARGET_MV_DDR_MARVELL_CUSTOM_REPO_VERSION))
MV_DDR_MARVELL_SITE = $(call qstrip,$(BR2_TARGET_MV_DDR_MARVELL_CUSTOM_REPO_URL))
MV_DDR_MARVELL_SITE_METHOD = git
BR_NO_CHECK_HASH_FOR += $(MV_DDR_MARVELL_SOURCE)
else
ifeq ($(BR2_TARGET_MV_DDR_MARVELL_CUSTOM_TARBALL),y)
MV_DDR_MARVELL_TARBALL = $(call qstrip,$(BR2_TARGET_MV_DDR_MARVELL_CUSTOM_TARBALL_LOCATION))
MV_DDR_MARVELL_SITE = $(patsubst %/,%,$(dir $(MV_DDR_MARVELL_TARBALL)))
MV_DDR_MARVELL_SITE_METHOD = file
MV_DDR_MARVELL_SOURCE = $(notdir $(MV_DDR_MARVELL_TARBALL))
BR_NO_CHECK_HASH_FOR += $(MV_DDR_MARVELL_SOURCE)
else
# This is the commit for mv_ddr-armada-18.12.0
MV_DDR_MARVELL_VERSION = 618dadd1491eb2f7b2fd74313c04f7accddae475
MV_DDR_MARVELL_SITE = $(call github,MarvellEmbeddedProcessors,mv-ddr-marvell,$(MV_DDR_MARVELL_VERSION))
endif
endif
MV_DDR_MARVELL_LICENSE = GPL-2.0+ or LGPL-2.1 with freertos-exception-2.0, BSD-3-Clause, Marvell Commercial
MV_DDR_MARVELL_LICENSE_FILES = ddr3_init.c

$(eval $(generic-package))
