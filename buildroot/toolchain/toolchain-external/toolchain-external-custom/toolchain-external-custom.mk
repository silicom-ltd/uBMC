################################################################################
#
# toolchain-external-custom
# copy from marvell SDK 19.06 buildroot
################################################################################

TOOLCHAIN_EXTERNAL_CUSTOM_SITE = $(patsubst %/,%,$(dir $(call qstrip,$(BR2_TOOLCHAIN_EXTERNAL_URL))))
TOOLCHAIN_EXTERNAL_CUSTOM_SOURCE = $(notdir $(call qstrip,$(BR2_TOOLCHAIN_EXTERNAL_URL)))

ifeq ($(BR2_TOOLCHAIN_EXTERNAL_CUSTOM),y)
# We can't check hashes for custom downloaded toolchains
BR_NO_CHECK_HASH_FOR += $(TOOLCHAIN_EXTERNAL_SOURCE)
endif

TOOLCHAIN_EXTERNAL_FIXUP_SYSROOT_SCRIPT = \
	$(call qstrip,$(BR2_TOOLCHAIN_EXTERNAL_FIXUP_SYSROOT_SCRIPT))

ifneq ($(TOOLCHAIN_EXTERNAL_FIXUP_SYSROOT_SCRIPT),)
define TOOLCHAIN_EXTERNAL_FIXUP_SYSROOT_SCRIPT_HOOK
	$(TOOLCHAIN_EXTERNAL_FIXUP_SYSROOT_SCRIPT) $(STAGING_DIR)
endef
TOOLCHAIN_EXTERNAL_CUSTOM_POST_INSTALL_STAGING_HOOKS += TOOLCHAIN_EXTERNAL_FIXUP_SYSROOT_SCRIPT_HOOK
endif

$(eval $(toolchain-external-package))
