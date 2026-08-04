#!/bin/bash
#
cd /home/pdsilva/Projects/Orion68/src/pico2VGA
rm -rf build
mkdir build
cd build
cmake -DPICO_BOARD=pico2 ..
make
cd ..
cd build
