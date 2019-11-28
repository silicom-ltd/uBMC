################################################################################
#
# marvell-cm3-toolchain
# This toolchain is copied from Centos to slove the issue CM3 cannot work on Centos
#
################################################################################
MARVELL_CM3_TOOLCHAIN_SITE = $(TOPDIR)/dl
#ARM_GNU_RM_TOOLCHAIN_SOURCE = arm-linux-gcc.tar.bz2
#The deafult toolchain can not work for CM3	on CentOS
#So we make a new one to fix this issue
MARVELL_CM3_TOOLCHAIN_SOURCE = marvell-cm3-toolchain.tar.bz2
#ARM_GNU_RM_TOOLCHAIN_SOURCE = gcc-arm-none-eabi-5_4-2016q3-20160926-linux.tar.bz2
#HOST_ARM_GNU_RM_TOOLCHAIN_INSTALL_DIR = $(HOST_DIR)/opt/gcc-arm-none-eabi-5_4-2016q3
HOST_MARVELL_CM3_TOOLCHAIN_INSTALL_DIR = $(HOST_DIR)/opt/marvell-cm3-toolchain

define HOST_MARVELL_CM3_TOOLCHAIN_INSTALL_CMDS
	rm -rf $(HOST_MARVELL_CM3_TOOLCHAIN_INSTALL_DIR)
	mkdir -p $(HOST_MARVELL_CM3_TOOLCHAIN_INSTALL_DIR)
	mv $(@D)/* $(HOST_MARVELL_CM3_TOOLCHAIN_INSTALL_DIR)/

	cd $(HOST_DIR)/bin; \
	for i in $(HOST_MARVELL_CM3_TOOLCHAIN_INSTALL_DIR)/bin/*; do \
		ln -sf $$(echo $$i | sed 's%^$(HOST_DIR)%..%') .; \
	done
endef

$(eval $(host-generic-package))
