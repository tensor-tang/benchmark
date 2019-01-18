#!/bin/bash

MKLML_ROOT=./mklml

/usr/bin/g++-4.8 test.cc -I${MKLML_ROOT}/include -lmklml_intel -L${MKLML_ROOT}/lib -liomp5 -Wl,--as-needed -lmklml_intel -Wl,--rpath=${MKLML_ROOT}/lib -ldl -lpthread -std=c++11 -mavx -o testso
