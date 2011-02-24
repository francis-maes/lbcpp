#!/bin/bash

workUnitDirectory="/home/boincadm/projects/evo/NetworkTest/.WorkUnit"
programDirectory="/home/boincadm/LBCPP/bin/Release"
rootProjectDirectory="/home/boincadm/projects/evo/NetworkTest/Projects"

serverName="boincadm@boinc.run"
masterHostname="monster24.montefiore.ulg.ac.be"

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
./RunWorkUnit GridWorkUnit --hostName $masterHostname --gridName $serverName --gridEngine SGE --projectDirectory $workUnitDirectory

rm $workUnitDirectory/.lock


