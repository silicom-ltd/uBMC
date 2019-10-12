################################################################################
#
# netopeer
#
################################################################################

NETOPEER_VERSION = master
NETOPEER_SITE = $(call github,CESNET,netopeer,$(NETOPEER_VERSION))
NETOPEER_SUBDIR = server
NETOPEER_INSTALL_STAGING = YES
NETOPEER_LICENSE = GPLv2+
NETOPEER_LICENSE_FILES = COPYING
NETOPEER_DEPENDENCIES = libnetconf openssl libssh
#NETOPEER_CONF_OPTS += LDFLAGS="$(TARGET_LDFLAGS) -lsilc_mgmtd -lsilc -lpthread"

$(eval $(autotools-package))
