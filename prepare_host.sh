#!/bin/sh

rm -rf ubmc_host_build
mkdir -p ubmc_host_build
cd ubmc_host_build

if [ $(whoami) != root ]; then
	echo "must run as root"
	exit 1
fi
yum install -y gcc gcc-c++ autoconf automake libtool libssh-devel libcurl-devel perl-ExtUtils-MakeMaker.noarch python libxslt-devel ncurses-devel unzip flex bison

#pyang
which pyang
if [ $? -eq 0 ]; then
	echo "pyang has already been installed."
else
	echo "Installing pyang......"
	cp ../dl/pyang-1.7.4.tar.gz ./
	tar -zxvf pyang-1.7.4.tar.gz
	cd pyang-1.7.4
	python setup.py install
	cd ..
	echo "pyang is installed."
fi

#libxml2
if [ -d /usr/include/libxml2 ]; then
	echo "libxml2 has already been installed."
else
	echo "Installing libxml2......"
	cp ../dl/libxml2-2.9.7.tar.gz ./
	tar -zxvf libxml2-2.9.7.tar.gz
	cd libxml2-2.9.7
	./configure
	make
	make install
	cd ..
	echo "libxml2 is installed."
fi

#libxslt
if [ -d /usr/include/libxslt ]; then
	echo "libxslt has already been installed."
else
	echo "Installing libxslt......"
	cp ../dl/libxslt-1.1.29.tar.gz ./
	tar -zxvf libxslt-1.1.29.tar.gz
	cd libxslt-1.1.29
	./configure
	make
	make install
	cd ..
	echo "libxslt is installed."
fi

#libnetconf
which lnctool
if [ $? -eq 0 ]; then
	echo "libnetconf has already been installed."
else
	echo "Installing libnetconf......"
	cp ../dl/libnetconf-master.tar.gz ./
	tar -zxvf libnetconf-master.tar.gz
	cd libnetconf-master
	./configure
	make
	make install
	cd ..
	echo "libnetconf is installed."
fi

echo "uBMC host build env is ready"
cd ..
rm -rf ubmc_host_build

