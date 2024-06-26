SECTION = "kernel"
DESCRIPTION = "Linux kernel for Microchip ARM SoCs (aka AT91)"
SUMMARY = "Linux kernel for Microchip ARM SoCs (aka AT91)"
LICENSE = "GPL-2.0-only"

inherit kernel

RDEPENDS:${KERNEL_PACKAGE_NAME}-base = ""

S = "${WORKDIR}/git"

PLATFORM_PATH ??= ""

python __anonymous () {
    if d.getVar('UBOOT_FIT_IMAGE', True) == 'xyes':
        d.appendVar('DEPENDS', ' u-boot-mkimage-native dtc-native')

    # Verify whether the kernel version is 6.6 or later and update the
    # device tree paths by prefixing them with our platform name.
    if d.getVar('LINUX_VERSION', True) >= '6.6':
        devicetree = d.getVar('KERNEL_DEVICETREE', True).split()
        devicetree = ['microchip/' + dtb for dtb in devicetree]
        d.setVar('KERNEL_DEVICETREE', ' '.join(devicetree))
        d.setVar('PLATFORM_PATH', 'microchip/')
        d.setVar('DTB_PATH', '${B}/arch/${ARCH}/boot/dts/microchip/')
}

do_configure:append() {
    frags=""
    for fragment in ${WORKDIR}/*.cfg
    do
        if [ -f ${fragment} ]; then
            cp -v ${fragment} ${B}
            frags=$frags" `basename ${fragment}`"
        fi
    done

    if [ ! -z "${frags}" ]; then
        echo "Fragments are: ${frags}"
        PATH=${S}/scripts/kconfig:${PATH}
        CFLAGS="${CFLAGS} ${TOOLCHAIN_OPTIONS}" HOSTCC="${BUILD_CC} ${BUILD_CFLAGS} ${BUILD_LDFLAGS}" HOSTCPP="${BUILD_CPP}" CC="${KERNEL_CC}" ARCH=${ARCH} merge_config.sh -m .config ${frags}  2>&1
        if [ $? -ne 0 ]; then
            bbfatal_log "Could not configure kernel fragments: ${frags}"
        fi
    fi
}

do_deploy:append() {
    if [ "${UBOOT_FIT_IMAGE}" = "xyes" ]; then
        if [ ! -e "${DTB_PATH}" ]; then
            DTB_PATH="${B}/arch/${ARCH}/boot/"
        fi

        if [ -e "${S}/arch/${ARCH}/boot/dts/${PLATFORM_PATH}${MACHINE}.its" ]; then
            cp ${S}/arch/${ARCH}/boot/dts/${PLATFORM_PATH}${MACHINE}*.its ${DTB_PATH}
            cd ${DTB_PATH}
            mkimage -f ${MACHINE}.its ${MACHINE}.itb

            # Ensure the deploy directory has the correct permissions
            chmod -R 755 ${DEPLOYDIR}
            chown -R $(whoami):$(whoami) ${DEPLOYDIR}

            install -m 0644 ${MACHINE}.itb ${DEPLOYDIR}/${MACHINE}.itb
            cd -
        else
            echo "ITS file ${PLATFORM_PATH}${MACHINE}.its not found in ${S}/arch/${ARCH}/boot/dts/"
            exit 1
        fi
    fi
}


do_install:append() {
    # Check if the directories exist before trying to remove them
    if [ -d "${D}/lib/modules/${KERNEL_VERSION}/source" ]; then
        rm -rf "${D}/lib/modules/${KERNEL_VERSION}/source"
    fi

    if [ -d "${D}/lib/modules/${KERNEL_VERSION}/build" ]; then
        rm -rf "${D}/lib/modules/${KERNEL_VERSION}/build"
    fi
}

KERNEL_MODULE_AUTOLOAD += "atmel_usba_udc g_serial"

