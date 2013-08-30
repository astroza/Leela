#!/bin/sh

already_loaded=$(cat /sys/devices/bone_capemgr.8/slots |grep am33xx_pwm|wc -l)
if [ $already_loaded == "0" ]; then 
	echo am33xx_pwm > /sys/devices/bone_capemgr.8/slots
	echo bone_pwm_P8_13 > /sys/devices/bone_capemgr.8/slots
	echo bone_pwm_P8_34 > /sys/devices/bone_capemgr.8/slots
fi
rm -fr /tmp/v4
mkdir /tmp/v4
ln -s /sys/devices/ocp.2/pwm_test_P8_13.* /tmp/v4/right
ln -s /sys/devices/ocp.2/pwm_test_P8_34.* /tmp/v4/left
echo $1 > /tmp/v4/left/period
echo $1 > /tmp/v4/right/period
echo 0 > /tmp/v4/left/polarity
echo 0 > /tmp/v4/right/polarity
echo "" > /tmp/v4/watch
