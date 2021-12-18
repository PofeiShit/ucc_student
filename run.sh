#!/bin/bash

sh build.sh

./output/ucl -o test.s test.c

/usr/bin/gcc -o test test.s -lc -lm

./test

