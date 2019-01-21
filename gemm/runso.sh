#!/bin/bash

export MKL_NUM_THREADS=1
export OMP_NUM_THREADS=1

#export KMP_DETERMINISTIC_REDUCTION=yes
#export MKL_CBWR=AVX

./testso $@ 
