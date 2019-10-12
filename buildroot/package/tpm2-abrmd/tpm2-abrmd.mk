################################################################################
#
# tpm2-abrmd
#
################################################################################

TPM2_ABRMD_VERSION = 1.2.0
TPM2_ABRMD_SITE = https://github.com/tpm2-software/tpm2-abrmd/releases/download/$(TPM2_ABRMD_VERSION)
TPM2_ABRMD_LICENSE = BSD-2-Clause
TPM2_ABRMD_LICENSE_FILES = LICENSE
TPM2_ABRMD_INSTALL_STAGING = YES
TPM2_ABRMD_DEPENDENCIES = dbus libglib2 tpm2-tss host-pkgconf

TPM2_ABRMD_CONF_OPTS += \
						--with-systemdsystemunitdir=$(if $(BR2_INIT_SYSTEMD),/usr/lib/systemd/system,no) \
						--with-udevrulesdir=$(if $(BR2_PACKAGE_HAS_UDEV),/usr/lib/udev/rules.d,no)

define TPM2_ABRMD_INSTALL_INIT_SYSTEMD
	$(TARGET_MAKE_ENV) $(MAKE1) -C $(@D) DESTDIR=$(TARGET_DIR) \
						install-systemdpresetDATA install-systemdsystemunitDATA
endef

# Without udev we need an init script to set the ownership of /dev/tpm[0-9]*
define TPM2_ABRMD_INSTALL_INIT_SYSV
	$(INSTALL) -D -m 0755 -t $(TARGET_DIR)/etc/init.d \
	$(if $(BR2_PACKAGE_HAS_UDEV),,$(TPM2_ABRMD_PKGDIR)/S30devtpmperms) \
	$(TPM2_ABRMD_PKGDIR)/S80tpm2-abrmd
	$(INSTALL) -D -m 0644 $(TPM2_ABRMD_PKGDIR)/etc.default.tpm2-abrmd \
	$(TARGET_DIR)/etc/default/tpm2-abrmd
endef

define TPM2_ABRMD_USERS
	tss -1 tss -1 * - - - TPM2 Access Broker & Resource Management daemon
endef

$(eval $(autotools-package))
