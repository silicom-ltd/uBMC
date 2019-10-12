#!/bin/sh

#set -x
windows_mount_point=/tmp/windows_share
local_mount_point=/var/images

usage()
{
    echo "Usage: $0 [option]..."
    echo "       -h | --help                          display this help and exit\n"
    echo "       -a | --attach <iso-file>             attach the local USB cdrom"
    echo "       -A | --attach-windows  <share-location> <share-path> <user-name> <password|none> "
    echo "                                            attach the windows USB cdrom"
    echo "       -d | --detach                        detach the USB cdrom"
    echo "       -r | --remove-local-isofile          remove iso file in local"
    echo "       -s | --show-cdrom-status             show whether CDROM is in use"
    echo "       -n | --show-cdrom-iso-name           show ISO name of current using CDROM"
    echo "       -t | --show-cdrom-attach-location    show the location in which CDROM attached"
    echo "       example: ubmc_usb_cdrom_ctrl.sh -A 192.168.49.223 share_directory/file.iso username password or none"
    echo "                ubmc_usb_cdrom_ctrl.sh -a file.iso"
}

attach_usb_cdrom()
{
    file_name=$1

    method_type=$(show_cdrom_attach_location)
    if [ "${method_type}" != "N/A" ];then
        echo "CDROM attached"
        exit
    fi

    local_file_path="${file_name}"
    if [ -f ${local_file_path} ];then
        res_str=$(modprobe g_mass_storage file=${local_file_path} cdrom=y)
        if [ -n "${res_str}" ];then
            echo ${res_str}
        else
            echo "OK"
        fi
    else
        echo "File not found"
    fi
}

mount_windows_share_dir()
{

    ip_addr=$1
    file_dir=$2
    usr_name=$3

    windows_remote_addr="//${ip_addr}/${file_dir}"

    if [ ! -f ${windows_mount_point} ];then
        mkdir -p ${windows_mount_point}
    fi

    if [ $# -eq 3 ];then
        res_str=$(mount -t cifs -o username=${usr_name} ${windows_remote_addr} ${windows_mount_point} 2>&1)
    elif [ $# -eq 4 ];then
        passwd=$4
        res_str=$(mount -t cifs -o username=${usr_name},password=${passwd} ${windows_remote_addr} ${windows_mount_point} 2>&1)
    fi

    if [ -n "${res_str}" ];then
        echo ${res_str}
        exit
    fi

}

umount_windows_share_dir()
{
    res_str=$(df |grep ${windows_mount_point})
    if [ -n "${res_str}" ];then
        umount ${windows_mount_point}
    fi
}

attach_windows_usb_cdrom()
{
    ip_addr=$1
    file_path=$2
    usr_name=$3

    file_dir=$(dirname ${file_path})
    file_name=$(basename ${file_path})
    local_file_path="${windows_mount_point}/${file_name}"

    method_type=$(show_cdrom_attach_location)
    if [ "${method_type}" != "N/A" ];then
        echo "CDROM attached"
        exit
    fi

    case $# in
        3)
            mount_windows_share_dir ${ip_addr} ${file_dir} ${usr_name}
            ;;
        4)
            passwd=$4
            mount_windows_share_dir ${ip_addr} ${file_dir} ${usr_name} ${passwd}
            ;;
        *)
            echo "Invalid parameter"
            exit
            ;;
    esac

    if [ -f ${local_file_path} ];then
        res_str=$(modprobe g_mass_storage file=${local_file_path} cdrom=y 2>&1)
        if [ -n "${res_str}" ];then
            echo ${res_str}
            umount_windows_share_dir
            exit
        else
            echo "OK"
        fi
    else
        echo "File not found"
        umount_windows_share_dir
    fi
}

detach_usb_cdrom()
{
    method_type=$(show_cdrom_attach_location)
    if [ "${method_type}" == "N/A" ];then
        echo "CDROM not attached"
        exit
    fi

    res_str=$(rmmod g_mass_storage 2>&1)
    if [ -n "${res_str}" ];then
        echo ${res_str}
    else
        echo "OK"
        umount_windows_share_dir
    fi

}

remove_local_isofile()
{
    target_iso_file=$1
    if [ -f "${target_iso_file}" ];then
        target_iso_name=$(basename ${target_iso_file})
        current_in_use_iso_name=$(show_cdrom_name)
        if [ "${current_in_use_iso_name}" != "N/A" -a "${current_in_use_iso_name}" == "${target_iso_name}" ];then
            echo "ISO file in use"
        else
            rm ${target_iso_file}
        fi
    else
        echo "ISO file not found"
    fi
}

show_cdrom_status()
{
    res_str=$(lsmod |grep g_mass_storage)
    if [ -n "${res_str}" ];then
        echo "CDROM is occupied"
    else
        echo "CDROM is free"
    fi
}

show_cdrom_name()
{
    iso_name_file=/sys/module/g_mass_storage/parameters/file 
    msg=$(cat ${iso_name_file} 2&>/dev/null)
    if [ $? -eq 0 ];then
        echo ${msg##*/}
    else
        echo 'N/A'
    fi
}

show_cdrom_attach_location()
{
    res_str=$(show_cdrom_status)
    res_str=$(echo $res_str|grep 'occupied')
    if [ -n "${res_str}" ];then
        remote_res_str=$(df |grep ${windows_mount_point})
        if [ -n "${remote_res_str}" ];then
            echo $(df |grep ${windows_mount_point} | awk '{print $1}')
        else
            echo ${local_mount_point}
        fi
    else
        echo 'N/A'
    fi
}

if [ -z "$1" ];then
    usage 
    exit
fi

#get input parameters
while [ "$1" != ""  ];do
    case $1 in 
        -h | --help)
                        usage
                        exit
                        ;;
        -a | --attach)
                        attach_usb_cdrom $2
                        exit
                        ;;
        -A | --attach-windows)
                        attach_windows_usb_cdrom $2 $3 $4  $5
                        exit
                        ;;
        -d | --detach)
                        detach_usb_cdrom
                        exit
                        ;;
        -r | --remove-local-isofile)
                        remove_local_isofile $2
                        exit
                        ;;
        -s | --show-cdrom-status)
                        show_cdrom_status
                        exit
                        ;;
        -n | --show-cdrom-iso-name)
                        show_cdrom_name
                        exit
                        ;;
        -t | --show-cdrom-attach-location)
                        show_cdrom_attach_location
                        exit
                        ;;
        *)
                        echo "Invalid parameter"
                        usage
                        exit
                        ;;
    esac
done
