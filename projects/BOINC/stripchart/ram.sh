#! /bin/bash 

ram=`vmstat -S M | awk '{print $3}' | tr '\n' ' ' | awk '{print $3}'`
CIVDATE=`/bin/date "+%Y:%m:%d:%H:%M"`
UNIXDATE=`/usr/bin/perl -e 'print time()'`

echo $CIVDATE $UNIXDATE $ram
