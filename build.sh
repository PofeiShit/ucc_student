#!/bin/bash

set -e

if [ -d "./build" ]; then
    rm -r build
fi
if [ -d "./output" ]; then
    rm -r output
fi
mkdir -p output
mkdir -p build

pushd build
cmake .. -DBUILD_LLT=$1
make -j4
popd build

cp ./ucl/ucl ../output/