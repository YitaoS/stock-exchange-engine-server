#!/bin/bash

make clean

make

./engine 0.0.0.0 12345 1000

while true
do 
	sleep 1 
done