#!/bin/bash

if [ ! -f "bin/kernel.bin" ]; then
  echo "Please run ./build.sh first"
  exit 0
fi

cd bin
#-append cmdline use 'cmdline' as kernel command line
#qemu debug log -d int,cpu_reset
qemu-system-i386 -kernel kernel.bin -serial file:serial.log -d cpu_reset
exit 0
