#!/bin/sh

umount $(dirname ${IMG_FILE})
umount /var/log || echo "/var/log may not be mounted"

