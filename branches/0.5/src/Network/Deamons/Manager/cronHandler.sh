#!/bin/bash

managerDirectory="/home/jbecker/Manager/"
programDirectory="/home/jbecker/Workspace/LBC++/bin/Debug/"

cd $managerDirectory

if [ -f .lock ]
then
  exit
fi

touch .lock

startDate=`date`
echo "$startDate - Start Manager" >> .cronHandler

cd $programDirectory
./RunWorkUnit ServerWorkUnit --projectDirectory $managerDirectory 2>&1 >> $managerDirectory/.cronHandler

cd $managerDirectory
endDate=`date`
echo "$endDate - Stop Manager" >> .cronHandler

rm .lock

