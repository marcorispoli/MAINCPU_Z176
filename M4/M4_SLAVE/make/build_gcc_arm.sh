#!/usr/bin/env bash

status=0


make TOOL=gcc_arm CONFIG=release  LOAD=ram build  -j3
if [ "$?" != "0" ]
	/opt/arm-2017/bin/arm-none-eabi-objcopy -O binary ./gcc_arm/ram_release/m4_master.elf ./gcc_arm/ram_release/m4_master.bin
then
    status=-1
fi

if [ "${1}" != "nopause" ]; then
read -p "Press any key to continue... " -n1 -s
fi

exit $status

