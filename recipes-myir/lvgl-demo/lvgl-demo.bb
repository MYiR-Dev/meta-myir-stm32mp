SUMMARY = "lvgl demo"
DESCRIPTION = "lvgl demo"

DEPENDS = "libdrm "
LICENSE = "GPLv2"
LIC_FILES_CHKSUM = "file://licenses/GPL-2;md5=94d55d512a9ba36caa9b7df079bae19f"
PV = "0.1"
PR = "v1"

SRC_URI = " \
	   file://sbin/lvgl_demo \
	   file://usr/bin/autorun.sh \
	   file://autorun.service \
           file://licenses/GPL-2 \
          "
inherit  systemd          
S = "${WORKDIR}"

				
do_install (){
	install -d ${D}${sbindir}
	install -d ${D}${systemd_system_unitdir}
	install -d ${D}${bindir}
	
	install -m 0755 ${WORKDIR}/sbin/lvgl_demo ${D}${sbindir}/lvgl_demo
	install -m 644 ${WORKDIR}/autorun.service ${D}${systemd_system_unitdir}/autorun.service
	install -m 755 ${WORKDIR}${bindir}/autorun.sh ${D}${bindir}/autorun.sh

	
}

FILES:${PN} = "${sbindir} \
	      ${systemd_system_unitdir} \
	      ${bindir} \
"


SYSTEMD_PACKAGES = "${PN}"
SYSTEMD_SERVICE:${PN} = "autorun.service"
SYSTEMD_AUTO_ENABLE = "enable"

RDEPENDS:${PN} = " "
