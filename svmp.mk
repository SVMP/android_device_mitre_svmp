# MOCSI Makefile

ifdef NET_ETH0_STARTONBOOT
  PRODUCT_PROPERTY_OVERRIDES += net.eth0.startonboot=1
endif

$(call inherit-product, $(SRC_TARGET_DIR)/product/full_base.mk)

############## MOCSI ###########

DEVICE_PACKAGE_OVERLAYS := device/mitre/svmp/overlay

LOCAL_PATH := $(call my-dir)

KERNEL_SRC := kernel/android-3.4
KERNEL_BIN := $(KERNEL_SRC)/arch/x86/boot/bzImage
SVMP_KERN_CONFIG := device/mitre/svmp/svmp-kernel.config

PRODUCT_PROPERTY_OVERRIDES := \
    ro.ril.hsxpa=1 \
    ro.ril.gprsclass=10

PRODUCT_PACKAGES += \
		hwcomposer.default \
		librtsp_jni \
		fbset \
		static_busybox \
		SVMPProtocol \
		remote_events \
		audio.primary.svmp  \
		sensors.svmp  \
		libremote_events_jni \
		webrtc_helper

PRODUCT_COPY_FILES += \
    device/generic/goldfish/data/etc/apns-conf.xml:system/etc/apns-conf.xml \
    development/tools/emulator/system/camera/media_profiles.xml:system/etc/media_profiles.xml \
    development/tools/emulator/system/camera/media_codecs.xml:system/etc/media_codecs.xml \
    system/core/rootdir/etc/vold.fstab:system/etc/vold.fstab \
    device/generic/goldfish/data/etc/vold.conf:system/etc/vold.conf \
    device/mitre/svmp/init.rc:root/init.rc \
    device/mitre/svmp/init.svmp.rc:root/init.svmp.rc \
    device/mitre/svmp/fixaudio.sh:system/bin/fixaudio.sh \
    device/mitre/svmp/excluded-input-devices.xml:system/etc/excluded-input-devices.xml \
    frameworks/native/data/etc/android.hardware.touchscreen.multitouch.jazzhand.xml:system/etc/permissions/android.hardware.touchscreen.multitouch.jazzhand.xml \
    device/mitre/svmp/fbstream_webrtc:system/bin/fbstream_webrtc \
    device/mitre/svmp/svmp-fbstream-webrtc:system/bin/svmp-fbstream \
    $(KERNEL_BIN):kernel

ifeq ($(SVMP_BUILD_TYPE),virtio)
    PRODUCT_COPY_FILES += device/mitre/svmp/fstab.svmp.virtio:root/fstab.svmp
else
    PRODUCT_COPY_FILES += device/mitre/svmp/fstab.svmp:root/fstab.svmp
endif

# using a precompiled binary for now until we figure out in-tree building again for fbstream v3
#external/svmp/fbstream/trunk/out/Release/fbstream_webrtc :
#	external/svmp/fbstream/trunk/build.sh

.PHONY : do_fbstream
do_fbstream:
	device/mitre/svmp/fbstream-config.sh
device/mitre/svmp/fbstream_webrtc: do_fbstream


# No need to specify -j for the submake because the parent make process
# will communicate the information to the child
.PHONY: build_kernel
build_kernel:
	cp $(SVMP_KERN_CONFIG) $(KERNEL_SRC)/.config
	$(MAKE) ARCH=x86 bzImage -C $(KERNEL_SRC)
$(KERNEL_BIN): build_kernel

$(call inherit-product, frameworks/native/build/phone-xhdpi-1024-dalvik-heap.mk)

PRODUCT_POLICY := android.policy_phone

PRODUCT_NAME := svmp
PRODUCT_DEVICE := svmp
PRODUCT_BRAND := MITRE
PRODUCT_MODEL := MOCSI SVMP Thin Client VM
