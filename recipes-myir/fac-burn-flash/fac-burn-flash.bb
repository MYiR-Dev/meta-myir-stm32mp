SUMMARY = "for sdcard program"
DESCRIPTION = "use sdcard boot up and program full image to emmc"

LICENSE = "GPLv2"
LIC_FILES_CHKSUM = "file://licenses/GPL-2;md5=94d55d512a9ba36caa9b7df079bae19f"

inherit  systemd

S = "${WORKDIR}"

RDEPENDS:${PN} += "bash"

SRC_URI = "file://home/root/burn_flash.sh \
	   file://home/root/fdisk_emmc.sh \
	   file://home/root/Manifest \
	   file://home/root/Manifest_SECURE \
	   file://fac-burn-flash.service \
           file://licenses/GPL-2 \
          "

do_install(){
  install -d ${D}${systemd_system_unitdir}
	install -d ${D}/home/root/
	install -d ${D}/home/root/yf13x_images

	install -m 755 ${WORKDIR}/fac-burn-flash.service ${D}${systemd_system_unitdir}/fac-burn-flash.service

       	install -m 755 ${WORKDIR}/home/root/burn_flash.sh ${D}/home/root/burn_flash.sh
       	install -m 755 ${WORKDIR}/home/root/fdisk_emmc.sh ${D}/home/root/fdisk_emmc.sh

	if [ "${SIGN_ENABLE}" = "1" ]
	then
		install -m 755 ${WORKDIR}/home/root/Manifest_SECURE ${D}/home/root/yf13x_images/Manifest
		install -m 755 ${DEPLOY_DIR_IMAGE}/myir-image-full-openstlinux-weston-myd-yf13x.rootfs.ext4  ${D}/home/root/yf13x_images/myir-image-full.ext4
                install -m 755 ${DEPLOY_DIR_IMAGE}/st-image-bootfs-openstlinux-weston-myd-yf13x.bootfs.ext4  ${D}/home/root/yf13x_images/st-image-bootfs.ext4
                install -m 755 ${DEPLOY_DIR_IMAGE}/myir-image-full-userfs-openstlinux-weston-myd-yf13x.userfs.ext4  ${D}/home/root/yf13x_images/st-image-userfs.ext4
                install -m 755 ${DEPLOY_DIR_IMAGE}/st-image-vendorfs-openstlinux-weston-myd-yf13x.vendorfs.ext4  ${D}/home/root/yf13x_images/st-image-vendorfs.ext4

                install -m 755 ${DEPLOY_DIR_IMAGE}/arm-trusted-firmware/tf-a-myb-stm32mp135x-512m-optee-emmc_Encrypted_Signed.stm3 ${D}/home/root/yf13x_images/tf-a-myb-stm32mp135x-512m-optee-emmc_Encrypted_Signed.stm3
                install -m 755 ${DEPLOY_DIR_IMAGE}/arm-trusted-firmware/metadata.bin ${D}/home/root/yf13x_images/metadata.bin
                install -m 755 ${DEPLOY_DIR_IMAGE}/fip/fip-myb-stm32mp135x-512m-optee-emmc_Encrypted_Signed.bin ${D}/home/root/yf13x_images/fip-myb-stm32mp135x-512m-optee-emmc_Encrypted_Signed.bin
	else
		install -m 755 ${WORKDIR}/home/root/Manifest ${D}/home/root/yf13x_images/Manifest
		install -m 755 ${DEPLOY_DIR_IMAGE}/myir-image-full-openstlinux-weston-myd-yf13x.rootfs.ext4  ${D}/home/root/yf13x_images/myir-image-full.ext4
		install -m 755 ${DEPLOY_DIR_IMAGE}/st-image-bootfs-openstlinux-weston-myd-yf13x.bootfs.ext4  ${D}/home/root/yf13x_images/st-image-bootfs.ext4
		install -m 755 ${DEPLOY_DIR_IMAGE}/myir-image-full-userfs-openstlinux-weston-myd-yf13x.userfs.ext4  ${D}/home/root/yf13x_images/st-image-userfs.ext4
		install -m 755 ${DEPLOY_DIR_IMAGE}/st-image-vendorfs-openstlinux-weston-myd-yf13x.vendorfs.ext4  ${D}/home/root/yf13x_images/st-image-vendorfs.ext4

		install -m 755 ${DEPLOY_DIR_IMAGE}/arm-trusted-firmware/tf-a-myb-stm32mp135x-512m-optee-emmc.stm32 ${D}/home/root/yf13x_images/tf-a-myb-stm32mp135x-512m-optee-emmc.stm32
		install -m 755 ${DEPLOY_DIR_IMAGE}/arm-trusted-firmware/metadata.bin ${D}/home/root/yf13x_images/metadata.bin
		install -m 755 ${DEPLOY_DIR_IMAGE}/fip/fip-myb-stm32mp135x-512m-optee-emmc.bin ${D}/home/root/yf13x_images/fip-myb-stm32mp135x-512m-optee-emmc.bin
	fi
	
}


FILES:${PN} = "/"

SYSTEMD_PACKAGES = "${PN}"
SYSTEMD_SERVICE:${PN} = "fac-burn-flash.service"
SYSTEMD_AUTO_ENABLE = "enable"
