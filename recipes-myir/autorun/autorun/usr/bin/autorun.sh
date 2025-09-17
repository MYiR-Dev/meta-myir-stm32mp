#!/bin/sh
source /etc/profile.d/weston_profile.sh

echo "[Application] Starting mxapp2..."
/usr/sbin/mxapp2 &
echo "[Application] Program launched"

