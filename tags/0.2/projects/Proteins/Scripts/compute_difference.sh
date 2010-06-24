#!/bin/sh

for line in `cat $1 | cut -f 1 -d ' '`
do
	val_1=( `cat $1 | grep "^$line "` )
	val_2=( `cat $2 | grep "^$line "` )
	
	train=`echo "${val_1[1]} - ${val_2[1]}" | bc`
	test=`echo "${val_1[2]} - ${val_2[2]}" |bc`
	time=`echo "${val_1[3]} - ${val_2[3]}" | bc`
	echo $line $train $test $time
done
