#!/bin/bash

workUnits=`ls ${workUnitDirectory}/PreProcessing`

for f in $workUnits
do
  fileName=`echo $f | cut -f 1 -d '.'`

  preProcessingFile="${workUnitDirectory}/PreProcessing/$f"
  workUnitFile="${workUnitDirectory}/WorkUnits/$f"
  finishedFile="${workUnitDirectory}/Finished/$fileName"
  traceFile="${workUnitDirectory}/Traces/$fileName.trace"
  jobFile="${workUnitDirectory}/Jobs/$fileName.qsub"
  requestFile="${workUnitDirectory}/Requests/$fileName.request"
  
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

  mv $preProcessingFile $workUnitFile

  cat > $jobFile << EOF
#$ -l h_vmem=${requiredMemory}G
#$ -l h_rt=${requiredTime}:10:00
#$ -cwd
#$ -o $rootProjectDirectory/$projectName/$fileName.o
#$ -e $rootProjectDirectory/$projectName/$fileName.e
#$ -j y
#$ -N ${projectName}_$fileName

cd $programDirectory
./RunWorkUnit $workUnitFile --trace $traceFile --projectDirectory $rootProjectDirectory/$projectName
touch $finishedFile
EOF

done

