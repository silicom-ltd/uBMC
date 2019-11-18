################################################################################
#
# ttyd
#
################################################################################

TTYD_VERSION = 1.5.2
TTYD_SITE = $(call github,tsl0922,ttyd,$(TTYD_VERSION))
TTYD_INSTALL_STAGING = YES
TTYD_LICENSE = MIT
TTYD_LICENSE_FILES = LICENSE
TTYD_DEPENDENCIES = json-c openssl libwebsockets

$(eval $(cmake-package))
