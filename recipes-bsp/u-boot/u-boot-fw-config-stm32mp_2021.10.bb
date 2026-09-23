SUMMARY = "U-Boot bootloader fw_printenv/setenv utilities"
LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://${COREBASE}/meta/files/common-licenses/MIT;md5=0835ade698e0bcf8506ecda2f7b4f302"

SRC_URI = " \
    file://fw_env.config.mmc \
    file://fw_env.config.nand \
    file://fw_env.config.nor \
"

DEPENDS += "u-boot-fw-utils"

UBOOT_FW_ENV_DEFAULT_CONFIG ??= ""

do_install () {
    install -d ${D}${sysconfdir}
    install -m 0644 ${WORKDIR}/fw_env.config.* ${D}${sysconfdir}/

    if [ -n "${UBOOT_FW_ENV_DEFAULT_CONFIG}" ]; then
        ln -sf fw_env.config.${UBOOT_FW_ENV_DEFAULT_CONFIG} \
            ${D}${sysconfdir}/fw_env.config
    fi
}

FILES:${PN} += "${sysconfdir}/"
RDEPENDS:${PN} += "u-boot-fw-utils"
