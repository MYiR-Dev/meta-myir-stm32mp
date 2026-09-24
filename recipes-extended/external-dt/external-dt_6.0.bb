UMMARY = "Provides Device Tree files for STM32MP boards"
LICENSE = "GPL-2.0-only"
LIC_FILES_CHKSUM = "file://${COMMON_LICENSE_DIR}/GPL-2.0-only;md5=801f80980d171dd6425610833a22dbe6"

SRC_URI = "git://github.com/myd-ld25x/myir-dt-stm32mp.git;protocol=https;branch=v6-stm32mp"
#SRC_URI = "git:///home/myir/bsp/myir-dt-stm32mp;protocol=file;branch=v6-stm32mp"
#SRCREV = "5ca6add7e4de2bcbbad0ff850af4d061c8f57686"
SRCREV = "a25db88a7858f32948a4995046279d896d84e359"

S = "${WORKDIR}/git"

EXT_DT_VERSION = "v6.0"
EXT_DT_RELEASE = "stm32mp-r3.1"

PV = "${EXT_DT_VERSION}-${EXT_DT_RELEASE}"

COMPATIBLE_MACHINE = "(stm32mpcommon)"

require external-dt-common.inc

# ---------------------------------
# Configure archiver use
# ---------------------------------
include ${@oe.utils.ifelse(d.getVar('ST_ARCHIVER_ENABLE') == '1', 'external-dt-archiver.inc','')}
