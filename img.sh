#! /bin/bash

OF=edOS.img
SECTORS=1048576
SECTORS_HEX='\xFF\xFF\x0F\x00'
SECTOR_SIZE=512

#Compile MBR bootstrap
make -C mbr

#Compile bootloader
make -C bootloader

#Compile stage2
make -C bootloader/stage2

if test -e $OF; then
    rm $OF
fi
touch $OF

dd if=/dev/zero bs=${SECTOR_SIZE} count=1 seek=$(($SECTORS-1)) of=$OF

#-----Create MBR-----
BOOTSTRAP_SPACE=446
sfdisk $OF < mbr.script
dd if=mbr/build/bootstrap.bin bs=1 count=$BOOTSTRAP_SPACE conv=notrunc of=$OF

#----Create file system---------------
mkfs.fat --mbr=no --offset 2048 -M 0xF0 -F 32 -D 0x80 -b 6 $OF
START=$(($SECTOR_SIZE*2048))

BOOTLOADER=bootloader/build/bootloader.bin 

dd if=${BOOTLOADER} conv=notrunc bs=1 count=422 seek=$(($START+90)) of=$OF

#Copy to backup sector
dd if=${BOOTLOADER} conv=notrunc bs=1 count=422 seek=$(($START+90+6*$SECTOR_SIZE)) of=$OF

#Mount image file and add stage2 to the file system
loop=$(sudo losetup -f)
echo Using loop device: $loop

sudo losetup -P  $loop $OF
sudo mount -o sync $loop"p1" img

sudo cp bootloader/stage2/build/stage2.bin img
sudo cp kernel/build/stage2.bin img/kernel.bin


sudo umount img
sudo losetup -d $loop

