#!/bin/bash

workUnitDirectory="/u/jbecker/.WorkUnit"
programDirectory="/u/jbecker/LBC++/bin/Release"
rootProjectDirectory="/u/jbecker/Projects"

serverName="jbecker@nic3"
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

# Launch the waiting workUnit
cd $workUnitDirectory
source ./WorkUnitToSgeRequest.sh

rm $workUnitDirectory/.lock

