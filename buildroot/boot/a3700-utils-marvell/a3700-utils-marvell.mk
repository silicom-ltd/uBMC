################################################################################
#
# a3700-utils-marvell
#
################################################################################

ifeq ($(BR2_TARGET_A3700_UTILS_MARVELL_CUSTOM_GIT),y)
A3700_UTILS_MARVELL_VERSION = $(call qstrip,$(BR2_TARGET_A3700_UTILS_MARVELL_CUSTOM_REPO_VERSION))
A3700_UTILS_MARVELL_SITE = $(call qstrip,$(BR2_TARGET_A3700_UTILS_MARVELL_CUSTOM_REPO_URL))
A3700_UTILS_MARVELL_SITE_METHOD = git
BR_NO_CHECK_HASH_FOR += $(A3700_UTILS_MARVELL_SOURCE)
else
ifeq ($(BR2_TARGET_A3700_UTILS_MARVELL_CUSTOM_TARBALL),y)
A3700_UTILS_MARVELL_TARBALL = $(call qstrip,$(BR2_TARGET_A3700_UTILS_MARVELL_CUSTOM_TARBALL_LOCATION))
A3700_UTILS_MARVELL_SITE = $(patsubst %/,%,$(dir $(A3700_UTILS_MARVELL_TARBALL)))
A3700_UTILS_MARVELL_SITE_METHOD = file
A3700_UTILS_MARVELL_SOURCE = $(notdir $(A3700_UTILS_MARVELL_TARBALL))
BR_NO_CHECK_HASH_FOR += $(A3700_UTILS_MARVELL_SOURCE)
else
# This is the commit for A3700-utils-armada-18.12.0
A3700_UTILS_MARVELL_VERSION = a0a1cb88327afa91415c59822fa9c5894d9ec3d7
A3700_UTILS_MARVELL_SITE = $(call github,MarvellEmbeddedProcessors,A3700-utils-marvell,$(A3700_UTILS_MARVELL_VERSION))
endif
endif
A3700_UTILS_MARVELL_LICENSE = GPL-2.0+ or LGPL-2.1 with freertos-exception-2.0, BSD-3-Clause, Marvell Commercial
A3700_UTILS_MARVELL_LICENSE_FILES = wtptp/src/TBB_Linux/readme.txt

A3700_UTILS_MARVEL_DEPENDENCIES= mv-ddr-marvell

$(eval $(generic-package))
