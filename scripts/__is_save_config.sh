#!/bin/bash
if [ -f buildroot/.config ]; then
    echo -n "Saving buildroot/.config to $1 ? (y/n)"; 
    read yn; 
    case $yn in 
    [Yy]* ) cp -a buildroot/.config $1
			exit 0;;
    *) echo "Not copying"
		exit 1;;
    esac;
fi

