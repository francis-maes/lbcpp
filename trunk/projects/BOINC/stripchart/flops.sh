#!/bin/bash

CIVDATE=`date "+%Y:%m:%d:%H:%M"`
UNIXDATE=`perl -e 'print time()'`

flops=`mysql -u boincadm -h localhost -p002230 alpha0 -Bse"SELECT SUM(cpu_time*flops_estimate)/3600/1000000000 FROM (SELECT cpu_time,flops_estimate FROM result WHERE server_state = 5 && outcome = 1 && client_state = 5 && validate_state = 1 && received_time >= ($UNIXDATE-3600) && received_time <= $UNIXDATE) a"`
flops=`echo $flops| tr -d 'ULL'| tr 'N' '0'`
echo $CIVDATE $UNIXDATE $flops
