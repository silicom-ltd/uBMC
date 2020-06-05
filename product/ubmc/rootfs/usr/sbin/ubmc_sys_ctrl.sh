#!/bin/sh
#
. /usr/sbin/silc_util.sh

prog_name=ubmc_sys_ctrl
prog=/usr/bin/$prog_name
bios_upg_status=/tmp/bmc_bios_upgrade.status
flash_img=/tmp/bmc_cli_upload_bios.img
host_bios_backup_auto=/var/images/host_bios_auto_backup.bin
host_bios_backup_manual=/var/images/host_bios_manual_backup.bin

#$1 is 0 or 1 
#1 means enable host falsh
#0 means disable host falsh
control_host_flash_mux()
{
	UBMC_XS_SLM_FLASH_MUX_PIN=27
	#BANK 2 PIN 27 
	UBMC_SKYD_FLASH_MUX_PIN=473  
	PRODUCT_SUB=$(get_product_sub_name)
	if [ "${PRODUCT_SUB}" == "UBMC_ESP" ]; then
		PINNUM=$UBMC_SKYD_FLASH_MUX_PIN
	else
		PINNUM=$UBMC_XS_SLM_FLASH_MUX_PIN
	fi
	if [ -d /sys/class/gpio/gpio$PINNUM ]; then
		echo $1 > /sys/class/gpio/gpio$PINNUM/value
	else
		echo $PINNUM > /sys/class/gpio/export
		echo $1 > /sys/class/gpio/gpio$PINNUM/value 
		if [ $? != 0 ]; then
			echo "error"
		fi
	fi
}
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
	#control_host_flash_mux 1
	modprobe m25p80
}

umount_mtd()
{
	echo 0 > /sys/class/gpio/gpio27/value
	#control_host_flash_mux 0
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
		if [ -e "${flash_img}" ]; then
			rm -rf ${flash_img}
		fi
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

	output_bios_upg_status "Processing: reprogramming the BIOS flash"
	flashcp ${img} ${mtd}

	output_bios_upg_status "Processing: verifying the BIOS flash"
	dd if=${mtd} of=${dump} bs=4096 count=2048
	ret=$(diff ${img} ${dump})
	rm -rf ${dump}
	if [ "${ret}" != "" ]; then
		output_bios_upg_status "Error: fail to verify the flash ${mtd}" "2"
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

	output_bios_upg_status "Processing: generate the flash region image"
	flash_img_part0=/tmp/bmc_flash_img_part0
	flash_img_part1=/tmp/bmc_flash_img_part1
	dd if=${flash_img} of=${flash_img_part0} bs=4096 count=2048 skip=0
	dd if=${flash_img} of=${flash_img_part1} bs=4096 count=2048 skip=2048

	output_bios_upg_status "Processing: mount the flash device"
	mount_mtd

	if [ ! -e ${mtd0} -o ! -e ${mtd1} ]; then
		output_bios_upg_status "Error: the flash device not found" "2"
	fi

	if [ "$(strings ${flash_img}|grep 'Vyatta Secure Boot')" != "" ]; then
		echo "It is blinkboot"
		output_bios_upg_status "Processing: merge the DMI info"
		dd conv=notrunc if=${mtd1} of=${flash_img_part1} bs=4096 count=31 skip=1552 seek=1552

	else
		echo "It is coreboot"
		output_bios_upg_status "Processing: merge the DMI info"
		dd conv=notrunc if=${mtd1} of=${flash_img_part1} bs=256 count=1 skip=32513 seek=32513

	fi

	if [ "$2" == "all" ]; then
		output_bios_upg_status "Processing: merge the LAN info"
		dd conv=notrunc if=${mtd0} of=${flash_img_part0} bs=4096 count=618 skip=1430 seek=1430

		upgrade_region ${flash_img_part0} ${mtd0}
	fi
	upgrade_region ${flash_img_part1} ${mtd1}

	umount_mtd

	output_bios_upg_status "Processing: reboot the host to complete upgrade"
	ubmc_sys_ctrl -c

	output_bios_upg_status "OK: the BIOS upgrade complete"
}

read_bios()
{
	bios_img=$1
	dd conv=notrunc if=${mtd0} of=${bios_img} bs=4096 count=2048
	dd conv=notrunc if=${mtd1} of=${bios_img} bs=4096 count=2048 seek=2048
}

write_bios()
{
	bios_img=$1
	bios_img_part0=/tmp/bmc_bios_img_part0
	bios_img_part1=/tmp/bmc_bios_img_part1
	dd if=${bios_img} of=${bios_img_part0} bs=4096 count=2048 skip=0
	dd if=${bios_img} of=${bios_img_part1} bs=4096 count=2048 skip=2048
	flashcp ${bios_img_part0} ${mtd0}
	flashcp ${bios_img_part1} ${mtd1}
}

backup_bios()
{
	bios_img=${host_bios_backup_manual}

	output_bios_upg_status "Processing: mount the flash device"
	mount_mtd

	output_bios_upg_status "Processing: back up the BIOS"
	read_bios ${bios_img}

	umount_mtd
	output_bios_upg_status "OK: the BIOS backup complete"
}

restore_bios()
{
	bios_img=${host_bios_backup_auto}
	if [ ! -e ${bios_img} ] || [ "$(du -b ${bios_img} |cut -f1)" != 16777216 ]; then
		bios_img=${host_bios_backup_manual}
		if [ ! -e ${bios_img} ] || [ "$(du -b ${bios_img} |cut -f1)" != 16777216 ]; then
			output_bios_upg_status "Error: the BIOS backup not found" "1"
		fi
		output_bios_upg_status "Processing: restore the BIOS to the manual-saved backup"
	else
		output_bios_upg_status "Processing: restore the BIOS to the auto-saved backup"
	fi

	output_bios_upg_status "Processing: mount the flash device"
	mount_mtd

	output_bios_upg_status "Processing: restore the BIOS"
	write_bios ${bios_img}

	umount_mtd
	ubmc_sys_ctrl -c
	output_bios_upg_status "OK: the BIOS restore complete"
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

	flash_img_cert=$(strings ${flash_img}|grep Silicom_SecureFlash)
	if [ "${flash_img_cert}" == "${flash_img_cert/Silicom_SecureFlash-ext/}" ] && [ "${flash_img_cert}" == "${flash_img_cert/Silicom_SecureFlash-key/}" ]; then
		output_bios_upg_status "Error: the images doesn't contain any valid certificate" "1"
	fi
	if [ "${flash_img_cert}" != "${flash_img_cert/Silicom_SecureFlash-ext/}" ] && [ "${flash_img_cert}" != "${flash_img_cert/Silicom_SecureFlash-key/}" ]; then
		output_bios_upg_status "Error: CROSS upgrade is not supported" "1"
	fi

	if [ "$(/usr/sbin/ubmc_usb_cdrom_ctrl.sh -s)" == "CDROM is occupied" ]; then
		output_bios_upg_status "Error: the usb-cdrom is occupied, please detach it first" "1"
	fi

	output_bios_upg_status "Processing: mount the flash device"
	mount_mtd

	host_img=${host_bios_backup_auto}
	if [ ! -e ${host_img} ] || [ "$(du -b ${host_img} |cut -f1)" != 16777216 ]; then
		output_bios_upg_status "Processing: back up the BIOS"
		read_bios ${host_img}
	else
		output_bios_upg_status "Processing: the BIOS has a backup"
	fi

	host_img_cert=$(strings ${host_img}|grep Silicom_SecureFlash)
	if [ "${host_img_cert}" == "${host_img_cert/Silicom_SecureFlash-ext/}" ] && [ "${host_img_cert}" == "${host_img_cert/Silicom_SecureFlash-key/}" ]; then
		output_bios_upg_status "Error: the host BIOS doesn't contain any valid certificate" "2"
	fi
	if [ "${host_img_cert}" != "${host_img_cert/Silicom_SecureFlash-ext/}" ] && [ "${flash_img_cert}" != "${flash_img_cert/Silicom_SecureFlash-key/}" ]; then
		output_bios_upg_status "Error: the host BIOS and the upgrade image have different certificates" "2"
	fi
	if [ "${host_img_cert}" != "${host_img_cert/Silicom_SecureFlash-key/}" ] && [ "${flash_img_cert}" != "${flash_img_cert/Silicom_SecureFlash-ext/}" ]; then
		output_bios_upg_status "Error: the host BIOS and the upgrade image have different certificates" "2"
	fi

	echo "This will corrupt the BIOS" > /tmp/corrupt.txt
	dd if=/tmp/corrupt.txt of=/dev/mtd1 bs=4096 seek=40
	umount_mtd

	output_bios_upg_status "Processing: prepare the usb device"
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

	output_bios_upg_status "Processing: attach the usb device"
	modprobe g_mass_storage file=/dev/zram0 removable=1
	sleep 1

	output_bios_upg_status "Processing: reboot the host to complete upgrade"
	upgrade_started=/tmp/h2o_bios_upgrade_started
	upgrade_done=/tmp/h2o_bios_upgrade_done
	upgrade_timeout=/tmp/h2o_bios_upgrade_timeout

	touch ${upgrade_started}
	ubmc_sys_ctrl -c

	need_restore=0
	timeout=0
	while [ true ]; do 
		if [ -e "${upgrade_done}" ]; then
			output_bios_upg_status "OK: the BIOS upgrade complete"
			rm -rf ${upgrade_done}
			break;
		fi
		sleep 1;
		timeout=$(($timeout+1))
		if [ $timeout == 180 ]; then
			output_bios_upg_status "Processing: the BIOS upgrade failed, restore the BIOS"
			touch ${upgrade_timeout}
			need_restore=1
			break
		fi
	done

	rmmod g_mass_storage
	if [ ${need_restore} == 1 ]; then
		mount_mtd
		write_bios ${host_img}
		umount_mtd
		ubmc_sys_ctrl -c
		output_bios_upg_status "Error: the BIOS upgrade failed, and the BIOS has been restored"
	fi
	rm -rf ${host_img}
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
		#upgrade_raw_image ${flash_img} $2
		output_bios_upg_status "Error: raw image upgrade is not allowed" "1"
	elif [ "${img_size}" -gt 16777216 ] && [ "${img_size}" -lt 18874368 ]; then
		upgrade_signed_image ${flash_img}
	else
		output_bios_upg_status "Error: ${flash_img} size ${img_size} is not right" "1"
	fi
	rm -rf ${flash_img}
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
  -su)
	show_bios_upg_status
	;;
  -bb)
    backup_bios
	;;
  -rb)
    restore_bios
	;;
  *)
    echo "usage: ${prog_name} [-o] [-s] [-f] [-c] [-r] [-p] [-q] [-g] [-b|-u <file>] [-su] [-bb] [-rb]"
    echo "-c            power cycle"
    echo "-o            power on"
    echo "-s            power off"
    echo "-f            power forceoff"
    echo "-r            power reset"
    echo "-q            power status"
    echo "-p            power operiton progress"
    echo "-g            serial connection status"
    echo "-b|-u <file>  upgrade BIOS/SPI flash"
    echo "-su           show upgrade BIOS status"
    echo "-bb           backup BIOS"
    echo "-rb           restore BIOS"
esac

exit 0

