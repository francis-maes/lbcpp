#!/bin/sh
cd ..
mkdir -p build/linux-x86-eclipse
cd build/linux-x86-eclipse
ccmake -G"Eclipse CDT4 - Unix Makefiles" ../../trunk
