#########################################
# Script to extract network consumption #
# Author : Arnaud Schoofs               #
#########################################

#!/bin/bash

RX=`vnstat -s | grep "today" | awk '{print $2}'`
TX=`vnstat -s | grep "today" | awk '{print $5}'`
ALL=`vnstat -s | grep "today" | awk '{print $8}'`

CIVDATE=`date "+%Y:%m:%d:%H:%M"`
UNIXDATE=`perl -e 'print time()'`

echo $CIVDATE $UNIXDATE $RX $TX $ALL
