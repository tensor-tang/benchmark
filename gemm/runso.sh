#!/bin/bash

export MKL_NUM_THREADS=1
export OMP_NUM_THREADS=1

./testso $1 $2 $3 $4 $5
