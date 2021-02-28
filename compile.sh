#!/bin/bash

echo Compiling Victim.cpp
g++ victim.cpp -o victim -w
echo Compilation Complete
sleep 1
echo Compiling Attacker.cpp
g++ attacker.cpp -o attacker -w
echo Compilation Complete
