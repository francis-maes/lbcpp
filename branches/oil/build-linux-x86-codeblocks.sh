#!/bin/sh
cd build
mkdir linux-x86-codeblocks
cd linux-x86-codeblocks
ccmake -G"CodeBlocks - Unix Makefiles" ../..
codeblocks LBCpp.cbp > /dev/null 2>&1 &
cd ../..
