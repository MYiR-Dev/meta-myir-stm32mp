SUMMARY = "MYiR HMI Qt6 demo application"
DESCRIPTION = "MXAPP2 Qt6/QML HMI demo for MYD-LD25X"
LICENSE = "CLOSED"

SRC_URI = "git://github.com/MYiR-Dev/mxapp.git;protocol=https;branch=MXAPP2-QT6-IMX9X \
           file://0001-mediapipeline-use-Linux-6.6-subdev-routing-API.patch \
           file://0002-main-show-maximized-on-Wayland.patch \
           file://0003-camera-fix-stm32mp25-ov5640-preview.patch \
           file://0004-camera-preserve-dcmipp-main-input-size.patch \
           file://0005-camera-prefer-real-sensor-over-dcmipp-tpg.patch \
           file://0006-camera-align-mmap-queue-with-v4l2src.patch \
           file://0007-main-handle-display-hotplug.patch \
"
SRCREV = "3da3aa1f8b108019ccacdcaecaae403de18173a5"

S = "${WORKDIR}/git"

DEPENDS = "\
    qtbase \
    qtdeclarative \
    qtdeclarative-native \
    qtmultimedia \
    qttools-native \
    qtvirtualkeyboard \
"

inherit qt6-cmake

EXTRA_OECMAKE = "-DCMAKE_BUILD_TYPE=Release -DENABLE_PLAYER=ON"

do_install() {
    install -d ${D}${sbindir}
    install -m 0755 ${B}/mxapp2 ${D}${sbindir}/mxapp2
}

FILES:${PN} = "${sbindir}/mxapp2"

COMPATIBLE_MACHINE = "stm32mp25common"
