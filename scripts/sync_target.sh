#!/bin/bash


if [ -z $1 ]; then
  echo "Usage:"
  echo -e "\t$0 <ipaddr>"
  exit 1
fi

BR2_ROOT=$(pwd)
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

if [ $BR2_ROOT == "/" ]; then
        BR2_ROOT=""
fi

if [ ! -z ${BR2_ROOT} ]; then
  cd ${BR2_ROOT}
fi
cd output/target

TGT=$1
ssh root@${TGT} killall is_snmpd
ssh root@${TGT} killall is_switchd
ssh root@${TGT} killall is_mgmtd
ssh root@${TGT} /etc/init.d/S50is_web stop

for mmm in `find usr|grep /is_`; do scp $mmm root@${TGT}:/$mmm; done
for mmm in `find usr|grep xgs`; do scp $mmm root@${TGT}:/$mmm; done
for mmm in `find usr|grep bcm`; do scp $mmm root@${TGT}:/$mmm; done
for mmm in `find usr|grep silc`; do scp $mmm root@${TGT}:/$mmm; done

ssh root@${TGT} /etc/init.d/S45is_mgmtd restart
sleep 2
ssh root@${TGT} /etc/init.d/S50is_web restart
ssh root@${TGT} /etc/init.d/S59is_snmp restart
