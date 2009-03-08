#!/bin/sh
cd build
mkdir linux-x86-kdevelop
cd linux-x86-kdevelop
ccmake -G"KDevelop3" ../..
kdevelop CRAlgo.kdevelop &
cd ../..
