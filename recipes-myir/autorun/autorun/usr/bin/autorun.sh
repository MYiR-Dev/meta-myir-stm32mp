#!/bin/sh
source /etc/profile.d/weston_profile.sh
source /etc/profile.d/pulse_profile.sh

CONFIG_FILE="/etc/systemd/timesyncd.conf"
TARGET_LINE="FallbackNTP=ntp.ntsc.ac.cn cn.ntp.org.cn time1.google.com time2.google.com time3.google.com time4.google.com"

# Check if target configuration already exists
if grep -q "^${TARGET_LINE}" "${CONFIG_FILE}"; then
    echo "Configuration already exists, no changes needed."
    exit 0
fi

# Create backup of original file
cp "${CONFIG_FILE}" "${CONFIG_FILE}.bak"

# Process commented FallbackNTP line
sed -i -E \
    -e '/^#FallbackNTP=/ {
        s/^#//;
        s/(=).*/\1ntp.ntsc.ac.cn cn.ntp.org.cn time1.google.com time2.google.com time3.google.com time4.google.com/;
    }' "${CONFIG_FILE}"

# Verify modification
if grep -q "^${TARGET_LINE}" "${CONFIG_FILE}"; then
    echo "Successfully updated existing configuration"
else
    # Add new configuration to [Time] section
    if grep -q '^$$Time$$' "${CONFIG_FILE}"; then
        sed -i '/^$$Time$$/a '"${TARGET_LINE}" "${CONFIG_FILE}"
        echo "Added new configuration to [Time] section"
    else
        echo "[Time] section not found in config file!"
        exit 1
    fi
fi

# Restart time synchronization service
systemctl restart systemd-timesyncd.service
echo "Time synchronization service restarted"

RULES_FILE="/etc/udev/rules.d/touchscreen.rules"
RULE_LINE='SUBSYSTEM=="input", KERNEL=="event[0-9]*", ENV{ID_INPUT_TOUCHSCREEN}=="1", ENV{LIBINPUT_CALIBRATION_MATRIX}=" 61.509373 0.480948 0.019359 2.795640 116.671989 -0.044791"'

if [ ! -f "$RULES_FILE" ]; then
    touch "$RULES_FILE"
fi

if ! grep -qF -- "$RULE_LINE" "$RULES_FILE"; then
    echo "$RULE_LINE" >> "$RULES_FILE"
else
	echo "OK"
fi

sync
udevadm control --reload
udevadm trigger --action=change --subsystem-match=input
/usr/sbin/mxapp2 &
