# Copyright (c) 2013 The Linux Foundation. All rights reserved.
#
include $(TOPDIR)/rules.mk

ARCH:=arm
BOARD:=cs75xx
BOARDNAME:=Cortina CS75x2
FEATURES:=squashfs nand fpu
CPU_TYPE:=cortex-a9
CPU_SUBTYPE:=neon
MAINTAINER:=John Crispin <john@phrozen.org>

KERNEL_PATCHVER:=4.9

KERNELNAME:=zImage Image dtbs

include $(INCLUDE_DIR)/target.mk
DEFAULT_PACKAGES += \
	kmod-leds-gpio kmod-gpio-button-hotplug swconfig \
	kmod-ata-core kmod-ata-ahci kmod-ata-ahci-platform \
	kmod-usb-core kmod-usb-ohci kmod-usb2 kmod-usb-ledtrig-usbport \
	uboot-envtools

$(eval $(call BuildTarget))
