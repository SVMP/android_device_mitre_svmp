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
SVMP_VDI_AIO_DISK_IMAGE_TARGET := $(PRODUCT_OUT)/svmp_aio_disk.vdi

.PHONY: svmp_system_disk_vdi svmp_data_disk_vdi svmp_aio_disk_vdi
svmp_system_disk_vdi: $(SVMP_VDI_SYSTEM_DISK_IMAGE_TARGET)
svmp_data_disk_vdi: $(SVMP_VDI_DATA_DISK_IMAGE_TARGET)
svmp_aio_disk_vdi: $(SVMP_VDI_AIO_DISK_IMAGE_TARGET)

%.vdi: %.img
	@echo "Converting image to VDI format: $^"
	@rm -f $@
	$(hide) $(qemu-img) convert \
		-O vdi -f raw \
		$^ $@
	@echo "Done converting image: $@"

########################################################################
# VMDK conversion
########################################################################

SVMP_VMDK_SYSTEM_DISK_IMAGE_TARGET := $(PRODUCT_OUT)/svmp_system_disk.vmdk
SVMP_VMDK_DATA_DISK_IMAGE_TARGET := $(PRODUCT_OUT)/svmp_data_disk.vmdk
SVMP_VMDK_AIO_DISK_IMAGE_TARGET := $(PRODUCT_OUT)/svmp_aio_disk.vmdk

.PHONY: svmp_system_disk_vmdk svmp_data_disk_vmdk svmp_aio_disk_vmdk
svmp_system_disk_vmdk: $(SVMP_VMDK_SYSTEM_DISK_IMAGE_TARGET)
svmp_data_disk_vmdk: $(SVMP_VMDK_DATA_DISK_IMAGE_TARGET)
svmp_aio_disk_vmdk: $(SVMP_VMDK_AIO_DISK_IMAGE_TARGET)

%.vmdk: %.img
	@echo "Converting image to VMDK format: $^"
	@rm -f $@
	$(hide) $(qemu-img) convert \
		-O vmdk -f raw -o compat6 \
		$^ $@
	@echo "Done converting image: $@"

#%.vmdk: %.img
#	@echo "Converting image to VMDK format: $^"
#	@rm -f $@
#	$(hide) $(virtual_box_manager) convertfromraw \
#		--format VMDK --variant Standard \
#		$^ $@
#	@echo "Done converting image: $@"

########################################################################
# QCOW2 conversion
########################################################################

SVMP_QCOW2_SYSTEM_DISK_IMAGE_TARGET := $(PRODUCT_OUT)/svmp_system_disk.qcow2
SVMP_QCOW2_DATA_DISK_IMAGE_TARGET := $(PRODUCT_OUT)/svmp_data_disk.qcow2
SVMP_QCOW2_AIO_DISK_IMAGE_TARGET := $(PRODUCT_OUT)/svmp_aio_disk.qcow2

.PHONY: svmp_system_disk_qcow2 svmp_data_disk_qcow2 svmp_aio_disk_qcow2
svmp_system_disk_qcow2: $(SVMP_QCOW2_SYSTEM_DISK_IMAGE_TARGET)
svmp_data_disk_qcow2: $(SVMP_QCOW2_DATA_DISK_IMAGE_TARGET)
svmp_aio_disk_qcow2: $(SVMP_QCOW2_AIO_DISK_IMAGE_TARGET)

%.qcow2: %.img
	@echo "Converting image to QCOW2 format: $^"
	@rm -f $@
	$(hide) $(qemu-img) convert \
		-O qcow2 -f raw -o compat=0.10 -c \
		$^ $@
	@echo "Done converting image: $@"

########################################################################
# OVF - VirtualBox
########################################################################

SVMP_OVA_VIRTUALBOX_TARGET := $(PRODUCT_OUT)/svmp_vbox.ova

.PHONY: svmp_vbox_ova
svmp_vbox_ova: $(SVMP_OVA_VIRTUALBOX_TARGET)

VBOX_OVA_TMP_VMNAME := svmp-vbox-ova
VBOX_OVA_TMP_DIR := $(PRODUCT_OUT)/$(VBOX_OVA_TMP_VMNAME)

$(SVMP_OVA_VIRTUALBOX_TARGET): \
					$(SVMP_VMDK_SYSTEM_DISK_IMAGE_TARGET) \
					$(SVMP_VMDK_DATA_DISK_IMAGE_TARGET)
	@echo "Creating Virtualbox OVA appliance: $@"
	@echo "  Delete old temp VM if it still exists"
	@rm -f $@
	$(hide) $(virtual_box_manager) unregistervm $(VBOX_OVA_TMP_VMNAME) --delete 2>/dev/null || /bin/true
	@echo "  Creating temporary VM"
	$(hide) $(virtual_box_manager) createvm --register \
		--name $(VBOX_OVA_TMP_VMNAME)
	$(hide) $(virtual_box_manager) modifyvm \
		"$(VBOX_OVA_TMP_VMNAME)" \
		--memory 1024 --vram 16 --acpi on \
		--ioapic on --cpus 1 --ostype Linux_64 \
		--rtcuseutc on
	$(hide) $(virtual_box_manager) modifyvm \
		"$(VBOX_OVA_TMP_VMNAME)" \
		--nic1 bridged
	$(hide) $(virtual_box_manager) modifyvm \
		"$(VBOX_OVA_TMP_VMNAME)" \
		--nictype1 82540EM
	@echo "  Adding disks"
	$(hide) $(virtual_box_manager) storagectl \
		"$(VBOX_OVA_TMP_VMNAME)" \
		--name "SATA Controller" --add sata --controller IntelAHCI \
		--portcount 2 --hostiocache on --bootable on
	$(hide) $(virtual_box_manager) storageattach \
		"$(VBOX_OVA_TMP_VMNAME)" \
		--storagectl "SATA Controller" --port 0 --device 0 --type hdd \
		--medium $(SVMP_VMDK_SYSTEM_DISK_IMAGE_TARGET)
	$(hide) $(virtual_box_manager) storageattach \
		"$(VBOX_OVA_TMP_VMNAME)" \
		--storagectl "SATA Controller" --port 1 --device 0 --type hdd \
		--medium $(SVMP_VMDK_DATA_DISK_IMAGE_TARGET)
	@echo "  Exporting to OVF"
	$(hide) $(virtual_box_manager) export \
		"$(VBOX_OVA_TMP_VMNAME)" \
		-o $@ \
		--ovf10 --manifest \
		--vsys 0 \
		--product "Secure Virtual Mobile Platform" \
		--producturl "https://svmp.github.io" \
		--version "$(PRODUCT_VERSION)" \
		--description "$(PRODUCT_DESCRIPTION)" \
		--eulafile $(TARGET_DEVICE_DIR)/NOTICE
	# clean up the temp VM
	@echo "  Deleting the temp VM"
	$(hide) $(virtual_box_manager) storageattach \
		"$(VBOX_OVA_TMP_VMNAME)" \
		--storagectl "SATA Controller" --port 0 --device 0 --type hdd \
		--medium none
	$(hide) $(virtual_box_manager) storageattach \
		"$(VBOX_OVA_TMP_VMNAME)" \
		--storagectl "SATA Controller" --port 1 --device 0 --type hdd \
		--medium none
	$(hide) $(virtual_box_manager) closemedium disk $(SVMP_VMDK_SYSTEM_DISK_IMAGE_TARGET) 
	$(hide) $(virtual_box_manager) closemedium disk $(SVMP_VMDK_DATA_DISK_IMAGE_TARGET) 
	$(hide) $(virtual_box_manager) unregistervm $(VBOX_OVA_TMP_VMNAME) --delete
	@rm -rf $(VBOX_OVA_TMP_DIR)
	@echo "Done creating OVA: $@"


########################################################################
# OVF - VMware
########################################################################

SVMP_OVA_VMWARE_TARGET := $(PRODUCT_OUT)/svmp_vmware.ova
SVMP_OVF_VMWARE_TARGET := $(PRODUCT_OUT)/svmp_vmware_ovf/svmp_vmware.ovf

.PHONY: svmp_vmware_ova
svmp_vmware_ova: $(SVMP_OVA_VMWARE_TARGET)

VMWARE_OVA_TMP_VMNAME := svmp-vmware-ova
VMWARE_OVA_TMP_DIR := $(PRODUCT_OUT)/$(VMWARE_OVA_TMP_VMNAME)

VMWARE_OVF_SED_REPLACE := </Name><ovf:EulaSection>
VMWARE_OVF_SED_WITH := </Name><ProductSection><Info>Meta-information about the installed software</Info><Product>Secure Virtual Mobile Platform</Product><Version>$(PRODUCT_VERSION)</Version><ProductUrl>https://svmp\.github\.io</ProductUrl></ProductSection><ovf:EulaSection>

$(SVMP_OVF_VMWARE_TARGET): \
					$(SVMP_VMDK_SYSTEM_DISK_IMAGE_TARGET) \
					$(SVMP_VMDK_DATA_DISK_IMAGE_TARGET)
	@echo "Creating VMware OVF appliance: $@"
	@mkdir -p `dirname $@`
	@cp $(TARGET_DEVICE_DIR)/image_build/svmp_vmware.vmx $(PRODUCT_OUT)/svmp_vmware.vmx
	@echo ovftool --overwrite --eula@=$(TARGET_DEVICE_DIR)/NOTICE $(PRODUCT_OUT)/svmp_vmware.vmx $@
	$(hide) ovftool --overwrite --eula@=$(TARGET_DEVICE_DIR)/NOTICE $(PRODUCT_OUT)/svmp_vmware.vmx $@
	@echo "Replace this: \"$(VMWARE_OVF_SED_REPLACE)\""
	@echo "With this: \"$(VMWARE_OVF_SED_WITH)\""
	@echo sed -i'.orig' 's|$(VMWARE_OVF_SED_REPLACE)|$(VMWARE_OVF_SED_WITH)|' $@
	@sed -i'.orig' 's|$(VMWARE_OVF_SED_REPLACE)|$(VMWARE_OVF_SED_WITH)|' $@
	@echo "Done creating OVF: $@" 

$(SVMP_OVA_VMWARE_TARGET): $(SVMP_OVF_VMWARE_TARGET)
	@echo "Creating VMware OVA appliance: $@"
	@echo ovftool --skipManifestCheck --overwrite --name="svmp-vmware-ova" $^ $@
	$(hide) ovftool --skipManifestCheck --overwrite --name="svmp-vmware-ova" $^ $@
	@echo "Done creating OVA: $@"
