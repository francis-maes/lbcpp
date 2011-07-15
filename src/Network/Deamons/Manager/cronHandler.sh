#!/bin/bash

managerDirectory="/home/jbecker/Manager/"
programDirectory="/home/jbecker/Workspace/LBC++/bin/Release/"

cd $managerDirectory

if [ -f .lock ]
then
  exit
fi

touch .lock

startDate=`date`
echo "$startDate - Start Manager" >> .cronHandler

cd $programDirectory
./RunWorkUnit ManagerWorkUnit --projectDirectory $managerDirectory --numThreads 1 2>&1 >> $managerDirectory/.cronHandler

cd $managerDirectory
endDate=`date`
echo "$endDate - Stop Manager" >> .cronHandler

rm .lock

