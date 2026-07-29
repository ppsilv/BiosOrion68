#!/bin/bash
#
cd /home/pdsilva/project/Orion68/src/pico2W
rm -rf build
mkdir build
cd build
cmake -DPICO_BOARD=pico2_w ..
make
cd ..
cd build
