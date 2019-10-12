################################################################################
#
# pam_tacplus
#
################################################################################

PAM_TACPLUS_VERSION = 1.3.9
PAM_TACPLUS_SITE = $(TOPDIR)/../base/src/linux_common/pam/pam_tacplus-$(PAM_TACPLUS_VERSION)
PAM_TACPLUS_SITE_METHOD = local
PAM_TACPLUS_INSTALL_STAGING = YES
PAM_TACPLUS_LICENSE = Apache
PAM_TACPLUS_LICENSE_FILES = LICENSE
PAM_TACPLUS_DEPENDENCIES = linux-pam

#fix libtool relinking problem when cross-compiling
define PAM_TACPLUS_RELINK_FIX
	sed -i "/relink_command/d" $(@D)/pam_tacplus.la
endef

PAM_TACPLUS_PRE_INSTALL_STAGING_HOOKS += PAM_TACPLUS_RELINK_FIX

define PAM_TACPLUS_INSTALL_TARGET_LIBS
	$(INSTALL) -D -m 0755 $(@D)/.libs/pam_tacplus.so $(TARGET_DIR)/lib/security/
endef

PAM_TACPLUS_POST_INSTALL_TARGET_HOOKS += PAM_TACPLUS_INSTALL_TARGET_LIBS

$(eval $(autotools-package))
