################################################################################
#
# dying_gasp
#
################################################################################
DYING_GASP_VERSION = 1.0
DYING_GASP_SITE = $(TOPDIR)/../product/ubmc/src/dying_gasp
DYING_GASP_SITE_METHOD = local
DYING_GASP_LICENSE = Silicom
DYING_GASP_AUTORECONF = YES
#IS_UBMC_IPMI_LICENSE_FILES = README
DYING_GASP_INSTALL_STAGING = YES
DYING_GASP_DEPENDENCIES = silc_mgmtd silc_common


$(eval $(autotools-package))

