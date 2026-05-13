SUMMARY = "for sdcard program"
DESCRIPTION = "use sdcard boot up and program full image to emmc"

LICENSE = "GPLv2"
LIC_FILES_CHKSUM = "file://licenses/GPL-2;md5=94d55d512a9ba36caa9b7df079bae19f"

inherit  systemd

S = "${WORKDIR}"

RDEPENDS:${PN} += "bash"

SRC_URI = "file://home/root/burn_flash.sh \
	   file://home/root/Manifest \
	   file://fac-burn-flash.service \
           file://licenses/GPL-2 \
          "

do_install(){
  install -d ${D}${systemd_system_unitdir}
	install -d ${D}/home/root/
	install -d ${D}/home/root/yf13x_images

	install -m 755 ${WORKDIR}/fac-burn-flash.service ${D}${systemd_system_unitdir}/fac-burn-flash.service

       	install -m 755 ${WORKDIR}/home/root/burn_flash.sh ${D}/home/root/burn_flash.sh
        install -m 755 ${WORKDIR}/home/root/Manifest ${D}/home/root/yf13x_images/Manifest
	install -m 755 ${DEPLOY_DIR_IMAGE}/myir-image-core-openstlinux-weston-myd-yf13x.rootfs_nand_4_256_512_multivolume.ubi  ${D}/home/root/yf13x_images/myir-image-core.ubi

	install -m 755 ${DEPLOY_DIR_IMAGE}/arm-trusted-firmware/tf-a-myb-stm32mp135x-512n512d-optee-spinand.stm32 ${D}/home/root/yf13x_images/tf-a-myb-stm32mp135x-512n512d-optee-spinand.stm32
	install -m 755 ${DEPLOY_DIR_IMAGE}/arm-trusted-firmware/metadata.bin ${D}/home/root/yf13x_images/metadata.bin
	install -m 755 ${DEPLOY_DIR_IMAGE}/fip/fip-myb-stm32mp135x-512n512d-optee-spinand.bin ${D}/home/root/yf13x_images/fip-myb-stm32mp135x-512n512d-optee-spinand.bin
	
}


FILES:${PN} = "/"

SYSTEMD_PACKAGES = "${PN}"
SYSTEMD_SERVICE:${PN} = "fac-burn-flash.service"
SYSTEMD_AUTO_ENABLE = "enable"
