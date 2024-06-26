SUMMARY = "A small image just capable of allowing a device to boot."

IMAGE_FEATURES += "ssh-server-openssh package-management"
IMAGE_INSTALL = "packagegroup-core-boot ${CORE_IMAGE_EXTRA_INSTALL} \
		mraa \
		upm \
		python3-mraa \
		python3-upm \
		python3-pip \
		wpa-supplicant \
		paho-mqtt-c \
		e2fsprogs \
		bluez5 \
		iproute2 \
"

IMAGE_LINGUAS = " "

LICENSE = "MIT"

inherit core-image

IMAGE_ROOTFS_SIZE ?= "8192"
IMAGE_ROOTFS_EXTRA_SPACE:append = "${@bb.utils.contains("DISTRO_FEATURES", "systemd", " + 4096", "" ,d)}"



