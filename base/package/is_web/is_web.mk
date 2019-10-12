################################################################################
#
# is_web
#
################################################################################

IS_WEB_VERSION = 1.0
IS_WEB_SITE = $(TOPDIR)/../base/src/is_web
IS_WEB_SITE_METHOD = local
IS_WEB_INSTALL_STAGING = YES
IS_WEB_LICENSE = Apache
IS_WEB_LICENSE_FILES = LICENSE
IS_WEB_DEPENDENCIES = silc_web

define IS_WEB_BUILD_CMDS
	$(MAKE) CC="$(TARGET_CC)" TARGET_CFLAGS="$(TARGET_CFLAGS)" AR="$(TARGET_AR)" LD="$(TARGET_LD)" -j1 -C $(@D) host
endef

define IS_WEB_INSTALL_STAGING_CMDS
endef

define IS_WEB_INSTALL_TARGET_CMDS
	mkdir -p $(TARGET_DIR)/opt/silc-web
	cp -ar $(@D)/host $(TARGET_DIR)/opt/silc-web/
endef

$(eval $(generic-package))
