#!/bin/bash

workUnitDirectory="/home/boincadm/projects/evo/Network/.WorkUnit"
programDirectory="/home/boincadm/LBCPP/bin/Release"
rootProjectDirectory="/home/boincadm/projects/evo/Network/Projects"

serverName="boincadm@boinc.run"
masterHostname="localhost"

# Take the lock
if [ -f $workUnitDirectory/.lock ]
then
  date=`date`
  echo "$date - Warning - Lock is still busy" >> log
  exit
fi

touch $workUnitDirectory/.lock

# Communication with Monster24
cd $programDirectory
./RunWorkUnit GridWorkUnit --hostName $masterHostname --port 1665 --gridName $serverName --gridEngine BOINC --projectDirectory $workUnitDirectory

rm $workUnitDirectory/.lock


