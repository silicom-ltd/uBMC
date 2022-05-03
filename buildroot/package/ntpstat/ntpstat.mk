################################################################################
#
# ntpstat
#
################################################################################

NTPSTAT_VERSION = master
NTPSTAT_SITE = $(call github,darkhelmet,ntpstat,$(LIBNETCONF_VERSION))

define NTPSTAT_BUILD_CMDS
	echo build
	$(MAKE) $(TARGET_CONFIGURE_OPTS) $(NTPSTAT_MAKE_OPTS) -C $(@D)
endef

define NTPSTAT_INSTALL_TARGET_CMDS
	echo install
	echo $(TARGET_DIR)
	cp $(@D)/ntpstat $(TARGET_DIR)/usr/bin
endef


$(eval $(generic-package))
