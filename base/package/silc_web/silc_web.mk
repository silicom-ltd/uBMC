################################################################################
#
# silc_web
#
################################################################################

SILC_WEB_VERSION = 1.0
SILC_WEB_SITE = $(TOPDIR)/../base/src/silc_web
SILC_WEB_SITE_METHOD = local
SILC_WEB_INSTALL_STAGING = YES
SILC_WEB_LICENSE = Apache
SILC_WEB_LICENSE_FILES = LICENSE
SILC_WEB_DEPENDENCIES = lua linux-pam silc_mgmtd

define SILC_WEB_BUILD_CMDS
	$(MAKE) CC="$(TARGET_CC)" TARGET_CFLAGS="$(TARGET_CFLAGS)" AR="$(TARGET_AR)" LD="$(TARGET_LD)" -j1 -C $(@D) hostenv hostcopy
endef

define SILC_WEB_INSTALL_STAGING_CMDS
endef

define SILC_WEB_INSTALL_TARGET_CMDS
	cp -ar $(@D)/build/luci.cgi $(@D)/host/www/cgi-bin/sweb
	mkdir -p $(TARGET_DIR)/opt/silc-web
	cp -ar $(@D)/build $(TARGET_DIR)/opt/silc-web/
	# po2lmo is a host app
	rm -rf $(TARGET_DIR)/opt/silc-web/build/po2lmo
	cp -ar $(@D)/host $(TARGET_DIR)/opt/silc-web/
	cp -ar $(@D)/silc-web-*.sh $(TARGET_DIR)/opt/silc-web/
endef

$(eval $(generic-package))
