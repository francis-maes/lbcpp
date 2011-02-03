#!/bin/sh
mkdir build
cd build
mkdir macosx-make
cd macosx-make
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
