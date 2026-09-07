SUMMARY = "STM32MP25 secure boot keys"
DESCRIPTION = "Deploys manual secure boot keys or generates a fallback key set"
LICENSE = "LGPLv2"
LIC_FILES_CHKSUM = "file://LICENSE;md5=309cc7bace8769cfabdd34577f654f8e"

EXCLUDE_FROM_WORLD = "1"
INHIBIT_DEFAULT_DEPS = "1"

STM32MP25_KEY_DIR ??= "${DEPLOY_DIR_IMAGE}/stm32mp25-key"
STM32MP25_MANUAL_KEY_DIR ??= "${WORKDIR}/stm32mp25"
STM32MP25_FORCE_KEYGEN ??= "0"
STM32_CUBE_PROGRAMMER_PATH ??= "${HOME}/STMicroelectronics/STM32Cube/STM32CubeProgrammer/bin"

DEPENDS = "openssl-native"

SRC_URI += "file://LICENSE file://stm32mp25"
S = "${WORKDIR}"

do_configure[noexec] = "1"
do_compile[noexec] = "1"

do_keygen() {
    export PATH="${STM32_CUBE_PROGRAMMER_PATH}:$PATH"

    manual_dir="${STM32MP25_MANUAL_KEY_DIR}"
    if [ -d "${STM32MP25_MANUAL_KEY_DIR}/stm32mp25" ]; then
        manual_dir="${STM32MP25_MANUAL_KEY_DIR}/stm32mp25"
    fi
    mkdir -p "${STM32MP25_KEY_DIR}"

    manual_complete=1
    for key in privateKey00.pem publicKey00.pem stm32mp_encryption_key.bin; do
        [ -s "$manual_dir/$key" ] || manual_complete=0
    done
    if [ ! -s "$manual_dir/FIP_EDMK.bin" ] && \
       [ ! -s "$manual_dir/stm32mp_encryption_key_256bits.bin" ]; then
        manual_complete=0
    fi

    if [ "${STM32MP25_FORCE_KEYGEN}" != "1" ] && [ "$manual_complete" = "1" ]; then
        bbnote "Using manual STM32MP25 keys from $manual_dir"

        # A malformed manual set must never silently fall back to unrelated keys.
        ${STAGING_BINDIR_NATIVE}/openssl pkey \
            -in "$manual_dir/privateKey00.pem" -passin pass:0000 -noout
        ${STAGING_BINDIR_NATIVE}/openssl pkey -pubin \
            -in "$manual_dir/publicKey00.pem" -noout
        private_hash="$(${STAGING_BINDIR_NATIVE}/openssl pkey \
            -in "$manual_dir/privateKey00.pem" -passin pass:0000 \
            -pubout -outform DER | sha256sum | cut -d' ' -f1)"
        public_hash="$(${STAGING_BINDIR_NATIVE}/openssl pkey -pubin \
            -in "$manual_dir/publicKey00.pem" -outform DER | \
            sha256sum | cut -d' ' -f1)"
        [ "$private_hash" = "$public_hash" ] || \
            bbfatal "Manual privateKey00.pem does not match publicKey00.pem"
        [ "$(wc -c < "$manual_dir/stm32mp_encryption_key.bin")" -eq 16 ] || \
            bbfatal "Manual FSBL encryption key must be exactly 16 bytes"

        fip_key="$manual_dir/FIP_EDMK.bin"
        if [ ! -s "$fip_key" ]; then
            fip_key="$manual_dir/stm32mp_encryption_key_256bits.bin"
        fi
        [ "$(wc -c < "$fip_key")" -eq 32 ] || \
            bbfatal "Manual FIP encryption key must be exactly 32 bytes"

        rm -f "${STM32MP25_KEY_DIR}"/privateKey*.pem \
              "${STM32MP25_KEY_DIR}"/publicKey*.pem \
              "${STM32MP25_KEY_DIR}"/publicKey*.der \
              "${STM32MP25_KEY_DIR}"/*.bin
        install -m 0600 "$manual_dir/privateKey00.pem" \
            "${STM32MP25_KEY_DIR}/privateKey00.pem"
        for key in "$manual_dir"/publicKey*.pem; do
            [ -f "$key" ] || continue
            ${STAGING_BINDIR_NATIVE}/openssl pkey -pubin -in "$key" -noout
            install -m 0644 "$key" \
                "${STM32MP25_KEY_DIR}/$(basename "$key")"
        done
        install -m 0600 "$manual_dir/stm32mp_encryption_key.bin" \
            "${STM32MP25_KEY_DIR}/stm32mp_encryption_key.bin"
        install -m 0600 "$fip_key" \
            "${STM32MP25_KEY_DIR}/stm32mp_encryption_key_256bits.bin"
        echo "manual:$manual_dir" > "${STM32MP25_KEY_DIR}/.key-source"
    else
        if [ -d "$manual_dir" ] && [ "${STM32MP25_FORCE_KEYGEN}" != "1" ]; then
            bbwarn "Manual STM32MP25 key set in $manual_dir is incomplete; using generated keys"
        else
            bbnote "No complete manual STM32MP25 key set found; using generated keys"
        fi
        command -v STM32_KeyGen_CLI >/dev/null 2>&1 || \
            bbfatal "STM32_KeyGen_CLI not found; check STM32_CUBE_PROGRAMMER_PATH"

        if grep -q '^manual:' "${STM32MP25_KEY_DIR}/.key-source" 2>/dev/null || \
           [ "${STM32MP25_FORCE_KEYGEN}" = "1" ]; then
            rm -f "${STM32MP25_KEY_DIR}"/privateKey*.pem \
                  "${STM32MP25_KEY_DIR}"/publicKey*.pem \
                  "${STM32MP25_KEY_DIR}"/publicKey*.der \
                  "${STM32MP25_KEY_DIR}"/*.bin
        fi

        generate_sign_keys=0
        [ -s "${STM32MP25_KEY_DIR}/privateKey00.pem" ] || generate_sign_keys=1
        for index in 00 01 02 03 04 05 06 07; do
            [ -s "${STM32MP25_KEY_DIR}/publicKey${index}.pem" ] || generate_sign_keys=1
        done
        if [ "$generate_sign_keys" = "1" ]; then
            rm -f "${STM32MP25_KEY_DIR}"/privateKey*.pem \
                  "${STM32MP25_KEY_DIR}"/publicKey*.pem \
                  "${STM32MP25_KEY_DIR}"/publicKey*.der
            bbnote "Generating 8 RSA key pairs..."
            STM32_KeyGen_CLI -abs "${STM32MP25_KEY_DIR}/" \
                -pwd 0000 1111 2222 3333 4444 5555 6666 7777 \
                -n 8 || bbfatal "Failed to generate STM32MP25 key pairs"
        fi
        if [ ! -s "${STM32MP25_KEY_DIR}/stm32mp_encryption_key.bin" ]; then
            bbnote "Generating FSBL encryption key (16 bytes)..."
            STM32_KeyGen_CLI -rand 16 \
                "${STM32MP25_KEY_DIR}/stm32mp_encryption_key.bin" || \
                bbfatal "Failed to generate FSBL encryption key"
        fi
        if [ ! -s "${STM32MP25_KEY_DIR}/stm32mp_encryption_key_256bits.bin" ]; then
            bbnote "Generating FIP encryption key (32 bytes)..."
            STM32_KeyGen_CLI -rand 32 \
                "${STM32MP25_KEY_DIR}/stm32mp_encryption_key_256bits.bin" || \
                bbfatal "Failed to generate FIP encryption key"
        fi
        echo "generated" > "${STM32MP25_KEY_DIR}/.key-source"
    fi

    # Keep the STM32 reference name alongside Yocto's configured FIP key name.
    install -m 0600 \
        "${STM32MP25_KEY_DIR}/stm32mp_encryption_key_256bits.bin" \
        "${STM32MP25_KEY_DIR}/FIP_EDMK.bin"

    if [ ! -s "${STM32MP25_KEY_DIR}/publicKey00.der" ]; then
        ${STAGING_BINDIR_NATIVE}/openssl pkey -pubin \
            -in "${STM32MP25_KEY_DIR}/publicKey00.pem" \
            -outform DER -out "${STM32MP25_KEY_DIR}/publicKey00.der"
    fi

    chmod 600 "${STM32MP25_KEY_DIR}"/privateKey*.pem 2>/dev/null || true
    chmod 644 "${STM32MP25_KEY_DIR}"/publicKey*.pem 2>/dev/null || true
    chmod 600 "${STM32MP25_KEY_DIR}"/*.bin 2>/dev/null || true
    chmod 644 "${STM32MP25_KEY_DIR}"/*.der 2>/dev/null || true
    {
        echo "STM32MP25 secure boot key deployment"
        echo "Source: $(cat "${STM32MP25_KEY_DIR}/.key-source")"
        echo "Key directory: ${STM32MP25_KEY_DIR}"
    } > "${STM32MP25_KEY_DIR}/key_generation_info.txt"
    chmod 644 "${STM32MP25_KEY_DIR}/key_generation_info.txt" \
              "${STM32MP25_KEY_DIR}/.key-source"
    bbnote "STM32MP25 keys are ready in ${STM32MP25_KEY_DIR}"
}

addtask keygen after do_unpack do_prepare_recipe_sysroot before do_build
do_keygen[depends] += "openssl-native:do_populate_sysroot"
do_keygen[nostamp] = "1"
