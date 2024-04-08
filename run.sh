#! /bin/bash

qemu-system-x86_64 -drive file=edOS.img,format=raw,index=0,media=disk -no-shutdown -no-reboot -d int,cpu_reset