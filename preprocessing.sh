#!/bin/bash
PROJ_DIR=./preprocessing/
IMPLEMENTATION="pps serialrecombine mutexlock conditionalmutex altconditionalmutex tmerge"
MIN=2
MAX=2048
export LD_LIBRARY_PATH=$PROJ_DIR
for i in {1..100}
do	
	for IMPL in $IMPLEMENTATION
	do
		echo "Iteration $i $IMPL"
		"$PROJ_DIR"/"$IMPL" "$PROJ_DIR"/../data/1k1.hex "$PROJ_DIR"/../data/1k2.hex "$MIN" "$MAX";
	done;
done;
