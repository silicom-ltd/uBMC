#!/bin/sh
STAGING_DIR=$1
ln -snf ld-2.27.so ${STAGING_DIR}/lib/ld-linux-aarch64.so.1
#fix :can not link libnsl when build is_console_manage
ln -snf libnsl-2.27.so ${STAGING_DIR}/lib/libnsl.so
