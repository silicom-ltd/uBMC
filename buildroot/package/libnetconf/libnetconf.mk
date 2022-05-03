################################################################################
#
# libnetconf
#
################################################################################

LIBNETCONF_VERSION = master
LIBNETCONF_SITE = $(call github,CESNET,libnetconf,$(LIBNETCONF_VERSION))
LIBNETCONF_INSTALL_STAGING = YES
LIBNETCONF_LICENSE = GPLv2+
LIBNETCONF_LICENSE_FILES = COPYING
LIBNETCONF_DEPENDENCIES = python-pyang libxml2 libxslt libssh

LIBNETCONF_AUTORECONF = YES

$(eval $(autotools-package))
