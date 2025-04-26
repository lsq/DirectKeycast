#!/usr/bin/bash

cmake -DCMAKE_CXX_COMPILER=g++ -DCMAKE_CC_COMPILER=gcc -DCMAKE_MAKE_PROGRAM=mingw32-make -G "MinGW Makefiles"  -S . -B ./build/
cmake --build ./build/ 
[ -f ./build/bin/directkeycast.exe ] && cp ./build/bin/directkeycast.exe ./directkeycast.exe
