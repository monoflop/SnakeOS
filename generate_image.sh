#!/bin/bash

if [ ! -f "bin/kernel.bin" ]; then
  echo "Please run ./build.sh first"
  exit 0
fi

sudo mkdir -p /media/floppy
sudo mount -o loop resources/tools/floppy.img /media/floppy
sudo cp bin/kernel.bin /media/floppy/boot/kernel.bin
sudo umount /media/floppy
cp resources/tools/floppy.img bin/floppy.img
