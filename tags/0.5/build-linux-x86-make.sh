#!/bin/sh
cd build
mkdir linux-x86-make
cd linux-x86-make
mkdir debug
cd debug
ccmake -DCMAKE_BUILD_TYPE:String=Debug ../../..
make
cd ..
mkdir release
cd release
ccmake -DCMAKE_BUILD_TYPE:String=Release ../../..
make
cd ../../..
