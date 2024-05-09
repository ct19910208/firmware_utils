#!/bin/bash
## Top level directory.
export TOP_DIR=`pwd`/../
export TOOLS_DIR=$TOP_DIR/tools
export UTIL_LIN_DIR=$TOOLS_DIR/lin/utils

## for JTAG interface
## for openocd
export OPENOCD_HOME=$TOOLS_DIR/lin/jtag/openocd-0.10.0-0-20220418-1947/
export PATH=$OPENOCD_HOME/bin:$PATH
export OPENOCD_SCRIPTS=$OPENOCD_HOME/scripts

## for at1k gnu tools
export PATH=$TOOLS_DIR/lin/gcc/7.1.1-20181125-20201113-1236/bin/:$PATH
## for at5k gnu tools
export PATH=$TOOLS_DIR/lin/gcc/gcc-arm-10.3-2021.07-x86_64-arm-none-eabi/bin/:$PATH
## for at5k gnu tools old
export PATH=$TOOLS_DIR/lin/gcc/gcc-arm-8.3-2019.03-x86_64-arm-eabi/bin/:$PATH

## for misc
export PATH=$UTIL_LIN_DIR/imgbuilder:$PATH
export PATH=$UTIL_LIN_DIR/uncrustify:$PATH

export AT_SDK_HOME_SH=`pwd`
export PATH=$AT_SDK_HOME_SH/tools/scripts:$PATH
export PATH=$AT_SDK_HOME_SH/tools/gen_gdbscript:$PATH

# export CHIP=AT1K
export RELEASE=no

# ## for makefile
# export MAKEFILES=$AT_SDK_HOME_SH/tools/mk/function.mk
