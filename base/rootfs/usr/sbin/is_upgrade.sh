#!/bin/sh

#set -x

. /usr/sbin/silc_util.sh

print_step="0"
debug_on="0"
force="0"
PRODUCT=$(get_product_name)
PRODUCT_SUB=$(get_product_sub_name)
export PRODUCT
export PRODUCT_SUB

work_path=/tmp/upgrade/is_upgrade_path


# input parameters
public_key="/etc/silicom-is-public.key"

#source upgrade env for specific product
. /etc/prod_upgrade/upgrade_var
#source upgrade ops for specific product
#these include the following
#is_upgrade_get_flag $1       #arg1 is flag name
#is_upgrade_set_flag $1 $2  #arg1 is flag name, arg2 is value
#is_upgrade_get_flash_to_file $1 $2 $3 #arg1 is flash name, arg2 is file dest, arg3 is error message
#is_upgrade_burn_flash $1 $2 $3 $4 $5 
    #arg1 is src img_file
    #arg2 is flash destination
    #arg3 is info the display
    #arg4 is progress percent in/out
    #arg5 is load factor
    
. /etc/prod_upgrade/upgrade_ops

tmp_tar="tmp_upgrade.tar"
tmp_file="/tmp/is_upgrade_flash_tmp.txt"
status_file="/tmp/is_upgrade.status"
log_file="/tmp/is_upgrade.log"
pid_file="/tmp/is_upgrade.pid"


device_name=""

#########################################################
# version must start with ## and end with ##
# boot flag(0|1) select which image bank will be used.
# upgrade flag, 0: normal, without upgrade (after burn the upgrade image, app will change it to 1, and change boot flag also.)
#               1: upgrade finished, need check (uboot will change it to 2)
#               2: start check (after boot up, app will change it to 0. uboot think upgrade is failed, will change it to 0, and downgrade)
###############################3
# output
output_status()
{
    if [ "$print_step" == "1" ]; then
        if [ "$debug_on" == "1" ]; then
            echo -e "[$1%]\t$2"
        else
            echo -e "\r                                                         \c"
            echo -e "\r[$1%]\t$2\c"
        fi
    fi
    if [ "$3" == "" ]; then
        echo -e "$2\n$1" > $status_file
        echo -e "$2\n$1" >> /tmp/ug_debug_save
    fi
}

error_quit()
{
    echo -e "\nError: $1"
    if [ $2 ]; then
        echo -e "Error: $1\n$2" > $status_file
        echo -e "Error: $1\n$2" >> /tmp/ug_debug_save
    fi
    exit 1
}

show_flags()
{
    software_boot_flag=$(is_upgrade_get_flag $software_boot_flag_addr)
    software_upgrade_flag=$(is_upgrade_get_flag $software_upgrade_flag_addr)
    echo "software boot flag:$software_boot_flag"
    echo "software upgrade flag:$software_upgrade_flag"
}

clear_flags()
{
    is_upgrade_set_flag $software_boot_flag_addr "0"
    is_upgrade_set_flag $software_upgrade_flag_addr "0"
}

# version
get_version_from_flash()
{
    #read a flash location into a tmp file
    is_upgrade_get_flash_to_file $1 $tmp_file $2
    ver_str=$(cat $tmp_file|grep "##")
    if [ -z "$ver_str" ]; then
        echo "none"
    else
        echo $(cat $tmp_file|sed "s/##//g"|head -n 1)
    fi
}

get_devname()
{
    product=$(get_product_name)
    vendor_curr=$(get_version_from_flash ${vendor_addr})
	devname_file=/etc/prod_defconfig/vendor_config/${vendor_curr}/${vendor_curr}.devname
	if [ -e ${devname_file} ]; then
	    device_name=$(cat ${devname_file}|head -n 1)
	fi
}

translate_version()
{
    oldver=$1
    newver=${oldver/SILICOM_/}
    product=$(get_product_name)
    if [ "" != "${device_name}" ]; then
        newver=${newver/${product}/${device_name}}
    fi
    echo ${newver}
}

get_version()
{
    ver_str=$(get_version_from_flash $1)
    echo $(translate_version ${ver_str})
}

show_versions()
{
    get_devname

    boot_flag=$(is_upgrade_get_flag $software_boot_flag_addr)
    echo "Current booting bank: $boot_flag"
    if [ "$boot_flag" == "0" ]; then
        software_cur_ver=$(get_version ${sw_bank0_ver_addr})
        software_back_ver=$(get_version ${sw_bank1_ver_addr})
    else
        software_cur_ver=$(get_version ${sw_bank1_ver_addr})
        software_back_ver=$(get_version ${sw_bank0_ver_addr})
    fi
    echo "Software current version: $software_cur_ver"
    echo "Software backup version: $software_back_ver"
    for idx in ${firmware_parts}; do
        echo "$idx version: $(get_version $(attrib_get $idx ver_addr))"
    done
    vendor_curr=$(get_version_from_flash ${vendor_addr})
    echo "Vendor: $vendor_curr"
}

show_vendor()
{
	get_version_from_flash ${vendor_addr}
}

show_upgrade_versions()
{
	if [ ! -e ${work_path} ]; then
		error_quit "${work_path} doesn't exist"
	fi
    cd ${work_path}
    get_devname
	for idx in ${package_parts}; do
		ver_file=$(attrib_get $idx ver_name)
		ver_name=$(attrib_get $idx info)
		if [ -e $ver_file ]; then
			ver_str=$(cat $ver_file|sed "s/##//g")
			if [ "vendor" == "$idx" ]; then
				echo "${ver_name}: ${ver_str}"
			else
				echo "${ver_name}: $(translate_version ${ver_str})"
			fi
		fi
	done
}

# upgrade
verify_package()
{
    package_file=$(realpath $1)

    is_upgrade_prepare_workpath

    cd ${work_path}
    # import public key
    output_status "1" "Import gpg public key" "1"
    gpg --import ${public_key} &> /dev/null|| error_quit "import public key failed, check your system time"

    # verify and decrypt
    output_status "50" "Decrypt upgrade package" "1"
    gpg --decrypt ${package_file} 1> $tmp_tar 2>/dev/null || error_quit "Invalid upgrade package: Corrupted content"

    # unpack
    output_status "80" "Unpack package" "1"
    tar -xf $tmp_tar >/dev/null || error_quit "Invalid upgrade package: Corrupted content"

    # check file
    output_status "90" "Check file" "1"
	check_sw=1
	if [ "$force" -ne "1" ]; then
	if [ -f ${vendor_name} ]; then
		vendor_curr=$(get_version_from_flash ${vendor_addr})
		vendor_old=$(cat ${vendor_old_name}|sed "s/##//g"|head -n 1)
		if [ "$vendor_curr" == "none" ]; then
			# no existing vendor, this is ok then.
			vendor_curr=${vendor_curr}
		else
			if [ "$vendor_curr" != "${vendor_old}" ]; then
				# has exsting vendor, and not mathcing specified old vendor name
				error_quit "Invalid upgrade package: Invalid vendor information"
			fi
		fi
		#vendor tranistion image doesn't need software
		check_sw=0
		#check software only if we have a kernel iamge
		ls $sw_ver_name &>/dev/null && check_sw=1
		ls $uimage_name &>/dev/null && check_sw=1
		ls $rootfs_name.* &>/dev/null && check_sw=1
	fi
	fi
	if [ "$check_sw" -eq "1" ]; then
	    ls $sw_ver_name &>/dev/null || error_quit "Invalid upgrade package: version information not found"
		ls $uimage_name &>/dev/null || error_quit "Invalid upgrade package: Missing files"
		#check rootfs file, also check legacy split files
		if [ ! -f $rootfs_name ]; then
		    ls $rootfs_name.* &>/dev/null || error_quit  "Invalid upgrade package: Missing files"
		fi
	fi

    output_status "100" "Done" "1"
    if [ $debug_on == "1" ]; then
        echo ""
    fi

	#deal with legacy split files
	if [ -f ${rootfs_name}.00 ]; then
		cat ${rootfs_name}.*  >> ${rootfs_name} || error_quit "not enough space"
		rm -rf ${rootfs_name}.*
	fi
	if [ "$print_step" == "0" ] ;then
		show_upgrade_versions
	fi
}


clear_file()
{
	if [ "$manu_doing" != "" ]; then
	#	echo "manu upgrade in processing, don't clear the path and files"
		return 0
	fi
    rm -f $uimage_name
	rm -rf ${work_path}
}

log_print()
{
	echo $1
	logger $1
}

upgrade_software()
{
    # get upgrade flag
	if [ ! -d ${work_path} ]; then
		error_quit "no file to upgrade"
	fi
	cd ${work_path}
	#upgrade hook script inside the upgrade package
	if [ -e ${work_path}/custom_upgrade.sh ]; then
		echo "0" "Upgrade Maintenencee"
		chmod a+x ${work_path}/custom_upgrade.sh
		${work_path}/custom_upgrade.sh -u
		if [ ! -z $(cat /tmp/custom_upgrade.status) ]; then
			if [ $(cat /tmp/custom_upgrade.status) == "done" ]; then
				#success_ful
    			clear_file
				return 0
			fi
			if [ $(cat /tmp/custom_upgrade.status) == "error" ]; then
				return 1
			fi
		fi
	fi
    output_status "0" "Start to upgrade"
    output_status "1" "Check upgrade flag"
    upgrade_flag=$(is_upgrade_get_flag $software_upgrade_flag_addr)
	if [ ! -z "$upgrade_flag" ]; then
	    if [ ! $upgrade_flag -eq "0" ]; then
    	    error_quit "upgrade flag is already upgraded, reboot first" "0"
	        exit 1
	    fi
	else
		output_status "2" "Setting up upgrade environment"
	fi
    # get boot flag
    output_status "2" "Check boot flag"
	new_boot_flag="0"

    boot_flag=$(is_upgrade_get_flag $software_boot_flag_addr)
	if [ -z "$boot_flag" ]; then
		output_status "2" "Fresh system installation"
    elif [ -f "$uimage_name" ]; then
		#only switch when software available
		if [ "$boot_flag" == "0" ]; then
			#switch to bank1
		    uimage_addr=$uimage_bank1_addr
		    rootfs_addr=$rootfs_bank1_addr
		    ver_addr=$sw_bank1_ver_addr
			dtb_addr=$dtb_bank1_addr
			sw_ver_addr=$sw_bank1_ver_addr
		    new_boot_flag="1"
		fi
	else
		new_boot_flag="$boot_flag"
    fi

	percent=3
    #this 
    for idx in ${upgrade_parts}; do
        if [ -f $(attrib_get $idx name) ]; then
			wr_load=$(attrib_get $idx load)
			if [ -z $wr_load ]; then wr_load=1; fi
			idxinfo=$(attrib_get $idx info)
	        output_status $percent "Updating $idxinfo"
	        is_upgrade_burn_flash $(attrib_get $idx name) $(attrib_get $idx addr) ${idxinfo} ${percent} ${wr_load}
	    fi
	done

    output_status $percent "Setup next bootup bank"
    is_upgrade_set_flag $software_boot_flag_addr $new_boot_flag $percent
    # change upgrade flag
    is_upgrade_set_flag $software_upgrade_flag_addr "1" $percent
    output_status "100" "Done"

	echo ""
    log_print "System upgrade successfully."

    boot_flag=$(is_upgrade_get_flag $software_boot_flag_addr)
    log_print "Current booting bank $boot_flag"

    get_devname

    if [ "$boot_flag" == "0" ]; then
        software_cur_ver=$(get_version ${sw_bank0_ver_addr})
        software_back_ver=$(get_version ${sw_bank1_ver_addr})
    else
        software_cur_ver=$(get_version ${sw_bank1_ver_addr})
        software_back_ver=$(get_version ${sw_bank0_ver_addr})
    fi
    log_print "Software upgrade version: $software_cur_ver"
    log_print "Software backup version: $software_back_ver"

    clear_file
}


confirm_software_upgrade()
{
    # clear the uploaded pkg in flash when start
	rm -rf /var/log/tmp
	mkdir -p /var/log/tmp
	chmod 777 /var/log/tmp

    software_upgrade_flag=$(is_upgrade_get_flag $software_upgrade_flag_addr)
    if [ "$software_upgrade_flag" == "0" ]; then
        exit 0
    fi
    if [ "$software_upgrade_flag" == "2" ]; then
        echo "Confirm software upgrade flag."
        is_upgrade_set_flag $software_upgrade_flag_addr "0"
        exit 0
    fi
    echo "Upgrade flag($software_upgrade_flag) is error, reset it."
    is_upgrade_set_flag $software_upgrade_flag_addr "0"
    exit 0
}

switch_software()
{
    upgrade_flag=$(is_upgrade_get_flag $software_upgrade_flag_addr)
    if [ ! -z "$upgrade_flag" ]; then
        if [ ! $upgrade_flag -eq "0" ]; then
                error_quit "Please reboot the system to fininsh last upgrade" "0"
            exit 1
        fi
    else
            output_status "2" "Setting up upgrade environment"
    fi
    new_boot_flag="0"
    boot_flag=$(is_upgrade_get_flag $software_boot_flag_addr)
    if [ -z "$boot_flag" ]; then
            error_quit "Fresh system installation" "0"
    elif [ "$boot_flag" == "0" ]; then
        #switch to bank1
        new_boot_flag="1"
    fi
    is_upgrade_set_flag $software_boot_flag_addr $new_boot_flag
    # change upgrade flag
    is_upgrade_set_flag $software_upgrade_flag_addr "1" 
}

set_flag_value()
{
    # $1: flag_type, $2: flag_value
    echo "setting flag type: $1 to value: $2"
    if [ "$2" != "" ]; then
        if [ "$1" == "fw-boot" ]; then
            is_upgrade_set_flag $software_boot_flag_addr $2
            return 0
        fi
        if [ "$1" == "fw-upgrade" ]; then
            is_upgrade_set_flag $software_upgrade_flag_addr $2
            return 0
        fi
        echo "Unknown flag type: $1"
        exit 1;
    fi
}

upgrade_manufacture()
{
    echo "Start manufacture upgrade on both bank 0 & bank 1"
    echo "Before manufacture upgrade, show version"
    #echo "-------------------------------------------"
    show_versions
    #echo "-------------------------------------------"  
    echo "Installing image on bank 0"
    manu_doing="doing"
    set_flag_value "fw-boot" 1
    set_flag_value "fw-upgrade" 0    
    
    upgrade_software #upgrade
    
    echo "Bank 0 installation done"
    echo "Installing image on bank 1"
    set_flag_value "fw-boot" 0
    set_flag_value "fw-upgrade" 0
    
    upgrade_software  #upgrade
    
    echo "Bank 1 installation done"
    #echo "-------------------------------------------"
    
    # reset the flag_type & flag_value
    set_flag_value "fw-boot" 0 
    set_flag_value "fw-upgrade" 0

    echo "Installation of ${PRODUCT} image finished, both banks upgraded"
    manu_doing=""
    manu_done="YES"
}

show_upgrade_state()
{
	#return values:
	#0: ok to do upgrade
	#1: upgrade is running
	#2: upgrade is complete
	if [ -e ${status_file} ]; then
		if [ "$(cat ${status_file}|grep Done)" != "" ]; then
			exit 2
		fi
	fi
	if [ -e ${pid_file} ]; then
		up_pid=$(cat ${pid_file})
		kill -0 ${up_pid} 2>/dev/null
		if [ "$?" == "0" ]; then
			exit 1
		fi
	fi
	exit 0
}

show_upgrade_realtime()
{
	count=0
	while [ true ]; do
		if [ -e ${log_file} ]; then
			break
		fi
		usleep 1000
		count=$((count+1))
		if [ "${count}" == "3000" ]; then
			echo "${log_file} not found"
			exit 0
		fi
	done

	tail -f ${log_file} &
	tail_pid=$!

	up_pid=$(cat ${pid_file})
	while [ true ]; do
		kill -0 ${up_pid} 2>/dev/null
		if [ "$?" != "0" ]; then
			break
		fi
		sleep 1
	done

	sleep 1

	kill ${tail_pid}
	exit 0
}

usage()
{
    echo "Usage: $0 [option]..."
    echo "Upgrade IS."
    echo "    -h | --help                  display this help and exit"
    echo "    -i | --init <package>        init system"
    echo "    -s | --show-flag             show all flags"
    echo "    -S | --show-version          show version"
    echo "    -U | --show-upgrade-version  show upgrade version"
    echo "    -c | --clear-flag            clear all flags"
    echo "    -p | --print-step            print command step information"
    echo "    -d | --debug-on              only debug, not write to flash"
    echo "    -C | --confirm               confirm upgrade flags"
    echo "    -V | --verify <package>      verify the upgrade package"
    echo "    -u | --upgrade               upgrade the special package, need verify first"
    echo "    -w | --switch-software       switch software"
    echo "    -t | --flag-type <type>      special flag type, default is fw-boot, only for debug"
    echo "    -v | --flag-value <0|1|2>    flag value, used with -t, only for debug"
    echo "    -N | --device-name           show vendor device name"
    echo "    -o | --vendor-name           show vendor name"
    echo "    -M | --manufacture           upgrade both banks on manufacturer's demand"
    echo "    -f | --force                 force upgrade both banks"
    echo "    -a | --state                 show upgrade state (0:ok; 1: running; 2: complete)"
    echo "    -r | --realtime              show upgrade realtime log"
}

flag_type="fw-boot"
flag_value=""
init_pkg=""
manu_doing=""
manu_done=""

# get input parameters
while [ "$1" != "" ]; do
    case $1 in
        -h | --help )                       usage
                                            exit;
                                                  ;;
        -i | --init )                       shift
                                            init_pkg=$1
                                                  ;;
        -s | --show-flag )
                                            show_flags
                                            exit;
                                                  ;;
        -S | --show-version )
                                            show_versions
                                            exit;
                                                  ;;
        -U | --show-upgrade-version )
                                            show_upgrade_versions
                                            exit;
                                                  ;;
        -c | --clear-flag )
                                            clear_flags
                                            exit;
                                                  ;;
        -p | --print-step )
                                            print_step="1"
                                                  ;;
        -d | --debug-on )
                                            debug_on="1"
                                                  ;;
        -C | --confirm )
                                            confirm_software_upgrade
                                            exit;
                                                  ;;
        -V | --verify )                     shift
                                            package=$1
                                                  ;;
        -u | --upgrade )
                                            upgrade="1"
                                                  ;;
        -w | --switch-software )
                                            switch_software
                                                  ;;
        -t | --flag-type )                  shift
                                            flag_type=$1
                                                  ;;
        -v | --flag-value )                 shift
                                            flag_value=$1
                                                  ;;
        -o | --vendor-name )
                                            show_vendor
                                            exit;
                                                  ;;
        -N | --device-name )
                                            get_devname               
                                            echo ${device_name}                                            
                                            exit;                                          
                                                  ;;
		-f | --force   )                    force="1"
											   	  ;;
        -M | --manufacture )
                                            upgrade_manufacture               
                                            #echo "upgrade both banks"                                            
                                            exit;                                          
                                                  ;;
        -a | --state )
                                            show_upgrade_state
                                            exit;
                                                  ;;
        -r | --realtime )
                                            show_upgrade_realtime
                                            exit;
                                                  ;;
    esac
    shift
done

if [ "${init_pkg}" != "" ]; then
    clear_flags
    debug_on="1"
    echo "Start verify package ${init_pkg}"
    verify_package ${init_pkg}
    echo "Start upgrade."
    upgrade_software
    exit 0
fi

if [ "$package" != "" ]; then
    verify_package $package
    exit 0
fi

if [ "$manu_done" != "" ]; then
    echo "upgrade manufacture already done"
    exit 0
fi

if [ "$upgrade" != "" ]; then
    upgrade_software
    exit 0
fi

set_flag_value $flag_type $flag_value
echo "---- finished -----"

