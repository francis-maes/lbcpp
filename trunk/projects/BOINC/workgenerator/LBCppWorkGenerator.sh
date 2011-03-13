#!/bin/bash

# TODO may be not up to date

workUnitDirectory="/home/boincadm/projects/evo/Network/.WorkUnit"
BoincProjectDir="/home/boincadm/projects/evo"
rootProjectDirectory="/home/boincadm/projects/evo/Network/Projects"

workUnits=`ls ${workUnitDirectory}/Waiting`

for f in $workUnits
do
  fileName=`echo $f | cut -f 1 -d '.'`
  
  waitingFile="${workUnitDirectory}/Waiting/$f"
  inProgressFile="${workUnitDirectory}/InProgress/$f"
  finishedFile="${workUnitDirectory}/Finished/$f"
  traceFile="${workUnitDirectory}/Traces/$fileName.trace"
  requestFile="${workUnitDirectory}/Requests/$fileName.request"
  
  #projectName=`cat $requestFile | grep "projectName" | cut -f 2 -d '>' | cut -f 1 -d '<'`
  #requiredCpus=`cat $requestFile | grep "requiredCpus" | cut -f 2 -d '>' | cut -f 1 -d '<'`
  #requiredMemory=`cat $requestFile | grep "requiredMemory" | cut -f 2 -d '>' | cut -f 1 -d '<'`
  #requiredTime=`cat $requestFile | grep "requiredTime" | cut -f 2 -d '>' | cut -f 1 -d '<'`
  
  #if [ ! -d $rootProjectDirectory/$projectName ]
  #then
  #  mkdir $rootProjectDirectory/$projectName
  #  mkdir $rootProjectDirectory/$projectName/Output

  #  ln -s /scratch/jbecker/data/ $rootProjectDirectory/$projectName/data
  #fi

  mv $waitingFile $inProgressFile

  cd $BoincProjectDir
  cp $inProgressFile `./bin/dir_hier_path $f`
  ./bin/create_work --appname LBCppBeta --wu_name $fileName --wu_template ./templates/LBCPP_wu --result_template ./templates/LBCPP_result $f
done


