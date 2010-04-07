#!/bin/bash


function compute_average {
	NB_ITER=`wc -l $1 | cut -f 1 -d " "`
	
	for (( line_id = 0; line_id < $NB_ITER; line_id++ ))
	do
		cat $* | grep "^${line_id} " | awk '{id = $1; train+=$2; test+=$3; time+= $4; nb_file++} END {print id" "train/nb_file" "test/nb_file" "time/nb_file}'
	done
}


files=`ls -1 $1* | grep -v ".[o|e]"`
compute_average $files > ${1}avg
