#!/bin/bash
# Shell script to backup MySql database
# To backup Nysql databases file to /backup dir and later pick up by your
# script. You can skip few databases from backup too.
# For more info please see (Installation info):
# http://www.cyberciti.biz/nixcraft/vivek/blogger/2005/01/mysql-backup-script.html
# Last updated: Aug - 2005
# --------------------------------------------------------------------
# This is a free shell script under GNU GPL version 2.0 or above
# Copyright (C) 2004, 2005 nixCraft project
# Feedback/comment/suggestions : http://cyberciti.biz/fb/
# -------------------------------------------------------------------------
# This script is part of nixCraft shell script collection (NSSC)
# Visit http://bash.cyberciti.biz/ for more information.
# -------------------------------------------------------------------------

# EDITED by Arnaud Schoofs

NOW="$(date +"%d-%m-%Y,%Hh%M")"
echo #n newline
echo "Starting backup at $NOW ..."

# Paths/directory variables
ExecDir="/home/boincadm/LBCPP/projects/BOINC/tools"
WorkingDir="/home/boincadm"
DIR="db_backup"
HOST="$(hostname)"

# BOINC variable
Project="evo"

# DB variables
MyUSER=`cat $WorkingDir/dbinfo | awk '{print $1}'`	# USERNAME
MyPASS=`cat $WorkingDir/dbinfo | awk '{print $2}'`   # PASSWORD
MyHOST=`cat $WorkingDir/dbinfo | awk '{print $3}'`	# Hostname

# SCP variables
DestSCP=`cat $WorkingDir/scpinfo | awk '{print $1}'`		# login@host.domain:path/
PasswordSCP=`cat $WorkingDir/scpinfo | awk '{print $2}'`	# password

# Linux bin paths, change this if it can not be autodetected via which command
MYSQL="$(which mysql)"
MYSQLDUMP="$(which mysqldump)"
GZIP="$(which gzip)"
 
# Variables declaration
FILE=""
DBS=""
 
# DO NOT BACKUP these databases
IGGY="information_schema mysql phpmyadmin"
 
# Go to working di
cd $WorkingDir

# create DIR if necessary 
[ ! -d $DIR ] && mkdir -p $DIR || :

# stops boinc server
./projects/$Project/bin/stop
 
# Get database list
DBS="$($MYSQL -u $MyUSER -h $MyHOST -p$MyPASS -Bse 'show databases')"

# Backup databases except the exclude ones 
for db in $DBS
do
    skipdb=-1
    if [ "$IGGY" != "" ];
    then
	for i in $IGGY
	do
	    [ "$db" == "$i" ] && skipdb=1 || :
	done
    fi
 
    if [ "$skipdb" == "-1" ] ; then
	FILE="./$DIR/$db.$HOST.dump.gz"
	$MYSQLDUMP -u $MyUSER -h $MyHOST -p$MyPASS $db | $GZIP -9 > $FILE
    fi
done

# Log rotate
/usr/sbin/logrotate -s ./projects/$Project/pid_boinc/logrotate.state ./projects/$Project/logrotate.conf &> ./projects/$Project/logrotate.log

# Backup
$ExecDir/scplogin.exp $PasswordSCP $DestSCP $WorkingDir/$DIR
$ExecDir/rsynclogin.exp $PasswordSCP $DestSCP ./projects/$Project $ExecDir/excludePATH		# files listed in excludePATH are ignored

# Restarts boinc server
./projects/$Project/bin/start

echo "Backup finished"
echo # newline

 

