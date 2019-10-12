include build/config.mk

MODULES = modules/* themes/*

OS:=$(shell uname)
MODULES:=$(foreach item,$(wildcard $(MODULES)),$(if $(realpath $(wildcard $(item)/Makefile)),$(item)))

export OS

.PHONY: all build luabuild clean host hostcopy hostclean

all: build

build: luabuild

luabuild:
	for i in $(MODULES); do HOST=$(realpath host) \
		SDK="$(shell test -f .running-sdk && echo 1)" make -C$$i luabuild; done

clean:
	for i in $(MODULES); do make -C$$i clean; done

host: build hostcopy

hostcopy:
	mkdir -p host
	for i in $(MODULES); do cp -pR $$i/dist/* host/ 2>/dev/null || true; done

hostclean: clean
	rm -rf host

