#
# IA target for MOCSI
#

TARGET_ARCH=x86
DISABLE_DEXPREOPT := true
TARGET_COMPRESS_MODULE_SYMBOLS := false
TARGET_NO_RECOVERY := true
TARGET_HARDWARE_3D := true
BOARD_USES_GENERIC_AUDIO := false
USE_CAMERA_STUB := true
TARGET_PROVIDES_INIT_RC := true
TARGET_CPU_ABI := x86
TARGET_USERIMAGES_USE_EXT4 := true
TARGET_BOOTIMAGE_USE_EXT2 := true

# For VirtualBox and likely other emulators
BOARD_INSTALLER_CMDLINE := init=/init console=ttyS0 console=tty0 vga=788 verbose
#BOARD_KERNEL_CMDLINE := init=/init qemu=1 console=ttyS0 console=tty0 vga=788 verbose androidboot.hardware=svmp androidboot.console=ttyS0 android.qemud=ttyS1
#BOARD_KERNEL_CMDLINE := init=/init console=ttyS0 vga=normal verbose androidboot.hardware=svmp androidboot.console=ttyS0 video=vfb: vfb_enable=1 tsc=reliable qemu=1
BOARD_KERNEL_CMDLINE := init=/init console=ttyS0 verbose androidboot.hardware=svmp androidboot.console=ttyS0 video=vfb: vfb_enable=1 tsc=reliable qemu=1
TARGET_USE_DISKINSTALLER := true

TARGET_DISK_LAYOUT_CONFIG := build/target/board/vbox_x86/disk_layout.conf

BOARD_BOOTIMAGE_MAX_SIZE := 8388608
BOARD_SYSLOADER_MAX_SIZE := 7340032
BOARD_FLASH_BLOCK_SIZE := 512
# 50M
BOARD_USERDATAIMAGE_PARTITION_SIZE :=104857600
# 500M
BOARD_INSTALLERIMAGE_PARTITION_SIZE := 524288000
TARGET_USERIMAGES_SPARSE_EXT_DISABLED := true
# Reserve 265M  for the system partition
BOARD_SYSTEMIMAGE_PARTITION_SIZE := 268435456


# The eth0 device should be started with dhcp on boot.
# Useful for emulators that don't provide a wifi connection.
NET_ETH0_STARTONBOOT := true

ADDITIONAL_BUILD_PROPERTIES += dalvik.vm.heapsize=96m

# Build OpenGLES emulation host and guest libraries
BUILD_EMULATOR_OPENGL := true

# Build and enable the OpenGL ES View renderer. When running on the emulator,
# the GLES renderer disables itself if host GL acceleration isn't available.
USE_OPENGL_RENDERER := true

# Use software rasterizer from Mesa
BOARD_GPU_DRIVERS := swrast
