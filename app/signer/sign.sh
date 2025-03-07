#!/bin/bash

rm -rf build
mkdir -p build

gcc main.c Crc_32.c -o ./build/bl_calc

cd ./build

./bl_calc