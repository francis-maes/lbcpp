##############################
# Script to extract CPU load #
# Author : Arnaud Schoofs    #
##############################

#! /bin/bash

UPTIME=`/usr/bin/uptime | awk '{print $10}' | sed s/,//`
CIVDATE=`/bin/date "+%Y:%m:%d:%H:%M"`
UNIXDATE=`/usr/bin/perl -e 'print time()'`
echo $CIVDATE $UNIXDATE $UPTIME
