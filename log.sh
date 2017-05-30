#!/bin/bash

filename=$1
events=$2
loops=$3

for i in $(seq 1 $loops)
do
	./bin/logData "data/${filename}_${i}" $events 1
done
