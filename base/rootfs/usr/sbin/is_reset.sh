#!/bin/sh
#set -x

/etc/init.d/S01logging stop

/etc/init.d/IS_S50nginx stop

#is_format_flash.sh

rm -rf /var/log/*
rm -rf /config/*
rm -rf /var/images/*

reboot

