#!/bin/bash

function compute_average {
	NB_ITER=`wc -l $1 | cut -f 1 -d " "`

	for (( line_id = 0; line_id < $NB_ITER; line_id++ ))
	do
		cat $* | grep "^${line_id}	" | awk '{id = $1; train_SS3+=$2; test_SS3+=$3; train_SS8+=$4; test_SS8+=$5; train_SA+=$6; test_SA+=$7; time+=$8; pass = $9; nb_file++} END {print id" "train_SS3/nb_file" "test_SS3/nb_file" "train_SS8/nb_file" "test_SS8/nb_file" "train_SA/nb_file" "test_SA/nb_file" "time/nb_file" "pass}'
	done
}

for (( i=0 ; i < ${2}; i++ ))
do
	compute_average ${1}*.iter_${i} > avg.${1}${i}
done

compute_average ${1}*.pass > pass.${1}
