# Separate disk images for system and user data
SVMP_SYSTEM_DISK_IMAGE_TARGET := $(PRODUCT_OUT)/svmp_system_disk.img
SVMP_DATA_DISK_IMAGE_TARGET := $(PRODUCT_OUT)/svmp_data_disk.img

# All-in-one disk image combining all the system and data partitions
# in a single image
SVMP_AIO_DISK_IMAGE_TARGET := $(PRODUCT_OUT)/svmp_aio_disk.img

svmp_system_disk_layout := $(TARGET_DEVICE_DIR)/image_build/svmp_system_image_layout.conf
svmp_data_disk_layout := $(TARGET_DEVICE_DIR)/image_build/svmp_data_image_layout.conf
svmp_aio_disk_layout := $(TARGET_DEVICE_DIR)/image_build/svmp_aio_image_layout.conf

.PHONY: svmp_system_disk svmp_data_disk svmp_aio_disk_layout

svmp_system_disk: $(SVMP_SYSTEM_DISK_IMAGE_TARGET)
svmp_data_disk: $(SVMP_DATA_DISK_IMAGE_TARGET)
svmp_aio_disk: $(SVMP_AIO_DISK_IMAGE_TARGET)

# The qemu-img binary. Needed for converting between disk image types.
qemu-img := qemu-img

$(SVMP_SYSTEM_DISK_IMAGE_TARGET): \
					$(INSTALLED_SYSTEMIMAGE) \
					$(INSTALLED_BOOTIMAGE_TARGET) \
					$(INSTALLED_CACHEIMAGE_TARGET) \
					$(grub_bin) \
					$(edit_mbr) \
					$(svmp_system_disk_layout)
	@echo "Creating bootable android system-disk image: $@"
	@rm -f $@
	$(hide) cat $(grub_bin) > $@
	@echo $(edit_mbr) -l $(svmp_system_disk_layout) -i $@ \
		inst_boot=$(INSTALLED_BOOTIMAGE_TARGET) \
		inst_system=$(INSTALLED_SYSTEMIMAGE) \
                inst_cache=$(INSTALLED_CACHEIMAGE_TARGET) ;
	$(hide) $(edit_mbr) -l $(svmp_system_disk_layout) -i $@ \
		inst_boot=$(INSTALLED_BOOTIMAGE_TARGET) \
		inst_system=$(INSTALLED_SYSTEMIMAGE)  \
                inst_cache=$(INSTALLED_CACHEIMAGE_TARGET) ;
	@echo "Done with bootable android system-disk image -[ $@ ]-"

$(SVMP_DATA_DISK_IMAGE_TARGET): \
					$(INSTALLED_USERDATAIMAGE_TARGET) \
					$(INSTALLED_CACHEIMAGE_TARGET) \
					$(grub_bin) \
					$(edit_mbr) \
					$(svmp_data_disk_layout)
	@echo "Creating SVMP data-disk image: $@"
	@rm -f $@
	$(hide) cat $(grub_bin) > $@
	@echo $(edit_mbr) -l $(svmp_data_disk_layout) -i $@ \
		inst_data=$(INSTALLED_USERDATAIMAGE_TARGET) \
		inst_cache=$(INSTALLED_CACHEIMAGE_TARGET) ;
	$(hide) $(edit_mbr) -l $(svmp_data_disk_layout) -i $@ \
		inst_data=$(INSTALLED_USERDATAIMAGE_TARGET) \
		inst_cache=$(INSTALLED_CACHEIMAGE_TARGET) ;
	@echo "Done with SVMP data-disk image -[ $@ ]-"

$(SVMP_AIO_DISK_IMAGE_TARGET): \
					$(INSTALLED_SYSTEMIMAGE) \
					$(INSTALLED_BOOTIMAGE_TARGET) \
					$(INSTALLED_CACHEIMAGE_TARGET) \
					$(INSTALLED_USERDATAIMAGE_TARGET) \
					$(grub_bin) \
					$(edit_mbr) \
					$(svmp_aio_disk_layout)
	@echo "Creating bootable SVMP all-in-one disk image: $@"
	@rm -f $@
	$(hide) cat $(grub_bin) > $@
	@echo $(edit_mbr) -l $(svmp_aio_disk_layout) -i $@ \
		inst_boot=$(INSTALLED_BOOTIMAGE_TARGET) \
		inst_system=$(INSTALLED_SYSTEMIMAGE) \
                inst_cache=$(INSTALLED_CACHEIMAGE_TARGET) \
                inst_data=$(INSTALLED_USERDATAIMAGE_TARGET) ;
	$(hide) $(edit_mbr) -l $(svmp_aio_disk_layout) -i $@ \
		inst_boot=$(INSTALLED_BOOTIMAGE_TARGET) \
		inst_system=$(INSTALLED_SYSTEMIMAGE)  \
                inst_cache=$(INSTALLED_CACHEIMAGE_TARGET) \
                inst_data=$(INSTALLED_USERDATAIMAGE_TARGET) ;
	@echo "Done with bootable SVMP all-in-one disk image -[ $@ ]-"

########################################################################
# VDI conversion
########################################################################

SVMP_VDI_SYSTEM_DISK_IMAGE_TARGET := $(PRODUCT_OUT)/svmp_system_disk.vdi
SVMP_VDI_DATA_DISK_IMAGE_TARGET := $(PRODUCT_OUT)/svmp_data_disk.vdi

.PHONY: svmp_system_disk_vdi svmp_data_disk_vdi
svmp_system_disk_vdi: $(SVMP_VDI_SYSTEM_DISK_IMAGE_TARGET)
svmp_data_disk_vdi: $(SVMP_VDI_DATA_DISK_IMAGE_TARGET)

$(SVMP_VDI_SYSTEM_DISK_IMAGE_TARGET): $(SVMP_SYSTEM_DISK_IMAGE_TARGET)
	@rm -f $@
	$(hide) $(qemu-img) convert \
		-O vdi -f raw \
		$^ $@

$(SVMP_VDI_DATA_DISK_IMAGE_TARGET): $(SVMP_DATA_DISK_IMAGE_TARGET)
	@rm -f $@
	$(hide) $(qemu-img) convert \
		-O vdi -f raw \
		$^ $@

########################################################################
# VMDK conversion
########################################################################

SVMP_VMDK_SYSTEM_DISK_IMAGE_TARGET := $(PRODUCT_OUT)/svmp_system_disk.vmdk
SVMP_VMDK_DATA_DISK_IMAGE_TARGET := $(PRODUCT_OUT)/svmp_data_disk.vmdk

.PHONY: svmp_system_disk_vmdk svmp_data_disk_vmdk
svmp_system_disk_vmdk: $(SVMP_VMDK_SYSTEM_DISK_IMAGE_TARGET)
svmp_data_disk_vmdk: $(SVMP_VMDK_DATA_DISK_IMAGE_TARGET)

$(SVMP_VMDK_SYSTEM_DISK_IMAGE_TARGET): $(SVMP_SYSTEM_DISK_IMAGE_TARGET)
	@rm -f $@
	$(hide) $(qemu-img) convert \
		-O vmdk -f raw \
		$^ $@

$(SVMP_VMDK_DATA_DISK_IMAGE_TARGET): $(SVMP_DATA_DISK_IMAGE_TARGET)
	@rm -f $@
	$(hide) $(qemu-img) convert \
		-O vmdk -f raw \
		$^ $@

########################################################################
# QCOW2 conversion
########################################################################

SVMP_QCOW2_SYSTEM_DISK_IMAGE_TARGET := $(PRODUCT_OUT)/svmp_system_disk.qcow2
SVMP_QCOW2_DATA_DISK_IMAGE_TARGET := $(PRODUCT_OUT)/svmp_data_disk.qcow2

.PHONY: svmp_system_disk_qcow2 svmp_data_disk_qcow2
svmp_system_disk_qcow2: $(SVMP_QCOW2_SYSTEM_DISK_IMAGE_TARGET)
svmp_data_disk_qcow2: $(SVMP_QCOW2_DATA_DISK_IMAGE_TARGET)

$(SVMP_QCOW2_SYSTEM_DISK_IMAGE_TARGET): $(SVMP_SYSTEM_DISK_IMAGE_TARGET)
	@rm -f $@
	$(hide) $(qemu-img) convert \
		-O qcow2 -f raw -o compat=0.10 \
		$^ $@

$(SVMP_QCOW2_DATA_DISK_IMAGE_TARGET): $(SVMP_DATA_DISK_IMAGE_TARGET)
	@rm -f $@
	$(hide) $(qemu-img) convert \
		-O qcow2 -f raw -o compat=0.10 \
		$^ $@

