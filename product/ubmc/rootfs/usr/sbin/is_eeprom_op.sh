#!/bin/sh

#set -x
eeprom_op_bin=/usr/bin/eeprom_op
eeprom_addr=/sys/bus/i2c/devices/0-0050/eeprom
read_type_raw="raw"
read_type_verbose="verbose"

usage()
{
    echo "Usage: $0 [variable name]   show the value of variable name"
    echo "       $0 -h                display this help and exit"
    echo "       $0 -v                show items in verbose"
    echo "       $0 -w [ file ]       raw writing from a file"
    echo "       $0 -t                write fake data into eeprom just for test"
}

read_eeprom()
{
    item=$1
    if [ -n "${item}" ];then
        res_str=$(${eeprom_op_bin} -r |grep "${item}")
        if [ -n "${res_str}" ];then
            echo ${res_str}|awk -F '=' '{print $2}'
        else
            echo "## Error: \"${item}\" not defined"
        fi
    else
        ${eeprom_op_bin} -r
    fi
}

read_eeprom_1()
{
	type=$1
	if [ "${type}" == "${read_type_verbose}" ];then
		 ${eeprom_op_bin} -v
	elif [ "${type}" == "${read_type_raw}" ];then
	 	 ${eeprom_op_bin} -r
	fi
}

write_eeprom()
{
    data_file=$1
    if [ -f "${data_file}" ];then
        if [ -f "${eeprom_addr}" ];then
            dd if=${data_file} of=${eeprom_addr} count=1 bs=256
        else
            echo "eeprom cannot be used"
        fi
    else
        echo "File cannot be found"
    fi
}

test_write_eeprom()
{ 
	${eeprom_op_bin} -w
}

if [ -z "$1" ];then
    read_eeprom
    exit
fi

#get input parameters
while [ "$1" != ""  ];do
    case $1 in
        -h | --help)
                        usage
                        exit
                        ;;
        -w | --raw-write)
                        write_eeprom $2
                        exit
                        ;;
        -v | --verbose-read)
                        read_eeprom_1 ${read_type_verbose}
                        exit
                        ;;
        -t | --test-for-write)
                        test_write_eeprom
                        exit
                        ;;
        *)
                        if [ -n "$(echo $1 |grep ^-)" ];then
                            echo "$0:option requires an valid argument"
                            echo "Try \'$0 --help\' for more information."
                        else
                            read_eeprom $1
                        fi
                        exit
                        ;;
    esac
done
