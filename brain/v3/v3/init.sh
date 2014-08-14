#!/bin/sh

already_loaded=$(cat /sys/devices/bone_capemgr.8/slots|grep BB-UART1|wc -l)
if [ $already_loaded == "0" ]; then 
	echo BB-UART1 > /sys/devices/bone_capemgr.8/slots
fi
