# uBMC
uBMC - open source remote management platform for edge devices, appliances.
## Getting the code
Simply clone the repo by 
```
git clone https://github.com/silicom-ltd/uBMC.git
```

## Compile the code
Simply call make in the checked out code base.
The first time make is called, you will be asked a few simply questions on what to make.

```
Select product to build. Press enter to select [ubmc]:    <-- simply press enter

Building product ubmc
Please Select a config as default config. If buildroot/.config doesn't exist, it will be copied
Available options:
	 ubmc ubmc_eval ubmc_install
ubmc   <-- type ubmc and enter to select ubmc image to start the compile
```
***The make will take a very long time the first time, as it needs to download a lot of packages***
### access to buildroot options
The root make will pass any unknown target to buildroot, this allows easy access to buildroot targets. For exmaple.
```
make menuconfig
make linux-menuconfig
make linux-reconfigure
```

### path_toolchain.sh
This add the current toolchain path, and uBMC help scripts to the current PATH.
It is not required to source this to build, but it gives easy access to the toolchain.
to use it. 
```
. path_toolchain.sh
```

### Building release
After make is done. call is_build_upgrade_package.sh to build an image for installing or upgrading.
For building image containing uboot
is_build_upgrade_package.sh -a
For building system software update only image
is_build_upgrade_package.sh

### Building the install image
To build the install image, you need to check out the uBMC.git again into a different dir, and rebuild with ubmc_install image
**Tips**
Copying the buildroot/dl to the new checkout will save download time

```
Available options:
	 ubmc ubmc_eval ubmc_install
ubmc_install   <-- type ubmc_install and enter to select ubmc image to start the compile
```

### Build a sdcard for install
Use the below command to generate a tar for sd card release.
```
is_make_sd_tar.sh output/images/ output/images/ <your image name>
```
You will then get an tarbar for use to create an sd card. Simply untar it and use the tools inside. 
***Obviously you need a USB sd card reader and an sd card***


## Code base Structure
The uBMC project code base is roughly 4 parts. 
- base - source for base system management
- buildroot - imported buildroot for make management
- product/ubmc - source for ubmc related features
- scripts - various scripts for help code maintainence, building, releasing, etc.

### base
This contains src for the management system. The management system is a set of programs prividing CLI, web GUI management for basic system management.
- src - where all management software source code is.
- package - where buildroot based makefile are defined
- rootfs - rootfs overlay for the base system
- rotofs_install - a rootfs overlay for the installation image

### product/ubmc
This contains src for software modules providing the uBMC features. The path structure is similar to base, except for the below additions.
- hardware - defines different hardware board support
- configs - stores build configs

### buildroot
This is mostly standard buildroot, with very minor modifications. 


### patch_config and patch management
We do need to modify certain opensource packages for achieving different required features. Some of these changes requires time and effort to push upstream, and some other changes are not fit for general upstream. So we have a set of scripts for maintiaining these patches. 
```
regen_patch.sh
```




