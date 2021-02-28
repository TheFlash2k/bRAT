#!/bin/bash

if [ ! -d "./bins" ]; then
	mkdir bins
fi

echo Compiling files...

g++ ./src/victim.cpp -m32 -o ./bins/victim32 -w 2>./compile.log
g++ ./src/attacker.cpp -m32 -o ./bins/attacker32 -w &>./compile.log

if grep -Fxq "compilation terminated." compile.log
then
    echo;echo "[!] An occurred during compilation. This must be due to the fact that g++ library is missing. To install the library:"
    echo "sudo apt-get install g++-multilib";echo
    exit
fi

echo Compilation Complete