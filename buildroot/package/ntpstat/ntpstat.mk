################################################################################
#
# ntpstat
#
################################################################################


NTPSTAT_SOURCE = ntpstat.tar.gz
NTPSTAT_SITE = https://github.com/darkhelmet


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
