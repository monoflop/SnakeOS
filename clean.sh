#!/bin/bash
clear
rm bin/*
cd src
find . -type f -name '*.o' -delete
exit 0
