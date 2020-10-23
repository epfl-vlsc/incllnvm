#!/usr/bin/env bash
touch *
./bootstrap.sh
./configure --with-malloc=jemalloc
 #--disable-assertions
make all -j