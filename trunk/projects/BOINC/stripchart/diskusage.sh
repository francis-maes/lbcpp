#!/bin/bash

CIVDATE=`date "+%Y:%m:%d:%H:%M"`
UNIXDATE=`perl -e 'print time()'`
disk=`df -h | grep "/dev/sda1" | awk '{print $5}' | tr '%' ' '`

echo $CIVDATE $UNIXDATE $disk
