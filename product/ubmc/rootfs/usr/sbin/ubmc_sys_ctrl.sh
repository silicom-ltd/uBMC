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

mtd0=/dev/mtd0
mtd1=/dev/mtd1

upgrade_raw_image()
{
	################################################
	##  upgrade with Coreboot/Blinkboot raw image
	################################################
	flash_img=$1

	output_bios_upg_status "Processing: generate flash region image"
	flash_img_part0=/tmp/bmc_flash_img_part0
	flash_img_part1=/tmp/bmc_flash_img_part1
	dd if=${flash_img} of=${flash_img_part0} bs=4096 count=2048 skip=0
	dd if=${flash_img} of=${flash_img_part1} bs=4096 count=2048 skip=2048

	output_bios_upg_status "Processing: mount flash device"
	mount_mtd

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

	output_bios_upg_status "Processing: reboot host to complete upgrade"
	ubmc_sys_ctrl -c

	output_bios_upg_status "OK: BIOS upgrade complete"
}

upgrade_signed_image()
{
	################################################
	##  upgrade with H2O signed image
	################################################
	flash_img=$1

	if [ "$(strings ${flash_img}|grep 'InsydeFlash')" == "" ]; then
		output_bios_upg_status "Error: it is not an Insyde H2O image" "1"
	fi

	if [ "$(/usr/sbin/ubmc_usb_cdrom_ctrl.sh -s)" == "CDROM is occupied" ]; then
		output_bios_upg_status "Error: usb-cdrom is occupied, please detach it first" "1"
	fi

	output_bios_upg_status "Processing: mount usb device"
	usb_mount=/mnt/bmc-usb
	if [ ! -e ${usb_mount} ]; then
		echo 32M > /sys/block/zram0/disksize
		echo 32M > /sys/block/zram0/mem_limit
		mkfs.vfat /dev/zram0
		mkdir ${usb_mount}
	fi
	mount /dev/zram0 ${usb_mount}
	cp -f ${flash_img} ${usb_mount}/GRANGEVILLE.fd
	sync
	umount ${usb_mount}

	output_bios_upg_status "Processing: attach usb device"
	modprobe g_mass_storage file=/dev/zram0 removable=1
	sleep 1

	output_bios_upg_status "Processing: mount flash device"
	mount_mtd
	echo "This will corrupt the BIOS" > /tmp/corrupt.txt
	dd if=/tmp/corrupt.txt of=/dev/mtd1 bs=4096 seek=40
	umount_mtd

	output_bios_upg_status "Processing: reboot host to complete upgrade"
	upgrade_started=/tmp/h2o_bios_upgrade_started
	upgrade_done=/tmp/h2o_bios_upgrade_done
	upgrade_timeout=/tmp/h2o_bios_upgrade_timeout

	touch ${upgrade_started}
	ubmc_sys_ctrl -c

	TIMEOUT=0
	while [ true ]; do 
		if [ -e "${upgrade_done}" ]; then
			output_bios_upg_status "OK: BIOS upgrade complete"
			break;
		fi
		sleep 1;
		TIMEOUT=$(($TIMEOUT+1))
		if [ $TIMEOUT == 180 ]; then
			rmmod g_mass_storage
			touch ${upgrade_timeout}
			output_bios_upg_status "Error: BIOS upgrade failed, please check host console log" "1"
			return
		fi
	done

	rmmod g_mass_storage
	rm -rf ${upgrade_done}
}

upgrade_flash()
{
	PRODUCT_SUB=$(get_product_sub_name)
	if [ "${PRODUCT_SUB}" != "" ]; then
		output_bios_upg_status "Error: Product ${PRODUCT_SUB} not supported yet" "1"
	fi

	model=$(cat /etc/product/UBMC/OEMI/model.txt)
	if [ "${model}" == "ATT-V150" ]; then
		output_bios_upg_status "Error: Module ${model} not supported yet" "1"
	fi

	output_bios_upg_status "Processing: verify flash image file"
	flash_img=$1
	if [ "" == "${flash_img}" ]; then
		output_bios_upg_status "Error: flash image is null" "1"
	fi
	if [ ! -e ${flash_img} ]; then
		output_bios_upg_status "Error: ${flash_img} doesn't exist" "1"
	fi

	img_size=$(du -b ${flash_img} |cut -f1)
	if [ "${img_size}" -eq 16777216 ]; then
		upgrade_raw_image ${flash_img} $2
	elif [ "${img_size}" -gt 16777216 ] && [ "${img_size}" -lt 18874368 ]; then
		upgrade_signed_image ${flash_img}
	else
		output_bios_upg_status "Error: ${flash_img} size ${img_size} is not right" "1"
	fi
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

