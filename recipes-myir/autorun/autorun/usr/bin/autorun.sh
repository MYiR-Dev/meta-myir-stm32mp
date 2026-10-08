#!/bin/sh

WESTON_RUNTIME_DIR="/run/user/$(id -u weston)"

export XDG_RUNTIME_DIR="${WESTON_RUNTIME_DIR}"
export WAYLAND_DISPLAY=wayland-0
if [ -S "${WESTON_RUNTIME_DIR}/wayland-1" ]; then
	export WAYLAND_DISPLAY=wayland-1
fi

export ELM_ENGINE=wayland_shm
export ECORE_EVAS_ENGINE=wayland_shm
export GDK_BACKEND=wayland
export LANG=en_US.UTF-8
export QT_QPA_PLATFORM=wayland
export USE_PLAYBIN3=1

export DBUS_SESSION_BUS_ADDRESS="unix:path=${WESTON_RUNTIME_DIR}/bus"
export PULSE_RUNTIME_PATH="${WESTON_RUNTIME_DIR}"
export PULSE_SERVER="unix:${WESTON_RUNTIME_DIR}/pulse/native"

if ! timeout 30 sh -c 'until /usr/bin/wayland-info >/dev/null 2>&1; do sleep 0.1; done'; then
	echo "Wayland compositor did not become ready" >&2
	exit 1
fi

exec /usr/sbin/mxapp2
