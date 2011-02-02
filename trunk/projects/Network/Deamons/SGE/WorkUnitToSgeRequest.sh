#!/bin/bash

workUnits=`ls ${workUnitDirectory}/Waiting`

for f in $workUnits
do
  fileName=`echo $f | cut -f 1 -d '.'`

  waitingFile="${workUnitDirectory}/Waiting/$f"
  inProgressFile="${workUnitDirectory}/InProgress/$f"
  finishedFile="${workUnitDirectory}/Finished/$f"
  requestFile="${workUnitDirectory}/Requests/$fileName.request"
  
  trace="${workUnitDirectory}/Traces/$fileName.trace"
  projectName=`cat $requestFile | grep "projectName" | cut -f 2 -d '>' | cut -f 1 -d '<'`
  requiredCpus=`cat $requestFile | grep "requiredCpus" | cut -f 2 -d '>' | cut -f 1 -d '<'`
  requiredMemory=`cat $requestFile | grep "requiredMemory" | cut -f 2 -d '>' | cut -f 1 -d '<'`
  requiredTime=`cat $requestFile | grep "requiredTime" | cut -f 2 -d '>' | cut -f 1 -d '<'`
  
  if [ ! -d $rootProjectDirectory/$projectName ]
  then
    mkdir $rootProjectDirectory/$projectName
    mkdir $rootProjectDirectory/$projectName/Output

    ln -s /scratch/jbecker/data/ $rootProjectDirectory/$projectName/data
  fi
  
  qsub << EOF
#$ -l h_vmem=${requiredMemory}G
#$ -l h_rt=${requiredTime}:10:00
#$ -cwd
#$ -o $rootProjectDirectory/$projectName/$fileName.o
#$ -e $rootProjectDirectory/$projectName/$fileName.e
#$ -j y
#$ -N ${projectName}_$fileName
mv $waitingFile $inProgressFile
cd $programDirectory
./RunWorkUnit $inProgressFile --trace $trace --projectDirectory $rootProjectDirectory/$projectName
mv $inProgressFile $finishedFile
EOF

done

