# MOCSI Makefile

ifdef NET_ETH0_STARTONBOOT
  PRODUCT_PROPERTY_OVERRIDES += net.eth0.startonboot=1
endif


DEVICE_PACKAGE_OVERLAYS := device/mitre/svmp/overlay

######### FULL_BASE.mk ##############

#    VideoEditor

PRODUCT_PACKAGES := \
    drmserver \
    libdrmframework \
    libdrmframework_jni \
    libfwdlockengine \
    OpenWnn \
    PinyinIME \
    libWnnEngDic \
    libWnnJpnDic \
    libwnndict \
    WAPPushManager

# Additional settings used in all AOSP builds
PRODUCT_PROPERTY_OVERRIDES := \
    ro.com.android.dateformat=MM-dd-yyyy \
    ro.config.ringtone=Ring_Synth_04.ogg \
    ro.config.notification_sound=pixiedust.ogg

# Put en_US first in the list, so make it default.
PRODUCT_LOCALES := en_US

# Get some sounds
$(call inherit-product-if-exists, frameworks/base/data/sounds/AllAudio.mk)

# Get the TTS language packs
$(call inherit-product-if-exists, external/svox/pico/lang/all_pico_languages.mk)

# Get a list of languages.
$(call inherit-product, $(SRC_TARGET_DIR)/product/locales_full.mk)


##### GENERIC_NO_TELEPHONY.mk ########

PRODUCT_POLICY := android.policy_phone


PRODUCT_PACKAGES := \
    Gallery2 \
    Launcher2 \
    Calendar \
    CertInstaller \
    DrmProvider \
    Email \
    Exchange \
    LatinIME \
    Music \
    MusicFX \
    Provision \
    Settings \
    Sync \
    SystemUI \
    Updater \
    CalendarProvider \
    SyncProvider \
    bluetooth-health \
    hostapd \
    test-sensorservice \
    wpa_supplicant.conf

PRODUCT_PACKAGES += \
    icu.dat

PRODUCT_PACKAGES += \
    librs_jni \
    libvideoeditor_jni \
    libvideoeditorplayer \
    libvideoeditor_core


PRODUCT_PACKAGES += \
    audio_policy.default

PRODUCT_COPY_FILES := \
        system/bluetooth/data/audio.conf:system/etc/bluetooth/audio.conf \
        system/bluetooth/data/auto_pairing.conf:system/etc/bluetooth/auto_pairing.conf \
        system/bluetooth/data/blacklist.conf:system/etc/bluetooth/blacklist.conf \
        system/bluetooth/data/input.conf:system/etc/bluetooth/input.conf \
        system/bluetooth/data/network.conf:system/etc/bluetooth/network.conf \
        frameworks/base/media/libeffects/data/audio_effects.conf:system/etc/audio_effects.conf \
	out/target/product/svmp/utilities/busybox:root/sbin/mkfs.ext2

$(call inherit-product-if-exists, frameworks/base/data/fonts/fonts.mk)
$(call inherit-product-if-exists, external/lohit-fonts/fonts.mk)
$(call inherit-product-if-exists, frameworks/base/data/keyboards/keyboards.mk)
$(call inherit-product, $(SRC_TARGET_DIR)/product/core.mk)

# Overrides
PRODUCT_BRAND := generic
PRODUCT_DEVICE := generic
PRODUCT_NAME := generic_no_telephony

############## MOCSI ###########

LOCAL_PATH := $(call my-dir)

LOCAL_KERNEL := device/mitre/svmp/kernel

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
                location_helper

#		rtspserv \
#		fbstream \
#		fbstream_libs \

PRODUCT_COPY_FILES += \
    development/data/etc/apns-conf.xml:system/etc/apns-conf.xml \
    development/tools/emulator/system/camera/media_profiles.xml:system/etc/media_profiles.xml \
    device/mitre/svmp/vold.fstab:system/etc/vold.fstab \
    device/mitre/svmp/init.rc:root/init.rc \
    device/mitre/svmp/init.svmp.rc:root/init.svmp.rc \
    device/mitre/svmp/fixaudio.sh:system/bin/fixaudio.sh \
    device/mitre/svmp/excluded-input-devices.xml:system/etc/excluded-input-devices.xml \
    frameworks/base/data/etc/android.hardware.touchscreen.multitouch.jazzhand.xml:system/etc/permissions/android.hardware.touchscreen.multitouch.jazzhand.xml \
    external/svmp/fbstream/trunk/out/Release/fbstream_webrtc:system/bin/fbstream_webrtc \
   $(LOCAL_KERNEL):kernel

external/svmp/fbstream/trunk/out/Release/fbstream_webrtc :
	external/svmp/fbstream/trunk/build.sh

PRODUCT_POLICY := android.policy_phone

PRODUCT_NAME := svmp
PRODUCT_DEVICE := svmp
PRODUCT_BRAND := MITRE
PRODUCT_MODEL := MOCSI SVMP Thin Client VM
