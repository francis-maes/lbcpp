#!/bin/bash

ProjectDIR="/home/boincadm/projects/evo"
cd $ProjectDIR

for (( c=1; c<=20; c++ ))
do
  uuid=$(python  -c 'import uuid; print uuid.uuid1()')
  ./bin/create_work -appname LBCppBeta -wu_name LBCPP_$uuid -wu_template ./templates/LBCPP_wu -result_template ./templates/LBCPP_result WorkUnitExample.xml
done
