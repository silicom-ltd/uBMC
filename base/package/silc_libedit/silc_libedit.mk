#############################################################
#
# silc_libedit
#
#############################################################

SILC_LIBEDIT_VERSION = 1.0
SILC_LIBEDIT_SITE = $(TOPDIR)/../base/src/silc_libedit/libedit
SILC_LIBEDIT_SITE_METHOD = local
SILC_LIBEDIT_INSTALL_STAGING = YES
SILC_LIBEDIT_LICENSE = BSD
SILC_LIBEDIT_LICENSE_FILES = COPYING
SILC_LIBEDIT_DEPENDENCIES = ncurses

#define SILC_MGMTD_LIBEDIT_INSTALL_TARGET_CMDS
#	$(MAKE) -C $(@D) CC=$(TARGET_CC) DESTDIR=$(TARGET_DIR) install
#endef

$(eval $(autotools-package))


