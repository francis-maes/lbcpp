#!/bin/sh
cd build
mkdir linux-x86-eclipse
cd linux-x86-eclipse
ccmake -G"Eclipse CDT4 - Unix Makefiles" ../..
