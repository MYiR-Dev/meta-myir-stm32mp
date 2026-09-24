#!/bin/bash

if [ -z "${DEVICE}" ]; then
	MMC_info=$(dmesg | grep "MMC")
	MMCtest=$(echo "$MMC_info" | cut -d':' -f1 | awk '{print $3}')
	MMCNUM=$(echo "$MMCtest" | awk -F'c' '{print $2}')
	DEVICE=/dev/mmcblk${MMCNUM}
fi

echo "$DEVICE"

# Match FlashLayout_emmc_myb-stm32mp257x-1GB-optee-ota-encrypt.tsv.
# Slot B partitions are created empty and are populated by the first OTA update.
sgdisk --clear \
    --set-alignment=1 \
    --new=1:1024:+512KiB --typecode=1:8a7a84a0-8387-40f6-ab41-a8b9a5a60d23 --change-name=1:metadata1 --partition-guid=1:b19f76dc-51b6-47ff-9fcd-ef229957c4b2 \
    --new=2:2048:+512KiB --typecode=2:8a7a84a0-8387-40f6-ab41-a8b9a5a60d23 --change-name=2:metadata2 --partition-guid=2:2234ca05-25e5-442a-95d8-2372adfd1e2f \
    --new=3:3072:+4MiB --typecode=3:19d5df83-11b0-457b-be2c-7559c13142a5 --change-name=3:fip-a --partition-guid=3:4fd84c93-54ef-463f-a7ef-ae25ff887087 \
    --new=4:11264:+4MiB --typecode=4:19d5df83-11b0-457b-be2c-7559c13142a5 --change-name=4:fip-b --partition-guid=4:09c54952-d5bf-45af-acee-335303766fb3 \
    --new=5:19456:+512KiB --typecode=5:3de21764-95bd-54bd-a5c3-4abe786f38a8 --change-name=5:u-boot-env --partition-guid=5:91213942-1460-4da7-8351-936656c5cd99 \
    --new=6:20480:+81MiB --typecode=6:0fc63daf-8483-4772-8e79-3d69d8477de4 --change-name=6:bootfs-a --partition-guid=6:95a33ae7-111c-43da-917e-ed818a70dee8 --attributes=6:set:2 \
    --new=7:186368:+81MiB --typecode=7:0fc63daf-8483-4772-8e79-3d69d8477de4 --change-name=7:bootfs-b --partition-guid=7:09cd2277-4858-403a-8589-b30f3a1528c0 \
    --new=8:352256:+64MiB --typecode=8:0fc63daf-8483-4772-8e79-3d69d8477de4 --change-name=8:vendorfs-a --partition-guid=8:dff396ae-895e-486d-a5f5-b0b58b1d9cc8 \
    --new=9:483328:+64MiB --typecode=9:0fc63daf-8483-4772-8e79-3d69d8477de4 --change-name=9:vendorfs-b --partition-guid=9:d95e66dc-f31e-45f7-9d14-08713cdce6b9 \
    --new=10:614400:+3GiB --typecode=10:0fc63daf-8483-4772-8e79-3d69d8477de4 --change-name=10:rootfs-a --partition-guid=10:491f6117-415d-4f53-88c9-6e0de54deac6 \
    --new=11:6905856:+3GiB --typecode=11:0fc63daf-8483-4772-8e79-3d69d8477de4 --change-name=11:rootfs-b --partition-guid=11:1e76153c-33d5-4c86-98cd-42034f50f756 \
    --new=12:13197312:0 --typecode=12:0fc63daf-8483-4772-8e79-3d69d8477de4 --change-name=12:userfs --partition-guid=12:59c76759-7f44-4a90-95ce-2cca2aaa362c \
    "$DEVICE"
