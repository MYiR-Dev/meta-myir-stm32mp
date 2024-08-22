DESCRIPTION = "myir tool and wifi firmware"
LICENSE = "LGPLv2"
LIC_FILES_CHKSUM = "file://LICENSE;md5=309cc7bace8769cfabdd34577f654f8e"

SRC_URI += " \
		file://etc/myir_test/ \
		file://bcmd  \
 		file://usr/bin/ \
		file://LICENSE \
"
S="${WORKDIR}"

do_install() {

	install -d ${D}${bindir}
	install -d ${D}${nonarch_base_libdir}/firmware/bcmd/
	install -d ${D}/etc/myir_test/

        install -m 755 ${S}/etc/myir_test/* ${D}/etc/myir_test/ 
	install -m 755 ${S}${bindir}/* ${D}/${bindir}/
        install -m 0644 ${S}/bcmd/* ${D}${nonarch_base_libdir}/firmware/bcmd/ 
        
}

FILES:${PN} =" ${bindir}   \
              ${nonarch_base_libdir}/firmware/bcmd/  \
	      /etc/myir_test/ \
"

INSANE_SKIP:${PN} = "file-rdeps"
INSANE_SKIP_${PN} = "${ERROR_QA} ${WARN_QA}"
