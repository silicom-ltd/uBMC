################################################################################
#
# ubmc_web
#
################################################################################

UBMC_WEB_VERSION = 1.0
UBMC_WEB_SITE = $(TOPDIR)/../product/ubmc/src/ubmc_web
UBMC_WEB_SITE_METHOD = local
UBMC_WEB_INSTALL_STAGING = YES
UBMC_WEB_LICENSE = Apache
UBMC_WEB_LICENSE_FILES = LICENSE
UBMC_WEB_DEPENDENCIES = is_web

define UBMC_WEB_BUILD_CMDS
	$(MAKE) CC="$(TARGET_CC)" TARGET_CFLAGS="$(TARGET_CFLAGS)" AR="$(TARGET_AR)" LD="$(TARGET_LD)" -j1 -C $(@D) host
endef

define UBMC_WEB_INSTALL_STAGING_CMDS
endef

define UBMC_WEB_INSTALL_TARGET_CMDS
	mkdir -p $(TARGET_DIR)/opt/silc-web
	cp -ar $(@D)/host $(TARGET_DIR)/opt/silc-web/
endef

$(eval $(generic-package))
