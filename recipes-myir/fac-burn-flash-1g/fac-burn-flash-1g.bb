SUMMARY = "for sdcard program"
DESCRIPTION = "use sdcard boot up and program full image to emmc"

LICENSE = "GPLv2"
LIC_FILES_CHKSUM = "file://licenses/GPL-2;md5=94d55d512a9ba36caa9b7df079bae19f"

inherit  systemd

S = "${WORKDIR}"

RDEPENDS:${PN} += "bash"

OTA_SUPPORT ?= "1"
ENCRYPT_ENABLE ?= "0"
SIGN_ENABLE ?= "0"

# Normal burn: ENCRYPT_ENABLE=0, SIGN_ENABLE=0.
# Secure burn: ENCRYPT_ENABLE=1, SIGN_ENABLE=1.
BURN_FLASH_SECURITY_SUFFIX = "${@bb.utils.contains('ENCRYPT_ENABLE', '1', '_Encrypted_Signed', '', d)}"
BURN_FLASH_MANIFEST = "Manifest${@bb.utils.contains('ENCRYPT_ENABLE', '1', '-secure', '', d)}${@bb.utils.contains('OTA_SUPPORT', '1', '-ota', '', d)}"
BURN_FLASH_PARTITION_SCRIPT = "${@bb.utils.contains('OTA_SUPPORT', '1', 'fdisk_emmc_ota.sh', 'fdisk_emmc.sh', d)}"

SRC_URI = "file://home/root/burn_flash.sh \
	   file://home/root/fdisk_emmc.sh \
	   file://home/root/fdisk_emmc_ota.sh \
	   file://home/root/Manifest \
	   file://home/root/Manifest-ota \
	   file://home/root/Manifest-secure \
	   file://home/root/Manifest-secure-ota \
	   file://fac-burn-flash.service \
           file://licenses/GPL-2 \
          "

do_install[depends] += " \
    myir-image-emmc:do_image_complete \
"

python __anonymous () {
    if d.getVar('OTA_SUPPORT') not in ('0', '1'):
        bb.fatal('fac-burn-flash-1g: OTA_SUPPORT must be "0" or "1"')
    if d.getVar('ENCRYPT_ENABLE') not in ('0', '1'):
        bb.fatal('fac-burn-flash-1g: ENCRYPT_ENABLE must be "0" or "1"')
    if d.getVar('SIGN_ENABLE') not in ('0', '1'):
        bb.fatal('fac-burn-flash-1g: SIGN_ENABLE must be "0" or "1"')
    if d.getVar('ENCRYPT_ENABLE') != d.getVar('SIGN_ENABLE'):
        bb.fatal('fac-burn-flash-1g: normal burn requires ENCRYPT_ENABLE=0 and SIGN_ENABLE=0; secure burn requires both values to be 1')
}

do_install(){
	install -d ${D}${systemd_system_unitdir}
	install -d ${D}/home/root/
	install -d ${D}/home/root/ld25x_images

	install -m 755 ${WORKDIR}/fac-burn-flash.service ${D}${systemd_system_unitdir}/fac-burn-flash.service

	install -m 755 ${WORKDIR}/home/root/burn_flash.sh ${D}/home/root/burn_flash.sh
	install -m 755 ${WORKDIR}/home/root/${BURN_FLASH_PARTITION_SCRIPT} ${D}/home/root/fdisk_emmc.sh
	install -m 644 ${WORKDIR}/home/root/${BURN_FLASH_MANIFEST} ${D}/home/root/ld25x_images/Manifest

	install -m 644 ${DEPLOY_DIR_IMAGE}/myir-image-emmc-openstlinux-weston-${MACHINENAME}.splitted-rootfs.ext4 ${D}/home/root/ld25x_images/myir-image-emmc.splitted-rootfs.ext4
	install -m 644 ${DEPLOY_DIR_IMAGE}/myir-image-emmc-openstlinux-weston-${MACHINENAME}.splitted-bootfs.ext4 ${D}/home/root/ld25x_images/myir-image-emmc.splitted-bootfs.ext4
	install -m 644 ${DEPLOY_DIR_IMAGE}/myir-image-emmc-openstlinux-weston-${MACHINENAME}.splitted-userfs.ext4 ${D}/home/root/ld25x_images/myir-image-emmc.splitted-userfs.ext4
	install -m 644 ${DEPLOY_DIR_IMAGE}/myir-image-emmc-openstlinux-weston-${MACHINENAME}.splitted-vendorfs.ext4 ${D}/home/root/ld25x_images/myir-image-emmc.splitted-vendorfs.ext4

	install -m 644 ${DEPLOY_DIR_IMAGE}/arm-trusted-firmware/tf-a-myb-stm32mp257x-1GB-optee-emmc${BURN_FLASH_SECURITY_SUFFIX}.stm32 ${D}/home/root/ld25x_images/tf-a-myb-stm32mp257x-1GB-optee-emmc${BURN_FLASH_SECURITY_SUFFIX}.stm32
	install -m 644 ${DEPLOY_DIR_IMAGE}/arm-trusted-firmware/metadata.bin ${D}/home/root/ld25x_images/metadata.bin
	install -m 644 ${DEPLOY_DIR_IMAGE}/fip/fip-myb-stm32mp257x-1GB-optee-emmc${BURN_FLASH_SECURITY_SUFFIX}.bin ${D}/home/root/ld25x_images/fip-myb-stm32mp257x-1GB-optee-emmc${BURN_FLASH_SECURITY_SUFFIX}.bin
}


FILES:${PN} = "/"

SYSTEMD_PACKAGES = "${PN}"
SYSTEMD_SERVICE:${PN} = "fac-burn-flash.service"
SYSTEMD_AUTO_ENABLE = "enable"
