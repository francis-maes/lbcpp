#!/bin/bash

cd $1

if [ ! -d Jobs ]
then
  echo "Error - Jobs directory not found !"
  exit
fi

# Take the lock
if [ -f .jobLock ]
then
  echo "Error - JobLock is still busy !"
  exit
fi

touch .jobLock

for f in Jobs/*.qsub
do

  export SGE_ROOT="/cvos/shared/apps/sge/6.1/"
  out=`/cvos/shared/apps/sge/6.1/bin/lx26-amd64/qsub $f`
  
  isSent=`echo $out | grep "^Your job" | grep "has been submitted$" | wc -l`
  
  if [ "$isSent" == "1" ]
  then
    echo "Job '$f' submitted"
    rm $f
  fi
  
done

rm .jobLock

