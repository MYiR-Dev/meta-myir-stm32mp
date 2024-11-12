#!/bin/sh
source /etc/profile.d/weston_profile.sh
source /etc/profile.d/pulse_profile.sh
if [ ! -e "/usr/share/calibrator.txt" ];then
weston-touch-calibrator -v /sys/devices/platform/soc@0/42080000.rifsc/40140000.i2c/i2c-3/3-0038/input/input1/event1 > /usr/share/calibrator.txt
calibrationvalue=`cat /usr/share/calibrator.txt | awk -F ':' '{print $2}'`
echo 'SUBSYSTEM=="input", KERNEL=="event[0-9]*", ENV{ID_INPUT_TOUCHSCREEN}=="1", ENV{LIBINPUT_CALIBRATION_MATRIX}="'$calibrationvalue'"' >> /etc/udev/rules.d/touchscreen.rules
fi
/usr/sbin/mxapp2 &
