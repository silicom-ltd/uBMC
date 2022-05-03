################################################################################
#
# ubmc_mgmtd
#
################################################################################

UBMC_MGMTD_VERSION = 1.0
UBMC_MGMTD_SITE = $(TOPDIR)/../product/ubmc/src/ubmc_mgmtd
UBMC_MGMTD_SITE_METHOD = local
UBMC_MGMTD_LICENSE = Silicom
UBMC_MGMTD_INSTALL_STAGING = YES
UBMC_MGMTD_AUTORECONF = YES
UBMC_MGMTD_DEPENDENCIES = is_ubmc_ipmi silc_mgmtd silc_common

SILC_PRODUCT = $(call qstrip, $(BR2_PACKAGE_SILC_PRODUCT_NAME))
SILC_PRODUCT_SUB = $(call qstrip, $(BR2_PACKAGE_SILC_PRODUCT_SUB))
UBMC_MGMTD_CONF_ENV = CFLAGS="-DSILC_PRODUCT=$(SILC_PRODUCT) -DSILC_PRODUCT_SUB=$(SILC_PRODUCT_SUB)"

$(eval $(autotools-package))

