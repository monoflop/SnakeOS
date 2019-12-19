#!/bin/bash
clear
cd src
make

mkdir -p ../bin

mv kernel ../bin/kernel.bin

exit 0
