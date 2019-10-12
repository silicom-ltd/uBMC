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
UBMC_MGMTD_DEPENDENCIES = is_ubmc_ipmi silc_mgmtd silc_common



$(eval $(autotools-package))

