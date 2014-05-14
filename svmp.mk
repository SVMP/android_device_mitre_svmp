# MOCSI Makefile

ifdef NET_ETH0_STARTONBOOT
  PRODUCT_PROPERTY_OVERRIDES += net.eth0.startonboot=1
endif

$(call inherit-product, $(SRC_TARGET_DIR)/product/full_base.mk)

############## MOCSI ###########

DEVICE_PACKAGE_OVERLAYS := device/mitre/svmp/overlay

LOCAL_PATH := $(call my-dir)

# 32-bit android kernel
KERNEL_SRC := kernel/android-3.4
KERNEL_BIN := $(KERNEL_SRC)/arch/x86/boot/bzImage
SVMP_KERN_CONFIG := device/mitre/svmp/svmp-kernel.config

# New Intel 64-bit android kernel
# remove "ARCH=x86" from the kernel build line below if this is used
#KERNEL_SRC := kernel/intel
#SVMP_KERN_CONFIG := device/mitre/svmp/svmp-kernel.config.intel-4.2.2

PRODUCT_PROPERTY_OVERRIDES := \
    ro.ril.hsxpa=1 \
    ro.ril.gprsclass=10

PRODUCT_PACKAGES += \
		hwcomposer.default \
		librtsp_jni \
		fbset \
		SVMPProtocol \
		remote_events \
		audio.primary.svmp  \
		sensors.svmp  \
		libremote_events_jni \
		libjingle_peerconnection_so.so \
		scp sftp sshd ssh-keygen start-ssh \
		e2fsck \
		Email

PRODUCT_COPY_FILES += \
    device/generic/goldfish/data/etc/apns-conf.xml:system/etc/apns-conf.xml \
    development/tools/emulator/system/camera/media_profiles.xml:system/etc/media_profiles.xml \
    development/tools/emulator/system/camera/media_codecs.xml:system/etc/media_codecs.xml \
    system/core/rootdir/etc/vold.fstab:system/etc/vold.fstab \
    device/generic/goldfish/data/etc/vold.conf:system/etc/vold.conf \
    device/mitre/svmp/init.rc:root/init.rc \
    device/mitre/svmp/init.svmp.rc:root/init.svmp.rc \
    device/mitre/svmp/excluded-input-devices.xml:system/etc/excluded-input-devices.xml \
    frameworks/native/data/etc/android.hardware.touchscreen.multitouch.jazzhand.xml:system/etc/permissions/android.hardware.touchscreen.multitouch.jazzhand.xml \
    device/mitre/svmp/sshd_config.android:system/etc/ssh/sshd_config \
    device/mitre/svmp/print-netcfg:system/bin/print-netcfg \
    device/mitre/svmp/remount/remount.sh:system/bin/remount.sh \
    $(KERNEL_BIN):kernel

ifeq ($(SVMP_BUILD_TYPE),virtio)
    PRODUCT_COPY_FILES += device/mitre/svmp/fstab.svmp.virtio:root/fstab.svmp
else
    PRODUCT_COPY_FILES += device/mitre/svmp/fstab.svmp:root/fstab.svmp
endif

ifdef SVMP_AUTHORIZED_KEYS
    PRODUCT_COPY_FILES += $(SVMP_AUTHORIZED_KEYS):system/etc/ssh/authorized_keys
endif

# No need to specify -j for the submake because the parent make process
# will communicate the information to the child
# If building a 64-bit kernel, remove ARCH=x86 from the make line
.PHONY: build_kernel
build_kernel:
	cp $(SVMP_KERN_CONFIG) $(KERNEL_SRC)/.config
	$(MAKE) ARCH=x86 bzImage -C $(KERNEL_SRC)
$(KERNEL_BIN): build_kernel

$(call inherit-product, frameworks/native/build/phone-xhdpi-1024-dalvik-heap.mk)

ifdef SVMP_DEV_CERTIFICATE
    PRODUCT_DEFAULT_DEV_CERTIFICATE := $(SVMP_DEV_CERTIFICATE)
endif

PRODUCT_POLICY := android.policy_phone

PRODUCT_NAME := svmp
PRODUCT_DEVICE := svmp
PRODUCT_BRAND := MITRE
PRODUCT_MODEL := Secure Virtual Mobile Platform v1.3.1
