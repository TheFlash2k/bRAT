#!/bin/bash

if [ ! -d "./bins" ]; then
	mkdir bins
fi


echo Compiling Victim.cpp
g++ ./src/victim.cpp -o ./bins/victim64 -w
echo Compilation Complete
echo Compiling Attacker.cpp
g++ ./src/attacker.cpp -o ./bins/attacker64 -w
echo Compilation Complete
