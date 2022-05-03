FROM ubuntu:18.04

RUN apt update && \
    apt dist-upgrade

RUN apt install -y \
    vim \
    ctags \
    wget \
    cpio \
    python \
    unzip \
    rsync \
    bc \
    git \
    libterm-readline-perl-perl \
    build-essential 

RUN apt install -y \
    libxml2 \
    libxml2-dev \
    libxslt1.1 \
    libxslt1-dev \
    xsltproc \
    python-setuptools 

RUN apt install -y \
    flex \
    bison \
    libtool \
    libtool-bin \
    autoconf \
    automake \
    libreadline-dev \
    libssh2-1 \
    libssh2-1-dev \
    libssh-4 \
    libssh-dev \
    doxygen \
    libcurl4-openssl-dev \
    python-libxml2 \
    python-lxml \
    libfdt-dev \ 
    libfdt1

WORKDIR /srv/ubmc

COPY dl/pyang-1.7.4.tar.gz .

RUN tar -xvpf pyang-1.7.4.tar.gz && \
    cd pyang-1.7.4 && \
    python setup.py install && \
    cd .. && rm -rf pyang-1.7.4

COPY dl/libnetconf-master.tar.gz .

RUN tar -xvpf libnetconf-master.tar.gz && \
    cd libnetconf-master && \
    autoreconf -f -i && \
    ./configure && \
    make && \
    make install && \
    cd .. && rm -rf libnetconf-master

