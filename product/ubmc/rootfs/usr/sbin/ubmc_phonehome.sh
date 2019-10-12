#!/bin/sh
#
# Start/stop is_netconf
#

set +e
#set -x

PH_ENABLE=0
PH_RETRY=1

PH_PROG=/etc/phonehome/phonehome.sh

PH_DIR=/tmp/phonehome
PH_ENABLE_CFG=${PH_DIR}/phonehome.enable
PH_RETRY_CFG=${PH_DIR}/phonehome.retry

FAC_CFG=${PH_DIR}/ubmc-factory-config.xml
RAW_CFG=${PH_DIR}/ubmc-factory-config-raw.xml
SYNC_DONE=${PH_DIR}/phonehome.sync

CONFIG_TOOL=/etc/netopeer/ubmc/ubmc-nc-config
DS_FILE=/etc/netopeer/ubmc/datastore.xml

mkdir -p ${PH_DIR}

if [ ! -e ${PH_ENABLE_CFG} ]; then
	echo ${PH_ENABLE} > ${PH_ENABLE_CFG}
fi
if [ ! -e ${PH_RETRY_CFG} ]; then
	echo ${PH_RETRY} > ${PH_RETRY_CFG}
fi

phonehome_sync()
{
	echo "Disable CLI/web access"
	killall -q is_cli nginx

	echo "Restart NETCONF to sync phonehome config"
	cp ${FAC_CFG} ${DS_FILE}
	/etc/init.d/S48is_netconfd restart

	echo "Check whether phonehome config is synced"
	retry=0
	tmp_cfg=/tmp/ubmc-latest-config.xml
	while [ ${retry} -lt 5 ]; do
		sleep 1
		retry=$((${retry}+1))
		${CONFIG_TOOL} -o ${tmp_cfg}
		if [ "$(diff ${tmp_cfg} ${FAC_CFG})" == "" ]; then
			echo "Phonehome config sync complete"
			touch ${SYNC_DONE}
			break
		fi
	done

	${CONFIG_TOOL} -s
	logger "Reboot the system to apply config"
	reboot
}

phonehome_start()
{
	PH_ENABLE=$(cat ${PH_ENABLE_CFG})
	if [ "${PH_ENABLE}" != "1" ]; then
		return
	fi

	if [ ! -e ${PH_PROG} ]; then
		echo "PhoneHome program is not found"
		return
	fi

	if [ -e ${FAC_CFG} ] && [ -e ${SYNC_DONE} ]; then
		echo "PhoneHome is already done"
		return
	fi

	if [ -e ${FAC_CFG} ]; then
		echo "PhoneHome config is already downloaded"
		phonehome_sync
		return
	fi

	echo "Start PhoneHome process"

	RETRY_INTERVAL=60
	while true; do
		${PH_PROG} ${RAW_CFG}
		if [ $? == 0 ] && [ -e ${RAW_CFG} ]; then
			${CONFIG_TOOL} -i ${RAW_CFG} -o ${FAC_CFG}
			if [ $? == 0 ]; then
				echo "Factory config is ready"
				break
			else
				logger "Error: Factory config corrupted"
				rm -rf ${FAC_CFG}
			fi
		fi

		retry=0
		while [ ${retry} -lt ${RETRY_INTERVAL} ]; do
			if [ "$(cat ${PH_RETRY_CFG})" != "1" ]; then
				echo "Stop PhoneHome process"
				return
			fi
			sleep 1
			retry=$((${retry}+1))
		done
	done

	if [ -e ${FAC_CFG} ]; then
		phonehome_sync
	fi
}

phonehome_stop()
{
	echo 0 > ${PH_RETRY_CFG}
}

phonehome_enable()
{
	echo 1 > ${PH_ENABLE_CFG}
	echo 1 > ${PH_RETRY_CFG}
	rm -rf ${FAC_CFG} ${SYNC_DONE}
}

phonehome_disable()
{
	echo 0 > ${PH_ENABLE_CFG}
	echo 0 > ${PH_RETRY_CFG}
	rm -rf ${FAC_CFG} ${SYNC_DONE}
}

phonehome_status()
{
	if [ ! -e ${PH_PROG} ]; then
		echo "N/A"
	elif [ "$(cat ${PH_ENABLE_CFG})" != "1" ]; then
		echo "Disabled"
	elif [ -e ${SYNC_DONE} ]; then
		echo "Complete"
	else
		echo "Running"
	fi
}

case "$1" in
  start)
		phonehome_start
        ;;
  stop)
		phonehome_stop
        ;;
  enable)
		phonehome_enable
        ;;
  disable)
		phonehome_disable
        ;;
  status)
		phonehome_status
        ;;
  *)
        echo "Usage: $0 {start|stop|enable|disable|status}"
        exit 1
esac

