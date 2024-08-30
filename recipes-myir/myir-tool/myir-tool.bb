DESCRIPTION = "myir tool and wifi firmware"
LICENSE = "LGPLv2"
LIC_FILES_CHKSUM = "file://LICENSE;md5=309cc7bace8769cfabdd34577f654f8e"

SRC_URI += " \
		file://etc/myir_test/ \
		file://etc/hostapd.conf \
		file://etc/udhcpd.conf \
		file://bcmd  \
 		file://usr/bin/ \
		file://usr/lib/libmyir_code.so.1 \ 
		file://usr/lib/locale/ \ 
		file://10-static-end1.network \
		file://11-static-end2.network \
		file://LICENSE \
"
S="${WORKDIR}"

do_install() {

	install -d ${D}${bindir}
	install -d ${D}${nonarch_base_libdir}/firmware/bcmd/
	install -d ${D}/etc/myir_test/
	install -d ${D}/etc/
	install -d ${D}/usr/lib
	install -d ${D}/${sysconfdir}/systemd/network/
	install -d ${D}/usr/lib/locale/

	cp -rf ${S}/usr/lib/locale/*     ${D}/usr/lib/locale/
	install -m 755 ${S}/10-static-end1.network  ${D}/${sysconfdir}/systemd/network/
	install -m 755 ${S}/11-static-end2.network  ${D}/${sysconfdir}/systemd/network/
        install -m 755 ${S}/etc/myir_test/* ${D}/etc/myir_test/ 
        install -m 755 ${S}/etc/hostapd.conf ${D}/etc/hostapd.conf 
        install -m 755 ${S}/etc/udhcpd.conf ${D}/etc/udhcpd.conf
	install -m 755 ${S}${bindir}/* ${D}/${bindir}/
        install -m 0644 ${S}/bcmd/* ${D}${nonarch_base_libdir}/firmware/bcmd/ 
	cp  -rf ${S}/usr/lib/libmyir_code.so.1 ${D}/usr/lib/libmyir_code.so.1
 	cd ${D}/usr/lib
	ln -sf libmyir_code.so.1 libmyir_code.so
        
}

FILES:${PN} =" ${bindir}   \
              ${nonarch_base_libdir}/firmware/bcmd/  \
	      /etc/myir_test/ \
	      /etc/ \
	      /usr/lib \
	     /usr/lib/locale/ \
	     ${sysconfdir}/systemd/network/ \
"
FILES_${PN} += "${libdir}/*.so.1"
FILES_${PN}-dbg += "${libdir}/.debug"
INSANE_SKIP_${PN} = "ldflags"
INHIBIT_PACKAGE_DEBUG_SPLIT = "1"
INHIBIT_PACKAGE_STRIP = "1"
INSANE_SKIP_${PN} = "${ERROR_QA} ${WARN_QA}"
INSANE_SKIP:${PN} = "file-rdeps"
