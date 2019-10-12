################################################################################
#
# pam_radius
#
################################################################################

PAM_RADIUS_VERSION = 1.4.0
PAM_RADIUS_SITE = $(TOPDIR)/../base/src/linux_common/pam/pam_radius-$(PAM_RADIUS_VERSION)
PAM_RADIUS_SITE_METHOD = local
PAM_RADIUS_INSTALL_STAGING = YES
PAM_RADIUS_LICENSE = Apache
PAM_RADIUS_LICENSE_FILES = LICENSE

define PAM_RADIUS_BUILD_CMDS
	$(MAKE) CC="$(TARGET_CC)" CFLAGS="$(TARGET_CFLAGS)  -Wall -fPIC" AR="$(TARGET_AR)" LD="$(TARGET_LD)" -C $(@D)
endef

define PAM_RADIUS_INSTALL_STAGING_CMDS
endef

define PAM_RADIUS_INSTALL_TARGET_CMDS
	$(INSTALL) -D -m 0755 $(@D)/pam_radius_auth.so $(TARGET_DIR)/lib/security/
	$(INSTALL) -D -m 0644 $(@D)/pam_radius_auth.conf $(TARGET_DIR)/etc/
endef

#$(eval $(generic-package))
$(eval $(autotools-package))
