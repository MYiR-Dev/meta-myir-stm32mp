SUMMARY = "MYIR camera test application (V4L2 + G2D)"
DESCRIPTION = "Camera capture and display program with YUYV to RGB conversion and G2D acceleration"
LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://${COMMON_LICENSE_DIR}/MIT;md5=0835ade698e0bcf8506ecda2f7b4f302"

SRC_URI = "file://main.c \
           file://camera/camera_view.c \
           file://camera/camera_view.h \
           file://CMakeLists.txt \
           file://G2D/ion_mem_alloc.h \
           file://G2D/sunxi-g2d.h \
           file://G2D/tina_g2d_util.h \
           file://G2D/libs \
           file://G2D/src \
          "

S = "${WORKDIR}"

inherit cmake

EXTRA_OEMAKE += "-j24"

do_install() {
    install -d ${D}${bindir}
    install -m 0755 ${B}/myir_camera_test ${D}${bindir}/myir_camera_test
}

FILES:${PN} += "${bindir}/myir_camera_test"