#!/bin/sh

#set -x
function error_exit()
{
		echo "$1"
		exit 1
}

BR2_ROOT=$(pwd)
IMAGE_PATH=$(pwd)
#search for buildroot in the path, and use it as BR2_ROOT if found
#if not found BR2_ROOT will be empty
while [ $BR2_ROOT != / ]; do
	if [ "buildroot" == $(basename $BR2_ROOT) ]; then
		break
	fi
	if [ -d "${BR2_ROOT}/buildroot" ]; then
		BR2_ROOT=${BR2_ROOT}/buildroot
		break
	fi
	BR2_ROOT=$(dirname $BR2_ROOT);
done
BASE_ROOT=${BR2_ROOT}/..
if [ $BR2_ROOT == "/" ]; then
	echo "This screscipt must be run in buildroot home, but we are not sure"
	BR2_ROOT=$(pwd)
	echo "Using current path as BR2_ROOT $(BR2_ROOT)"
fi

PRODUCT=$(cat ${BR2_ROOT}/output/target/etc/product.txt)
PRODUCT_SUB=$(cat ${BR2_ROOT}/output/target/etc/product_sub.txt 2>/dev/null)

#process -p and -U arg, this must be done before source is_image_env
path_image_output="output/images"
for arg in $* ; do
	if [ -z "${path_image_output}" ]; then
		path_image_output="${arg}"
		break;
	fi
	test ${arg} == "-p" && path_image_output=
done
test -z "${path_image_output}" && error_exit "image path not specified for -p"

path_image_uboot=${path_image_output}
for arg in $* ; do
	if [ -z "${path_image_uboot}" ]; then
		path_image_uboot="${arg}"
		break;
	fi
	test ${arg} == "-U" && path_image_uboot=
done
test -z "${path_image_uboot}" && error_exit "image path for uboot not specified for -U"

path_image_static="product/images"
path_version="output/target/etc/product/${PRODUCT}/version"

if [ -z "${PRODUCT}" ]; then
	error_exit "PRODUCT not defined, ${path_version}/product.txt missing or empty?"
fi

echo "Make uprade image for ${PRODUCT}"

IMG_TMP_PATH=/dev/shm/is_upgrade_build
custom_upgrade_script=

build_debug="0"

. ${BR2_ROOT}/product/scripts/upgrade/is_image_env

upgrade_version=$(cat ${BR2_ROOT}/${path_version}/${version_files[0]} 2>/dev/null|sed "s/#//g")

upgrade_image_name=${upgrade_version}.img
if [ $upgrade_image_name == ".img" ]; then
	upgrade_image_name="is.upgrade.img"
fi

key_id="silicom_is"

usage()
{
    echo "Usage: $0 [option]..."
    echo "Build a upgrade package for IS."
    echo "    -h | --help                display this help and exit"
    echo "    -p | --images-path <path>  input images path, default is ${path_image_output}"
    echo "    -c | --custom-upgrade <path>  inculde custom upgrdae script"
    echo "    -u | --uboot               include uboot"
    echo "    -f | --fman                include fman image"
    echo "    -r | --rcw                 include rcw image"
    echo "    -a | --all                 include all images"
    echo "    -y | --yes                 answer y automatically"
    echo "    -o | --output <name>       output file name, default is ${upgrade_image_name}"
	echo "    -V | --vendor <name>       new vendor to be programed"
	echo "    -O | --vendor-old <name>   old vendor to be matched for the new vendor to take effect"
	echo "    -x | --vendor-only <name>  the image is to be vendor only"
    echo "    -d | --debug               include debug info"
}

clear_files()
{
    echo "rm -f $uimage_name $rootfs_name $rootfs_name* $dtb_name $uboot_name $fman_name $rcw_name"
    rm -f $uimage_name $rootfs_name $rootfs_name* $dtb_name $uboot_name $fman_name $rcw_name
}

# get input parameters
while [ "$1" != "" ]; do
    case $1 in
        -h | --help )                       usage
                                            exit;
                                                  ;;
        -b | --buildroot )                  shift
                                            BR2_ROOT=$1
                                                  ;;
        -p | --images-path )                shift
											#this is actually handled at the beginning.
                                            path_image_output=$1
                                                  ;;
        -c | --custom-upgrade )             shift
                                            custom_upgrade_script=$1
                                                  ;;
        -u | --uboot )                      
                                            enabled[3]="1"
                                                  ;;
        -f | --fman )                       
                                            enabled[4]="1"
                                                  ;;
        -r | --rcw )                        
                                            enabled[5]="1"
                                                  ;;
        -a | --all )                        
                                            for ena_idx in $(seq 3 ${img_idx_max}); do
											    enabled[${ena_idx}]="1";
											done
                                                  ;;
        -y | --yes )                        
											DEFAULT_YES="1"
                                                  ;;
        -V | --vendor )                        
                                            shift || error_exit "Must specify vendor name"
                                            enabled[${vendor_idx}]="1"
											vendor=$1
                                                  ;;
        -O | --vendor-old )                        
                                            shift || error_exit "Must specify vendor old"
                                            enabled[${vendor_old_idx}]="1"
											vendor_old=$1
                                                  ;;
        -x | --vendor-only )                        
                                            enabled[0]="0"
                                            enabled[1]="0"
                                            enabled[2]="0"
                                                  ;;
        -d | --debug )                        
                                            build_debug="1"
                                                  ;;
        -o | --output )                     shift
                                            upgrade_image_name=$1
    esac
    shift
done

if [ ${enabled[${vendor_idx}]} != ${enabled[${vendor_old_idx}]} ]; then
	echo "--vendor must be used with --vendor-old"
	exit 1
fi


if [ -z "$BR2_ROOT" ]; then
	echo "Can't find buildroot in parent directory, use -b to select, use -h for help"
	exit 1
fi

which realpath &>/dev/null
if [ $? == 0 ]; then
	BR2_ROOT=$(realpath $BR2_ROOT)
fi

echo "Using $BR2_ROOT as buildroot path"
for idx in $(seq 0 ${img_idx_max}); do 
	version_files_path[$idx]="${BR2_ROOT}/${path_version}/${version_files[$idx]}"
done

if [ ${enabled[${vendor_idx}]} -eq 1 ]; then
	echo "##${vendor}##">${BR2_ROOT}/${src_fnames[${vendor_idx}]}
	echo "##${vendor_old}##">${BR2_ROOT}/${src_fnames[${vendor_old_idx}]}
fi

# show info
echo -e "Creating IS upgrade package: ${upgrade_image_name}\n\n"

if [ ! -z ${custom_upgrade_script} ]; then
	if [ ! -f "${custom_upgrade_script}" ]; then
		echo "Custom upgrade script invalid: ${custom_upgrade_script}"
		exit 1
	else
		echo -e "Custom Upgrade Script:\n\t${custom_upgrade_script}"
	fi
fi

echo "Version files:"
for idx in $(seq 0 ${img_idx_max}); do 
	if [ ${enabled[$idx]} -eq 1 ]; then
		echo -e "\t${infos[$idx]} file:\t${version_files_path[$idx]}"
	fi
done
echo "Include images:"
for idx in $(seq 0 ${img_idx_max}); do 
	if [ ${enabled[$idx]} -eq 1 ]; then
		version_str=
		if [ ! -z "${version_files_path[$idx]}" ]; then
			version_str=$(cat ${version_files_path[$idx]}|sed "s/##//g")
		fi
		echo -e "\t${infos[$idx]} version:\t${version_str}\t${BR2_ROOT}/${src_fnames[$idx]}"
	fi
done

if [ ${enabled[${vendor_idx}]} -eq 1 ]; then
	version_str=$(cat ${BR2_ROOT}/${src_fnames[${vendor_idx}]}|sed "s/##//g")
	echo -e "\t${infos[${vendor_idx}]} ${version_str}\t\t${BR2_ROOT}/${src_fnames[${vendor_idx}]}"
	version_str=$(cat ${BR2_ROOT}/${src_fnames[${vendor_old_idx}]}|sed "s/##//g")
	echo -e "\t${infos[${vendor_old_idx}]} ${version_str}\t\t${BR2_ROOT}/${src_fnames[${vendor_old_idx}]}"
fi

if [ "${DEFAULT_YES}" != "1" ]; then
yinput=""
while [[ "$yinput" != "y" && "$yinput" != "n" ]]; do
	echo -e "Do you want continue(y|n)?\c"
	read -n1 yinput;
done
fi

echo
if [ "$yinput" == "n" ]; then
	exit 
fi

echo ""

# check images file
for idx in $(seq 0 ${vendor_old_idx}); do 
	if [ ${enabled[$idx]} -ne 1 ]; then
		continue
	fi
	if [ $idx -lt ${vendor_idx} ]; then
		# except for vendor/vendor_old, everything has a version
		if [ ! -f "${version_files_path[$idx]}" ]; then
			echo "File ${version_files_path[$idx]} doesn't exist"
			exit 1
		fi
	fi
	if [ ! -f "${BR2_ROOT}/${src_fnames[$idx]}" ]; then
		echo "File ${BR2_ROOT}/${src_fnames[$idx]} doesn't exist"
		exit 1
	fi
	src_file_size=$(ls -l "${BR2_ROOT}/${src_fnames[$idx]}"|awk '{print $5}')
	if [ ${src_file_size} -gt "${MAX_ROOTFS_SIZE}" ]; then
		echo "File ${BR2_ROOT}/${src_fnames[$idx]} size($rootfs_size) is too big, max is ${MAX_ROOTFS_SIZE}"
		exit 1
	fi
done


rm -rf ${IMG_TMP_PATH}
mkdir -p ${IMG_TMP_PATH}

if [ ! -d ${IMG_TMP_PATH} ]; then
	echo "Failed to create working path ${IMG_TMP_PATH}"
	exit 1
fi

for idx in $(seq 0 ${vendor_old_idx}); do 
	if [ ${enabled[$idx]} -ne 1 ]; then
		continue
	fi
	if [ "${SPLIT_ROOTFS}" == "1" ] && [ ${dst_fnames[$idx]} == "rootfs.cpio.uboot" ]; then
		cd ${IMG_TMP_PATH}
		split -b $split_size -d ${BR2_ROOT}/${src_fnames[$idx]} ${dst_fnames[$idx]}.
		cd ${BR2_ROOT}
		file_list="$file_list ${dst_fnames[$idx]}.* ${version_files[$idx]}"
		cp -f ${version_files_path[$idx]} ${IMG_TMP_PATH}/
	else
		file_list="$file_list ${version_files[$idx]} ${dst_fnames[$idx]}"
		if [ $idx -lt 6 ]; then
			cp -f ${version_files_path[$idx]} ${IMG_TMP_PATH}/
		fi
		cp -f ${BR2_ROOT}/${src_fnames[$idx]} ${IMG_TMP_PATH}/${dst_fnames[$idx]}
	fi
done

if [ ! -z ${custom_upgrade_script} ]; then
	cp -f  ${custom_upgrade_script}  ${IMG_TMP_PATH}/custom_upgrade.sh
	file_list="$file_list custom_upgrade.sh"
fi

cd $IMG_TMP_PATH
tmp_tar_name="upgrade.tmp.tar"
# tar
echo "Create a temp archive."
echo "$file_list"
tar -cf $tmp_tar_name $file_list || exit 1

# sign and encrypt
echo "Sign and encrypt the temp archive."
echo "Silicom2015"|gpg --batch --passphrase-fd 0 --sign $tmp_tar_name >/dev/null || \
	error_exit "encryption failed, you will need to generate a private gpg key for encyrption. \nAnd import the key by the following command:\n gpg --import scripts/silicom-is-private.key"

# move the file to output
echo "Sign and encrypt the temp archive."
mv -f "$tmp_tar_name.gpg" ${IMAGE_PATH}/${upgrade_image_name} >/dev/null || exit 1

# clean env
echo "Clean the env."
#rm -f $file_list $tmp_tar_name > /dev/null

# build debug info
if [ ${build_debug} -eq 1 ]; then
	echo "Build the debug info."
	cd ${IMAGE_PATH}/
	debug_dir=${upgrade_version}.debuginfo
	rm -rf ${debug_dir}
	mkdir -p ${debug_dir}/build
	mkdir -p ${debug_dir}/staging
	if [ -n "$(ls ${BR2_ROOT}/output/build/|grep xgssdk)" ]; then
		cp -ar ${BR2_ROOT}/output/build/xgssdk-* ${debug_dir}/build/
	fi
	cp -ar ${BR2_ROOT}/output/build/silc_* ${debug_dir}/build/
	cp -ar ${BR2_ROOT}/output/build/is_* ${debug_dir}/build/
	cp -ar ${BR2_ROOT}/output/staging/* ${debug_dir}/staging/
	if [ "${PRODUCT}" == "UBMC" ]; then
		cp -ar ${BR2_ROOT}/output/build/dying_* ${debug_dir}/build/
	fi
	tar -cf ${upgrade_version}.debuginfo.tar ${debug_dir}
	gzip ${upgrade_version}.debuginfo.tar
	#mv -f ${upgrade_version}.debuginfo.tar.gz ${IMAGE_PATH}/
	rm -rf ${debug_dir}
fi

echo "Build upgrade package OK."





