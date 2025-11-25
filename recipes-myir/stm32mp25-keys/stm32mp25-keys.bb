SUMMARY = "STM32MP25 Key Generation"
DESCRIPTION = "Generates secure boot keys for STM32MP25 platforms using STM32_KeyGen_CLI"
LICENSE = "LGPLv2"
LIC_FILES_CHKSUM = "file://LICENSE;md5=309cc7bace8769cfabdd34577f654f8e"

#Prevent this recipe from being included in world builds
EXCLUDE_FROM_WORLD = "1"

# Key directory variable
STM32MP25_KEY_DIR ??= "${DEPLOY_DIR_IMAGE}/stm32mp25-key"

# STM32CubeProgrammer path configuration
STM32_CUBE_PROGRAMMER_PATH ??= "${HOME}/STMicroelectronics/STM32Cube/STM32CubeProgrammer/bin"

# No dependencies needed since using external tool
DEPENDS = ""

SRC_URI += " \
	file://LICENSE \
"
S="${WORKDIR}"

do_configure[noexec] = "1"
do_compile[noexec] = "1"

python do_check_programmer() {
    import os
    import subprocess
    
    # Check if STM32CubeProgrammer is in PATH
    try:
        subprocess.run(["STM32_KeyGen_CLI", "-h"], 
                      capture_output=True, check=True, 
                      env=dict(os.environ, PATH=d.getVar('STM32_CUBE_PROGRAMMER_PATH') + ':' + os.environ.get('PATH', '')))
    except (subprocess.CalledProcessError, FileNotFoundError):
        # If not in PATH, check specified path
        keygen_cli = os.path.join(d.getVar('STM32_CUBE_PROGRAMMER_PATH'), 'STM32_KeyGen_CLI')
        if not os.path.exists(keygen_cli):
            bb.fatal("STM32_KeyGen_CLI not found. Please install STM32CubeProgrammer and set STM32_CUBE_PROGRAMMER_PATH")
}

#  Key generation task
do_keygen() {
    # Set environment variable to include STM32CubeProgrammer path
    export PATH="${STM32_CUBE_PROGRAMMER_PATH}:$PATH"
    
    # Verify tool availability
    if ! command -v STM32_KeyGen_CLI >/dev/null 2>&1; then
        bberror "STM32_KeyGen_CLI not found in PATH. Please check STM32_CUBE_PROGRAMMER_PATH setting"
        exit 1
    fi
    
     # Create key directory
    mkdir -p "${STM32MP25_KEY_DIR}"
    
     # Check if forced regeneration or keys don't exist
    local need_generate=0
    if [ "${STM32MP25_FORCE_KEYGEN}" = "1" ]; then
        bbnote "Force regenerating STM32MP25 keys as requested"
        need_generate=1
    elif [ ! -f "${STM32MP25_KEY_DIR}/privateKey00.pem" ] || \
         [ ! -f "${STM32MP25_KEY_DIR}/stm32mp_encryption_key.bin" ] || \
         [ ! -f "${STM32MP25_KEY_DIR}/stm32mp_encryption_key_256bits.bin" ]; then
        bbnote "Some keys are missing, generating new keys"
        need_generate=1
    else
        bbnote "STM32MP25 keys already exist, skipping generation"
        need_generate=0
    fi
    
    if [ ${need_generate} -eq 1 ]; then
        bbnote "Generating STM32MP25 keys in ${STM32MP25_KEY_DIR}"
        
         # Generate key pairs
        bbnote "Generating 8 RSA key pairs..."
        STM32_KeyGen_CLI -abs "${STM32MP25_KEY_DIR}/" \
            -pwd 0000 1111 2222 3333 4444 5555 6666 7777 \
            -n 8
        
        if [ $? -ne 0 ]; then
            bberror "Failed to generate STM32MP25 key pairs"
            exit 1
        fi
        
        # Generate FSBL encryption key
        bbnote "Generating FSBL encryption key (16 bytes)..."
        STM32_KeyGen_CLI -rand 16 "${STM32MP25_KEY_DIR}/stm32mp_encryption_key.bin"
        
        if [ $? -ne 0 ]; then
            bberror "Failed to generate FSBL encryption key"
            exit 1
        fi
        
        # Generate FIP encryption key
        bbnote "Generating FIP encryption key (32 bytes)..."
        STM32_KeyGen_CLI -rand 32 "${STM32MP25_KEY_DIR}/stm32mp_encryption_key_256bits.bin"
        
        if [ $? -ne 0 ]; then
            bberror "Failed to generate FIP encryption key"
            exit 1
        fi
        
        # Set secure permissions
        chmod 600 "${STM32MP25_KEY_DIR}"/*.pem 2>/dev/null || true
        chmod 600 "${STM32MP25_KEY_DIR}"/*.bin 2>/dev/null || true
        
        # Generate key fingerprint file
        bbnote "Generating key fingerprints..."
        {
            echo "STM32MP25 Key Fingerprints"
            echo "Generated: $(date)"
            echo "Key Directory: ${STM32MP25_KEY_DIR}"
            echo ""
            echo "Key Files:"
            ls -la "${STM32MP25_KEY_DIR}/"
        } > "${STM32MP25_KEY_DIR}/key_generation_info.txt"
        
        chmod 644 "${STM32MP25_KEY_DIR}/key_generation_info.txt"
        
        bbnote "STM32MP25 keys generated successfully in ${STM32MP25_KEY_DIR}"
    fi
}

addtask check_programmer before do_keygen
addtask keygen
