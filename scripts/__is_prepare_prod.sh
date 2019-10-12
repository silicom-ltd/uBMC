#!/bin/bash
#set -x
set -e
echo "Select product to build. Default is ubmc:"
read prodname
if [ -z "${prodname}" ]; then
    prodname=ubmc
    prodpath=../product/ubmc
else
    echo "Product is ${prodname}"
	while true; do
	    echo "Please select product source path (relative to buildroot or absolute path)"
	    read prodpath
    	if [ -z "${prodpath}" ]; then
        	echo "Invalid product source path"
	        continue
    	fi
		if [ ${prodpath:0:1} == "/" ]; then 
			result_path=${prod_path}
		else
			result_path=buildroot/${prodpath}
		fi
		if [ ! -d "${result_path}" ]; then
			echo "Invalid product source path ${prodpath}"
			continue
		fi
		test_ok=1
		for test_tgt in package src configs; do
			if [ ! -d "${result_path}/package" ]; then
				echo "Invalid product source path ${prodpath}, target dir doesn't have ${test_tgt} dir"
				test_ok=0
				break
			fi
		done
		if [ "${test_ok}" == "1" ]; then
			break;
		fi
	done
fi

cd buildroot
if [ ! -d ${prodpath} ]; then
    echo "Invalid product source path ${prodpath}"
    exit 1
fi
cd ..
echo "Building product ${prodname}"
ln -f -T -s ${prodpath} buildroot/product

config_flist=$(ls buildroot/product/configs)
for fname in ${config_flist}; do
	config_list="${config_list} $(basename $fname .config)"
done

echo "Please Select a config as default config. If .config doesn't exist, it will be copied"

while true; do
	echo "Available options:"
	echo -e "\t${config_list}"
	cfgname=
    read readcfgname
	for mmm in ${config_list}; do
		if [ "$mmm" == "$readcfgname" ]; then
			cfgname=${readcfgname}
			break
		fi
	done

	if [ -n "${cfgname}" ]; then
		break
	fi
	echo "Invalid option \< ${readcfgname} \>"
done

CFG_FILE=buildroot/product/configs/${cfgname}.config
if [ ! -f buildroot/.config ]; then
	echo "Copying ${CFG_FILE} to buildroot/.config"
	cp  buildroot/.config
else
	echo "NOT Copying ${CFG_FILE} to buildroot/.config"
fi
mkdir -p buildroot/output
echo "${cfgname}">buildroot/output/.product_cfg
echo "${prodname}">buildroot/output/.product


