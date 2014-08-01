# MOCSI Makefile

ifdef NET_ETH0_STARTONBOOT
  PRODUCT_PROPERTY_OVERRIDES += net.eth0.startonboot=1
endif

$(call inherit-product, $(SRC_TARGET_DIR)/product/aosp_base.mk)

############## SVMP ###########

DEVICE_PACKAGE_OVERLAYS := device/mitre/svmp/overlay

LOCAL_PATH := $(call my-dir)

# 32-bit android kernel
KERNEL_SRC := kernel/android
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
		e2fsck \
		Email

PRODUCT_COPY_FILES += \
    device/generic/goldfish/data/etc/apns-conf.xml:system/etc/apns-conf.xml \
    device/mitre/svmp/camera/media_profiles.xml:system/etc/media_profiles.xml \
    device/mitre/svmp/camera/media_codecs.xml:system/etc/media_codecs.xml \
    device/mitre/svmp/init.rc:root/init.rc \
    device/mitre/svmp/init.svmp.rc:root/init.svmp.rc \
    device/mitre/svmp/excluded-input-devices.xml:system/etc/excluded-input-devices.xml \
    frameworks/native/data/etc/android.hardware.touchscreen.multitouch.jazzhand.xml:system/etc/permissions/android.hardware.touchscreen.multitouch.jazzhand.xml \
    device/mitre/svmp/sshd_config.android:system/etc/ssh/sshd_config \
    device/mitre/svmp/print-netcfg:system/bin/print-netcfg \
    device/mitre/svmp/remount/remount.sh:system/bin/remount.sh \
    device/mitre/svmp/audio_policy.conf:system/etc/audio_policy.conf \
    $(KERNEL_BIN):kernel

########################################################################
# fstab
########################################################################

FSTAB_FILE := device/mitre/svmp/fstab/fstab.svmp

FSTAB_FILE := $(addsuffix .$(SVMP_DISK_TYPE),$(FSTAB_FILE))

ifeq ($(SVMP_AIO_BUILD),yes)
    FSTAB_FILE := $(addsuffix .aio,$(FSTAB_FILE))
endif

PRODUCT_COPY_FILES += $(FSTAB_FILE):root/fstab.svmp

########################################################################
# sshd
########################################################################

ifdef SVMP_AUTHORIZED_KEYS
    PRODUCT_PACKAGES += scp sftp sshd ssh-keygen start-ssh
    PRODUCT_COPY_FILES += $(SVMP_AUTHORIZED_KEYS):system/etc/ssh/authorized_keys
endif

########################################################################
# kernel
########################################################################

# No need to specify -j for the submake because the parent make process
# will communicate the information to the child
# If building a 64-bit kernel, remove ARCH=x86 from the make line
.PHONY: build_kernel kernel_config
kernel_config: $(SVMP_KERN_CONFIG)
	cp $(SVMP_KERN_CONFIG) $(KERNEL_SRC)/.config
build_kernel: kernel_config
	$(MAKE) ARCH=x86 bzImage -C $(KERNEL_SRC)
$(KERNEL_BIN): build_kernel

#$(call inherit-product, frameworks/native/build/phone-xhdpi-1024-dalvik-heap.mk)

########################################################################
# custom signing keys
########################################################################

ifdef SVMP_DEV_CERTIFICATE
    PRODUCT_DEFAULT_DEV_CERTIFICATE := $(SVMP_DEV_CERTIFICATE)
endif

PRODUCT_POLICY := android.policy_phone

MESA_GPU_DRIVERS := swrast llvmpipe

PRODUCT_VERSION := 2.0.0-pre

PRODUCT_NAME := svmp
PRODUCT_DEVICE := svmp
PRODUCT_BRAND := MITRE
PRODUCT_MODEL := Secure Virtual Mobile Platform $(PRODUCT_VERSION)
