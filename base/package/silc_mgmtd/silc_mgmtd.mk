#############################################################
#
# silc_mgmtd
#
#############################################################

SILC_MGMTD_VERSION = 1.0
SILC_MGMTD_SITE = $(TOPDIR)/../base/src/silc_mgmtd
SILC_MGMTD_SITE_METHOD = local
SILC_MGMTD_INSTALL_STAGING = YES
SILC_MGMTD_LICENSE = BSD
SILC_MGMTD_LICENSE_FILES = LICENSE
SILC_MGMTD_DEPENDENCIES = silc_common silc_libedit

SILC_MGMTD_CONF_ENV = CFLAGS="-O0 -DSILC_LOG_SYSLOG"

define SILC_MGMTD_INSTALL_TARGET_CMDS
	cp -dpf $(STAGING_DIR)/usr/lib/libsilc_mgmtd.so* $(TARGET_DIR)/usr/lib
	cp -dpf $(STAGING_DIR)/usr/lib/libsilc_mgmtd_inst.so* $(TARGET_DIR)/usr/lib
	cp -dpf $(STAGING_DIR)/usr/lib/libsilc_cli_inst.so* $(TARGET_DIR)/usr/lib
	$(INSTALL) -D -m 0755 $(STAGING_DIR)/usr/bin/is_mgmtd $(TARGET_DIR)/usr/bin
	$(INSTALL) -D -m 0755 $(STAGING_DIR)/usr/bin/is_cli $(TARGET_DIR)/usr/bin

	$(INSTALL) -D -m 0755 $(STAGING_DIR)/usr/bin/is_upgrade $(TARGET_DIR)/usr/sbin
	$(INSTALL) -D -m 0755 $(STAGING_DIR)/usr/bin/is_action $(TARGET_DIR)/usr/sbin

endef

$(eval $(autotools-package))


