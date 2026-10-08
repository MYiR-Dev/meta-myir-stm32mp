LICENSE = "Apache-2.0 & MIT & BSD-3-Clause"
LIC_FILES_CHKSUM = "file://License.md;md5=f2f110de9c50e738bfd1e15ef52181c4"

SRC_URI = "git://github.com/MYiR-Dev/mxxprojects-stm32mp2.git;protocol=https;branch=develop_m33"
SRCREV  = "8f2aeac3c34315003297eb1edf94b5f063ba1200"

PV = "1.3.0"

S = "${WORKDIR}/git"

ARCHIVER_ST_BRANCH = "main"
ARCHIVER_ST_REVISION = "v${PV}"

#FIXME temporary
FILESEXTRAPATHS:prepend := "${THISDIR}/files:"

SRC_URI:append = " file://CMakeLists.txt "

do_configure:prepend() {
    install -m 0644 ${WORKDIR}/CMakeLists.txt ${S}
}
