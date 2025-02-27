SUMMARY = "Provides Device Tree files for STM32MP boards"
LICENSE = "GPL-2.0-only"
LIC_FILES_CHKSUM = "file://${COMMON_LICENSE_DIR}/GPL-2.0-only;md5=801f80980d171dd6425610833a22dbe6"

EXTDTB_SRC_URI ?= "git://github.com/MYiR-Dev/myir-st-external-dt.git;protocol=https;branch=develop-ld25x-v6"
SRC_URI = "${EXTDTB_SRC_URI}"
SRCREV = "${AUTOREV}"

S = "${WORKDIR}/git"

EXT_DT_VERSION = "v6.0"
EXT_DT_RELEASE = "stm32mp-r1"

COMPATIBLE_MACHINE = "(stm32mpcommon)"

require external-dt-common.inc

# ---------------------------------
# Configure archiver use
# ---------------------------------
include ${@oe.utils.ifelse(d.getVar('ST_ARCHIVER_ENABLE') == '1', 'external-dt-archiver.inc','')}
