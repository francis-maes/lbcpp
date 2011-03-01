#!/bin/bash

DATABASE=$1

function buildSplit
{
  echo -n "Building split '$1' ... "

  rm -rf ${DATABASE}/$1 2>&1 > /dev/null
  mkdir ${DATABASE}/$1

  for f in `cat ${DATABASE}/.$1`
  do
    cp ${DATABASE}/all/$f ${DATABASE}/$1
  done

  echo "Done"
}

if [ "x$1" == "x" ]
then
  echo "Usage: $0 DATABASE"
  exit
fi

if [ ! -d ${DATABASE}/all ]
then
  echo "Error - Directory not found: ${DATABASE}/all"
  exit
fi

if [[ -f ${DATABASE}/.test && -f ${DATABASE}/.train && -f ${DATABASE}/.validation ]]
then
  echo "Warning - Split already exists."
  buildSplit test
  buildSplit validation
  buildSplit train
  exit
fi

rm .tmp_create_split
for f in `ls ${DATABASE}/all/`
do
  echo ${RANDOM} $f >> .tmp_create_split
done

sort .tmp_create_split | sed -r 's/^[0-9]+ //' > .tmp_create_split_sorted

numFiles=`cat .tmp_create_split_sorted | wc -l`
numTest=`expr $numFiles / 4`
numValidation=$numTest

head -$numTest .tmp_create_split_sorted > ${DATABASE}/.test
numTest=`expr $numTest + 1`
tail -n +$numTest .tmp_create_split_sorted > .tmp_create_split
head -$numValidation .tmp_create_split > ${DATABASE}/.validation
tail -n +$numTest .tmp_create_split > ${DATABASE}/.train

rm .tmp_create_split
rm .tmp_create_split_sorted

buildSplit test
buildSplit validation
buildSplit train
