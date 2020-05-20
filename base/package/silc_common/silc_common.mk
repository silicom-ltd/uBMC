#############################################################
#
# silc_common
#
#############################################################

SILC_COMMON_VERSION = 1.0
SILC_COMMON_SITE = git://github.com/silicom-ltd/silc_common.git
SILC_COMMON_SITE_METHOD = git
SILC_COMMON_VERSION = f12e01195c1c6d893bead18be8354e224d9c4087
SILC_COMMON_INSTALL_STAGING = YES
SILC_COMMON_LICENSE = BSD
SILC_COMMON_LICENSE_FILES = COPYING

define SILC_COMMON_BUILD_CMDS
	$(MAKE) CC="$(TARGET_CC)" LD="$(TARGET_LD)" -C $(@D)
endef

define SILC_COMMON_INSTALL_TARGET_CMDS
#	$(MAKE) -C $(@D) CC=$(TARGET_CC) DESTDIR=$(TARGET_DIR) install
	cp -a $(STAGING_DIR)/usr/lib/libsilc.so* $(TARGET_DIR)/usr/lib
endef

$(eval $(autotools-package))


