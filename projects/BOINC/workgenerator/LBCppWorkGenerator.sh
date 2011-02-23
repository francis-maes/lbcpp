#!/bin/bash
workUnitDirectory="/home/boincadm/projects/evo/NetworkTest/.WorkUnit"
BoincProjectDir="/home/boincadm/projects/evo"

workUnits=`ls ${workUnitDirectory}/Waiting`

for f in $workUnits
do
  fileName=`echo $f | cut -f 1 -d '.'`

  waitingFile="${workUnitDirectory}/Waiting/$f"
  inProgressFile="${workUnitDirectory}/InProgress/$f"
  mv $waitingFile $inProgressFile

  cd $BoincProjectDir
  cp $inProgressFile `./bin/dir_hier_path $f`
  ./bin/create_work --appname LBCppBeta --wu_name $fileName --wu_template ./templates/LBCPP_wu --result_template ./templates/LBCPP_result $f
done


