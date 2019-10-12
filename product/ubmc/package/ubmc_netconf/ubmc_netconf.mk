################################################################################
#
# ubmc_netconf
#
################################################################################

UBMC_NETCONF_VERSION = 1.0
UBMC_NETCONF_SITE = $(TOPDIR)/../product/ubmc/src/ubmc_netconf
UBMC_NETCONF_SITE_METHOD = local
UBMC_NETCONF_LICENSE = Silicom
UBMC_NETCONF_AUTORECONF = YES
UBMC_NETCONF_INSTALL_STAGING = YES
UBMC_NETCONF_DEPENDENCIES = netopeer silc_mgmtd
UBMC_NETCONF_CONF_ENV += LDFLAGS="$(TARGET_LDFLAGS) -lsilc_cli_inst -lsilc_mgmtd -lsilc  -ledit -lncurses -lpthread"

define UBMC_NETCONF_POST_INSTALL_CMDS
	mkdir -p $(TARGET_DIR)/etc/netopeer/ubmc
	cp -ar $(@D)/ubmc-nc-config $(TARGET_DIR)/etc/netopeer/ubmc/
	cp -ar $(@D)/*.yin $(TARGET_DIR)/etc/netopeer/ubmc/
	cp -ar $(@D)/*.rng $(TARGET_DIR)/etc/netopeer/ubmc/
	cp -ar $(@D)/*.xsl $(TARGET_DIR)/etc/netopeer/ubmc/
	cp -ar $(@D)/cfgnetopeer/*.xml $(TARGET_DIR)/etc/netopeer/cfgnetopeer/
	cp -ar $(@D)/modules.conf.d/ubmc.xml $(TARGET_DIR)/etc/netopeer/modules.conf.d/
	cp -ar $(@D)/libnetconf/*.xml $(TARGET_DIR)/var/lib/libnetconf/
endef

UBMC_NETCONF_POST_INSTALL_TARGET_HOOKS += UBMC_NETCONF_POST_INSTALL_CMDS

$(eval $(autotools-package))

