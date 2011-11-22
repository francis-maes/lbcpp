#!/bin/bash

export LD_LIBRARY_PATH=/u/dcoligno/GCC/lib64:$LD_LIBRARY_PATH

workUnitDirectory="/scratch/jbecker/.WorkUnit"
programDirectory="/u/jbecker/LBC++/bin/Release"
rootProjectDirectory="/scratch/jbecker/Projects"

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
./RunWorkUnit GridWorkUnit --managerHostName $masterHostname -p 1664 -g $serverName -c sge --projectDirectory $workUnitDirectory

# Convert the waiting workUnits to SGE jobs
cd $workUnitDirectory
source ./WorkUnitToSgeRequest.sh

# Launch Jobs in an asyncronous way (because SGE don't work anytime and sometime induce very long latency)
./SubmitJobs.sh ${workUnitDirectory} &

rm $workUnitDirectory/.lock

