#############################################################
#
# silc_common
#
#############################################################

SILC_COMMON_VERSION = 1.0
SILC_COMMON_SITE = git://github.com/silicom-ltd/silc_common.git
SILC_COMMON_SITE_METHOD = git
SILC_COMMON_VERSION = 663828122229da371830b4e22815018d1201115d
SILC_COMMON_INSTALL_STAGING = YES
SILC_COMMON_LICENSE = BSD
SILC_COMMON_LICENSE_FILES = COPYING

SILC_COMMON_CONF_OPTS += --host=powerpc-fsl-linux

define SILC_COMMON_BUILD_CMDS
	$(MAKE) CC="$(TARGET_CC)" LD="$(TARGET_LD)" -C $(@D)
endef

define SILC_COMMON_INSTALL_TARGET_CMDS
#	$(MAKE) -C $(@D) CC=$(TARGET_CC) DESTDIR=$(TARGET_DIR) install
	cp -a $(STAGING_DIR)/usr/lib/libsilc.so* $(TARGET_DIR)/usr/lib
endef

$(eval $(autotools-package))


