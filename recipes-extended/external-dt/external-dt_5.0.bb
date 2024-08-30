SUMMARY = "Provides Device Tree files for STM32MP boards"
LICENSE = "GPL-2.0-only"
LIC_FILES_CHKSUM = "file://${COMMON_LICENSE_DIR}/GPL-2.0-only;md5=801f80980d171dd6425610833a22dbe6"

#SRC_URI = "git://github.com/STMicroelectronics/dt-stm32mp.git;protocol=https;branch=v5-stm32mp"
EXTDTB_SRC_URI ?= "git://github.com/MYiR-Dev/myir-st-external-dt.git;protocol=https"
SRC_URI = "${EXTDTB_SRC_URI};branch=develop-ld25x-v5"
SRCREV = "2cbc6bfe93288e69a11956c8ced7ca45a90dec79"
#SRCREV= "${AUTOREV}"

EXT_DT_VERSION = "v5.0"
EXT_DT_RELEASE = "stm32mp-r1"

PV = "${EXT_DT_VERSION}-${EXT_DT_RELEASE}"

S = "${WORKDIR}/git"

COMPATIBLE_MACHINE = "(stm32mpcommon)"

require external-dt-common.inc

# ---------------------------------
# Configure archiver use
# ---------------------------------
include ${@oe.utils.ifelse(d.getVar('ST_ARCHIVER_ENABLE') == '1', 'external-dt-archiver.inc','')}
