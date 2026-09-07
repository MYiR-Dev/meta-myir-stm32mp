SUMMARY = "Temp Ctrl"
DESCRIPTION = "Temperature Control"
LICENSE = "GPL-2"
LIC_FILES_CHKSUM = "file://licenses/GPL-2;md5=94d55d512a9ba36caa9b7df079bae19f"

S = "${WORKDIR}"

SRC_URI = " \
     file://licenses/GPL-2 \
     file://eeprom-pnsn.service \
     file://libmyir_code.so.1 \
     file://MEasyListen-DEV \
     "
inherit systemd

do_install() {
        install -d -m 755 ${D}${systemd_system_unitdir}
	install -d ${D}/usr/bin 
	install -d ${D}/usr/lib

        install -m 644 ${WORKDIR}/eeprom-pnsn.service ${D}${systemd_system_unitdir}/eeprom-pnsn.service
	install -m 755 ${WORKDIR}/MEasyListen-DEV ${D}/usr/bin
	install -m 755  ${WORKDIR}/libmyir_code.so.1 ${D}/usr/lib
}

SYSTEMD_PACKAGES = "${PN}"
SYSTEMD_SERVICE:${PN} = "eeprom-pnsn.service"
SYSTEMD_AUTO_ENABLE = "enable"

FILES:${PN} =" /usr/lib  \
		/usr/bin/MEasyListen-DEV \
		${systemd_system_unitdir}/eeprom-pnsn.service \
"
#INSANE_SKIP:eeprom-pnsn-dev += "dev-elf"
FILES:${PN} += " /usr/lib/libmyir_code.so*  "
FILES:${PN}-dev = " /usr/lib/libmyir_code.so"
INSANE_SKIP:${PN} += " dev-so already-stripped"
