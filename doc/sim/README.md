# Overview

The eLua "simulator" is an eLua build that runs on top of Linux. It doesn't emulate any hardware peripherals and it contains only a restricted set of the standard eLua features, but it's very useful for testing core-related issues, as well as modules that don't have hardware dependencies. It also offers a lot of possibilities in terms of code instrumentation. Some of its features:

* Emulates the eLua terminal on the Linux console.
* The maximum RAM size can be configured compile time.
* The SD card can be emulated (see below).
* The WOFS can also be emulated.
* The ROM file system is fully supported.
* Compiles in 32-bit mode even on 64-bit systems to better emulate the real hardware on which eLua runs.

## Environment

The instructions in this document were tested on an Ubuntu 16.04.4 64-bit installation running on a Core-i7 CPU, but other Linux flavours and other hardware should work equally well.

## How to build

The simulator can't be built with the regular Linux toolchain, because eLua assumes that the toolchain used for building is using Newlib. Consequently, it needs a Newlib-based toolchain that can generate code for x86. Such a toolchain used to be available for download from CodeSourcery, but at the time of writing this (April 2018) that's not the case anymore. Fortunately it's fairly easy to build such a toolchain using [crosstool-ng](https://crosstool-ng.github.io).

### Building the toolchain

To build the toolchain:

1. Follow the [crosstool-ng installation instructions](https://crosstool-ng.github.io/docs/install/) to install crosstool-ng. [Version 1.23.0](http://crosstool-ng.org/download/crosstool-ng/crosstool-ng-1.23.0.tar.xz) of crosstool-ng was used for generating the toolchain used in this document, but newer version should work too.
2. Add the directory with the 'ct-ng' executable to your PATH.
3. Create a new directory and copy the crossstool-ng configuation file from [here](.config) to that directory.
4. Execute `ct-ng menuconfig` in that directory. At this point you can examine and even (carefully!) modify the crosstool-ng configuration. When you're done, press "Exit".
5. Execute `ct-ng build` to build the toolchain. This step will take a while, as it will build the whole toolchain and install it. By default, it will install it to `$HOME/x-tools/i386-unknown-elf` (the installation path can be changed by modifying the configuration in step 4 above).
6. Add the toolchain's `bin` directory to your PATH:

```
$ export PATH=$PATH:$HOME/x-tools/i386-unknown-elf/bin
```

Test the installation by running `i686-elf-gcc --version`, which should display the toolchain's GCC version.

### Building the simulator

Install `nasm` if it's not installed:

```
$ sudo apt install nasm
```

Then invoke `build_elua` to build the simulator:

```
$ lua build_elua.lua board=sim
```

To specify the maximum RAM size available to the simulator (in bytes), build using `-DSIM_MEM_SIZE`:

```
$ lua build_elua.lua board=sim -DSIM_MEM_SIZE=64*1024 # emulated RAM size is 64KB
```

The default value of the `SIM_MEM_SIZE` macro can be found [here](../../src/platform/sim/cpu_linux.h).

The builder will create `elua_lua_sim.elf` which will be used to run the simulator (see next paragraph).

It's also possible to build different flavours of eLua. For example, to build a version with the number type set to "long long" instead of the default "double", run the builder like this:

```
$ lua build_elua.lua board=sim target=lualonglong # use the default value of SIM_MEM_SIZE
```

In this case, the builder will create `elua_lualonglong_sim.elf`.

## How to use

Run the simulator using the `run_elua_sim.sh` script, which sets the console in a mode compatible with eLua's terminal implementation, then runs the simulator:

```
$ ./run_elua_sim.sh
```

You can run other flavours of the simulator using the same script:

```
$ ./run_elua_sim.sh longlong # runs 'elua_lualonglong_sim.elf'
```

After the simulator starts, you'll be presented with the regular eLua shell prompt, and you can run `lua` from there. Just remember that you're not running on the regular eLua hardware, so anything hardware-related (like using GPIO pins, serial interfaces (such as UARTs) or configuring interrupt handlers) will not work.

To exit the Lua interpreter, press CTRL+Z. To exit the simulator, type `exit` at the eLua shell prompt.

### WOFS emulator

If WOFS is enabled, it is emulated into a file called `wofs.img` located in the same directory as the one used to start the simulator. If the file doesn't exist, it is created automatically.

### SD card emulator

If the FAT filesystem is enabled, the simulator looks for a file called `sdcard.img` in the same directory as the one used to start the simulator. If the file is found, it is considered a valid SD card image and it used to emulate an actual SD card. The easiest way to obtain such an image is to use an SD card that is formatted with a FAT file system. To do that from Linux, insert the card in your system, find out which block device it uses (by using `dmesg` for example) and use `dd` to obtain a complete image of the card:

```
# The next line assumes that the SD card's block device is /dev/sdd, which might not be true on your system
$ dd if=/dev/sdd of=sdcard.img bs=4M
```
