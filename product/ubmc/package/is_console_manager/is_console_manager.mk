################################################################################
#
# is_console_manager
#
################################################################################

IS_CONSOLE_MANAGER_VERSION = 1.0
IS_CONSOLE_MANAGER_SITE = $(TOPDIR)/../product/ubmc/src/is_console_manager
IS_CONSOLE_MANAGER_SITE_METHOD = local
IS_CONSOLE_MANAGER_LICENSE = Silicom
#IS_CONSOLE_MANAGER_LICENSE_FILES = README
IS_CONSOLE_MANAGER_INSTALL_STAGING = YES
IS_CONSOLE_MANAGER_DEPENDENCIES = silc_mgmtd silc_common
#IS_CONSOLE_MANAGER_CONF_ENV = CFLAGS="-O0 -g"



$(eval $(autotools-package))

