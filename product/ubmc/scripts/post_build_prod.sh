#!/bin/bash

set +e

sed -i "s/SILC_PRODUCT_TTY/ttyF0/g" ${TARGET_DIR}/etc/inittab


# copy phonehome cfg from file svr
phonehome_cfg_scp_url="file:/home/public/SILICOM/uBMC/ATT/phonehome"
scp -r ${phonehome_cfg_scp_url} ${TARGET_DIR}/etc/
if [ "$?" == "0" ]; then
	echo "PhoneHome configuration is downloaded from ${phonehome_cfg_scp_url}"
else
	echo "WARNING: PhoneHome configuration is unavailable from ${phonehome_cfg_scp_url}!!!!!!"
fi

exit 0
