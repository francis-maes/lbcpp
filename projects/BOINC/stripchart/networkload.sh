#!/bin/bash

RX1=`sudo ifconfig eth0 | grep "RX bytes" | tr ':' ' ' | awk '{ print $3 }'`
TX1=`sudo ifconfig eth0 | grep "TX bytes" | tr ':' ' ' | awk '{ print $8 }'`

sleep 30

RX2=`sudo ifconfig eth0 | grep "RX bytes" | tr ':' ' ' | awk '{ print $3 }'`
TX2=`sudo ifconfig eth0 | grep "TX bytes" | tr ':' ' ' | awk '{ print $8 }'`

RX=$((($RX2 - $RX1)*8/30/1000))
TX=$((($TX2 - $TX1)*8/30/1000))

CIVDATE=`date "+%Y:%m:%d:%H:%M"`
UNIXDATE=`perl -e 'print time()'`

echo $CIVDATE $UNIXDATE $RX $TX
