SUMMARY = "myir baord info"
DESCRIPTION = "sometimes driver will request firmware and wait for a 60s or 120s, We feed the firmware to it"

LICENSE = "GPLv2"
LIC_FILES_CHKSUM = "file://licenses/GPL-2;md5=94d55d512a9ba36caa9b7df079bae19f"
RDEPENDS:${PN} += "bash"

inherit  systemd

S = "${WORKDIR}"

SRC_URI = "file://hwrevision \
           file://licenses/GPL-2 \
           file://myir-swupdate.service \
           file://myir-swupdate.sh \
           file://board_part_info.conf \
           file://sw-versions \
           file://swupdate.cfg \
          "
          

HW_MAJOR ?="1"
HW_MINOR ?="0"

#EMMC_DEV ?="2"
#EMMC_DEV_mx8mm = "2"

#SD_DEV ?= "1"
#SD_DEV_mx8mm = "1"

#ROOTFS_A_PART ?="2"
#ROOTFS_B_PART ?="3"
					

do_install(){
	install -d ${D}${systemd_system_unitdir}
	install -d ${D}${sysconfdir}
	install -d ${D}/boot

	install -m 644 ${WORKDIR}/myir-swupdate.service ${D}${systemd_system_unitdir}/myir-swupdate.service
	install -m 755 ${WORKDIR}/myir-swupdate.sh ${D}${sysconfdir}/myir-swupdate.sh
	install -m 644 ${WORKDIR}/sw-versions ${D}${sysconfdir}/sw-versions
	install -m 644 ${WORKDIR}/hwrevision ${D}${sysconfdir}/hwrevision
	install -m 644 ${WORKDIR}/swupdate.cfg ${D}${sysconfdir}/swupdate.cfg
	echo "${MACHINE} ${HW_MAJOR}.${HW_MINOR}" >  ${D}${sysconfdir}/hwrevision
	
	install -m 644 ${WORKDIR}/board_part_info.conf ${D}${sysconfdir}/board_part_info.conf
	
	install -m 755 ${DEPLOY_DIR_IMAGE}/kernel/Image ${D}/boot/Image
	install -m 755 ${DEPLOY_DIR_IMAGE}/kernel/myb-stm32mp257x-2GB.dtb ${D}/boot/myb-stm32mp257x-2GB.dtb
	install -m 755 ${DEPLOY_DIR_IMAGE}/kernel/myb-stm32mp257x-1GB.dtb ${D}/boot/myb-stm32mp257x-1GB.dtb
}

FILES:${PN} = "/"

SYSTEMD_PACKAGES = "${PN}"
SYSTEMD_SERVICE:${PN} = "myir-swupdate.service"
SYSTEMD_AUTO_ENABLE = "enable"
