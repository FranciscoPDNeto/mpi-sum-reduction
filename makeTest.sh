#!/bin/bash

rm -rf test.txt
echo "time" >> test.txt
echo $1 >> test.txt
for i in $(seq $1); do echo $RANDOM >> test.txt; done;
