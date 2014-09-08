#! /bin/bash

make precv psend
taskset -c 1 ./precv 5000000 &
taskset -c 2 ./psend 5000000
