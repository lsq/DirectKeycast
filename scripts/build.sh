#!/usr/bin/bash

script_path=$(dirname "$(readlink -f "$0")")
cp "$script_path"/config_files/CMakeLists.txt "$script_path"/../
cmake -DCMAKE_CXX_COMPILER=g++ -DCMAKE_CC_COMPILER=gcc -DCMAKE_MAKE_PROGRAM=mingw32-make -G "MinGW Makefiles"  -S . -B ./build/
cmake --build ./build/ 
# commitId=$(git log -1 --format=%h)
commitId=$(printf "%s_%s" "$(git rev-list --count HEAD)" "$(git rev-parse --short HEAD)") 
[ -f ./build/bin/directkeycast.exe ] && cp ./build/bin/directkeycast.exe ./directkeycast-r"${commitId}".exe
