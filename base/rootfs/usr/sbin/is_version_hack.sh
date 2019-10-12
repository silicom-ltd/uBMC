#!/bin/sh

. /usr/sbin/silc_util.sh
PRODUCT=$(get_product_name)
#set -x
TMPD=/tmp/version_hack

mkdir -p ${TMPD}

cd ${TMPD}

if [ ! -e /dev/mtd/nor_bank0_dtb ]; then
	echo "dtb update not successful"
	# this would happen if we upgrade to bank1, and bank1 dtb is an old dtb.
	# we need to copy bank0 dtb to bank1
	BS=$((0x20000))
	dd if=/dev/mtd0 of=dtb0.dd skip=$((0x0800000/${BS})) count=1 bs=${BS}
	if [ "0000000 ffff ffff ffff ffff ffff ffff ffff ffff" == "$(hexdump dtb0.dd |head -n1)" ]; then
		echo "DTB bank0 is invalid"
		exit 0
	fi
	flash_erase /dev/mtd0 0x4800000 1
	dd if=dtb0.dd of=/dev/mtd0 seek=$((0x4800000/${BS})) count=1 bs=${BS}

	#also erase upgrade flag, becase we can't confirm it without the new dtb
	flash_erase /dev/mtd0 0x5020000 1
	echo "Rebooting to apply new dtb"
	reboot
fi

dd if=/dev/mtd/nor_uboot_flag_boot of=boot bs=1 count=1 &>/dev/null

BANK=0
if [ $(cat boot) != "0" ]; then
  BANK=1
fi

sw_file=/etc/prod_version/software_version.txt
uboot_file=/etc/prod_version/uboot_version.txt
fman_file=/etc/prod_version/fman_version.txt
rcw_file=/etc/prod_version/rcw_version.txt

#the following part has no bank
for part in fman rcw uboot; do
	dd if=/dev/mtd/nor_version_bank0_${part} of=${part}.dd bs=1 count=2 &>/dev/null
	if [ "$(cat ${part}.dd)" != "##" ]; then
		VER_FILE=$(attrib_get ${part} file)
		echo "Updating ${part} version $VER_FILE"
		flashcp ${VER_FILE} /dev/mtd/nor_version_bank0_${part}
	fi
done

part=sw
dd if=/dev/mtd/nor_version_bank${BANK}_${part} of=${part}.dd bs=1 count=2 &>/dev/null
if [ "$(cat ${part}.dd)" != "##" ]; then
	VER_FILE=$(attrib_get ${part} file)
	echo "Updating ${part} version $VER_FILE"
	flashcp ${VER_FILE} /dev/mtd/nor_version_bank${BANK}_${part}
fi

cd /tmp
rm -rf ${TMPD}



