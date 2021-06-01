#!/bin/bash

make

./ucl -o test.s test.c

/usr/bin/gcc -m32 -o test test.s -lc -lm

./test

make clean
