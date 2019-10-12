#############################################################
#
# silc_mgmtd
#
#############################################################

SILC_PRODUCT_VERSION = 1.0
SILC_PRODUCT_SITE = $(TOPDIR)/../base/src/silc_product
SILC_PRODUCT_SITE_METHOD = local
SILC_PRODUCT_INSTALL_STAGING = NO
SILC_PRODUCT_LICENSE = BSD
SILC_PRODUCT_LICENSE_FILES = COPYING
SILC_PRODUCT_DEPENDENCIES =


SILC_PRODUCT = $(call qstrip, $(BR2_PACKAGE_SILC_PRODUCT_NAME))
SILC_PRODUCT_SUB = $(call qstrip, $(BR2_PACKAGE_SILC_PRODUCT_SUB))

define SILC_PRODUCT_INSTALL_TARGET_CMDS
	mkdir -p $(TARGET_DIR)/etc
	echo "$(SILC_PRODUCT)" > $(TARGET_DIR)/etc/product.txt
	echo "$(SILC_PRODUCT_SUB)" > $(TARGET_DIR)/etc/product_sub.txt
endef

$(eval $(virtual-package))


