#!/bin/bash
clear

if [ -d "bin" ]; then
  rm bin/*
fi

cd src
find . -type f -name '*.o' -delete
exit 0
