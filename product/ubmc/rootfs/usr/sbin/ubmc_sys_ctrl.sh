#!/bin/sh
#
. /usr/sbin/silc_util.sh

prog_name=ubmc_sys_ctrl
prog=/usr/bin/$prog_name
bios_upg_status=/tmp/bmc_bios_upgrade.status

prepare_init()
{
    ori_str=$(echo $($prog -p))
    str1=$(echo $ori_str | grep "\.\.\.")
    if [ -z "$str1"  ];then
        echo "ok"
        $prog $1 &
    else
        echo "error"
    fi
}

mount_mtd()
{
	if [ "" != "$(lsmod|grep m25p80)" ]; then
		rmmod m25p80
	fi
	echo 1 > /sys/class/gpio/gpio27/value
	modprobe m25p80
}

umount_mtd()
{
	echo 0 > /sys/class/gpio/gpio27/value
	rmmod m25p80
}

output_bios_upg_status()
{
	echo "$1"
	echo "$1" > ${bios_upg_status}
	if [ "$2" == "2" ]; then
		umount_mtd
	fi
	if [ "$2" != "" ]; then
		exit 1
	fi
}

show_bios_upg_status()
{
	if [ -e ${bios_upg_status} ]; then
		cat ${bios_upg_status}
	fi
}

upgrade_region()
{
	img=$1
	mtd=$2
	dump=${img}.dump

	output_bios_upg_status "Processing: reprogramming BIOS flash"
	flashcp ${img} ${mtd}

	output_bios_upg_status "Processing: verifying BIOS flash"
	dd if=${mtd} of=${dump} bs=4096 count=2048
	ret=$(diff ${img} ${dump})
	rm -rf ${dump}
	if [ "${ret}" != "" ]; then
		output_bios_upg_status "Error: fail to verify flash ${mtd}" "2"
	fi
}

upgrade_flash()
{
	PRODUCT_SUB=$(get_product_sub_name)
	if [ "${PRODUCT_SUB}" != "UBMC_335X" ]; then
		output_bios_upg_status "Product ${PRODUCT_SUB} not supported yet"
		exit 1
	fi
	flash_img=$1
	flash_img_part0=/tmp/bmc_flash_img_part0
	flash_img_part1=/tmp/bmc_flash_img_part1

	output_bios_upg_status "Processing: verify flash image file"
	if [ "" == "${flash_img}" ]; then
		output_bios_upg_status "Error: flash image is null" "1"
	fi
	if [ ! -e ${flash_img} ]; then
		output_bios_upg_status "Error: ${flash_img} doesn't exist" "1"
	fi
	img_size=$(du -b ${flash_img} |cut -f1)
	if [ "16777216" != "${img_size}" ]; then
		output_bios_upg_status "Error: ${flash_img} size is not 16777216" "1"
	fi

	output_bios_upg_status "Processing: generate flash region image"
	dd if=${flash_img} of=${flash_img_part0} bs=4096 count=2048 skip=0
	dd if=${flash_img} of=${flash_img_part1} bs=4096 count=2048 skip=2048

	output_bios_upg_status "Processing: mount flash device"
	mount_mtd

	mtd0=/dev/mtd0
	mtd1=/dev/mtd1

	if [ ! -e ${mtd0} -o ! -e ${mtd1} ]; then
		output_bios_upg_status "Error: flash device not found" "2"
	fi

	if [ "$(strings ${flash_img}|grep 'Vyatta Secure Boot')" != "" ]; then
		echo "It is blinkboot"
		output_bios_upg_status "Processing: merge DMI info"
		dd conv=notrunc if=${mtd1} of=${flash_img_part1} bs=4096 count=31 skip=1552 seek=1552

	else
		echo "It is coreboot"
		output_bios_upg_status "Processing: merge DMI info"
		dd conv=notrunc if=${mtd1} of=${flash_img_part1} bs=256 count=1 skip=32513 seek=32513

	fi

	if [ "$2" == "all" ]; then
		output_bios_upg_status "Processing: merge LAN info"
		dd conv=notrunc if=${mtd0} of=${flash_img_part0} bs=4096 count=618 skip=1430 seek=1430

		upgrade_region ${flash_img_part0} ${mtd0}
	fi
	upgrade_region ${flash_img_part1} ${mtd1}

	umount_mtd
	output_bios_upg_status "OK: upgrade BIOS flash complete"
}

case "$1" in
  -c)
    prepare_init -c
	;;
  -o)
    prepare_init -o
	;;
  -s)
    prepare_init -s
	;;
  -f)
    prepare_init -f
	;;
  -r)
    prepare_init -r
	;;
  -q)
    $prog -q 
	;;
  -p)
    $prog -p 
	;;
  -g)
    $prog -g 
	;;
  -b)
    upgrade_flash "$2"
	;;
  -u)
    upgrade_flash "$2" "all"
	;;
  -up)
	show_bios_upg_status
	;;
  *)
    echo "usage: ${prog_name} [-o] [-s] [-f] [-c] [-r] [-p] [-q] [-g] [-b|-u <file>]"
    echo "-c            power cycle"
    echo "-o            power on"
    echo "-s            power off"
    echo "-f            power forceoff"
    echo "-r            power reset"
    echo "-q            power status"
    echo "-p            power operiton progress"
    echo "-g            serial connection status"
    echo "-b|-u <file>  upgrade BIOS/SPI flash"
    echo "-up           show upgrade BIOS status"
esac

exit 0

