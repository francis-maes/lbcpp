#!/bin/bash

processName="RunWorkUnit"

if [ "x$1" != "x" ]
then
  processName="$1"
fi

outputFile=""

echo -n "Looking for PID of ${processName} ... "
processId="";
for (( i=0 ; ; i++ ))
do
  sleep 1

  if [ "x$processId" == "x" ]
  then
    processId=`ps -a | tail -n +2 | grep "$processName" | grep -v "grep"`
    processId=` echo $processId | cut -f 1 -d ' '`
    if [ "x$processId" != "x" ]
    then
      i=0
      echo $processId
      startTime=`date "+%Y-%m-%d_%H-%M-%S"`
      outputFile="${processName}_${startTime}"
      echo -n > $outputFile
    else
      continue
    fi
  fi
  
  memory=`ps -o "rss" -p ${processId} | tail -n +2`
  if [ "x$memory" == "x" ]
  then
    memory="0"
  fi

  currentTime=`date "+%Y-%m-%d %H:%M:%S"`
  echo $i $currentTime $memory >> $outputFile

  if [ $memory == "0" ]
  then
    echo "Bye."
    break
  fi
done

