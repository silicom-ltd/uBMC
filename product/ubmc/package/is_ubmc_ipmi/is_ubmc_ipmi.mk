################################################################################
#
# is_ubmc
#
################################################################################

IS_UBMC_IPMI_VERSION = 1.0
IS_UBMC_IPMI_SITE = $(TOPDIR)/../product/ubmc/src/is_ubmc_ipmi
IS_UBMC_IPMI_SITE_METHOD = local
IS_UBMC_IPMI_LICENSE = Silicom
#IS_UBMC_IPMI_LICENSE_FILES = README
IS_UBMC_IPMI_INSTALL_STAGING = YES
IS_UBMC_IPMI_AUTORECONF = YES
IS_UBMC_IPMI_DEPENDENCIES = silc_common


$(eval $(autotools-package))

