define Build/buffalo-initramfs-trx
	$(STAGING_DIR_HOST)/bin/trx $(1) \
		-o $@.new -f $@ -a 4
  mv $@.new $@
endef

define Device/buffalo-wsr-2533dhp2
  DEVICE_TITLE := Buffalo WSR-2533DHP2
  DEVICE_DTS := mt7622-buffalo-wsr-2533dhp2
  DEVICE_DTS_DIR := $(DTS_DIR)/mediatek
  BLOCK_SIZE := 128k
  PAGESIZE := 2048
  KERNEL_INITRAMFS = kernel-bin | lzma | \
    fit lzma $$(KDIR)/image-$$(firstword $$(DEVICE_DTS)).dtb | \
    buffalo-initramfs-trx -M 44485032
endef
TARGET_DEVICES += buffalo-wsr-2533dhp2

define Device/mediatek_mt7622-rfb1
  DEVICE_VENDOR := MediaTek
  DEVICE_MODEL := MTK7622 rfb1 AP
  DEVICE_DTS := mt7622-rfb1
  DEVICE_DTS_DIR := $(DTS_DIR)/mediatek
  DEVICE_PACKAGES := kmod-usb-ohci kmod-usb2 kmod-usb3 \
			kmod-ata-core kmod-ata-ahci-mtk
endef
TARGET_DEVICES += mediatek_mt7622-rfb1

define Device/mediatek_mt7622-lynx-rfb1
  DEVICE_VENDOR := MediaTek
  DEVICE_MODEL := MTK7622 Lynx rfb1 AP
  DEVICE_DTS := mt7622-lynx-rfb1
  DEVICE_DTS_DIR := $(DTS_DIR)/mediatek
  SUPPORTED_DEVICES := mediatek,mt7622-rfb1
  DEVICE_PACKAGES := kmod-usb-ohci kmod-usb2 kmod-usb3 \
			kmod-ata-core kmod-ata-ahci-mtk
endef
TARGET_DEVICES += mediatek_mt7622-lynx-rfb1

define Device/lemaker_bananapi-bpi-r64
  DEVICE_VENDOR := LeMaker
  DEVICE_MODEL := Banana Pi R64
  DEVICE_DTS := mt7622-bananapi-bpi-r64
  DEVICE_DTS_DIR := $(DTS_DIR)/mediatek
  SUPPORTED_DEVICES := bananapi,bpi-r64
  DEVICE_PACKAGES := kmod-usb-ohci kmod-usb2 kmod-usb3 \
			kmod-ata-core kmod-ata-ahci-mtk
endef
TARGET_DEVICES += lemaker_bananapi-bpi-r64
