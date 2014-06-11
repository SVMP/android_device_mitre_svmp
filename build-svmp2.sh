#!/bin/bash

# List of environment variables:
#    SVMP_BUILD_TYPE
#    SVMP_DISK_TYPE
#    SVMP_AIO_BUILD
#    SVMP_SSH_AUTHORIZED_KEYS 
#    SVMP_USE_CUSTOM_KEYS
#    SVMP_CERT_INFO

NUM_PROCS=`grep -c ^processor /proc/cpuinfo`
NUM_PROCS=$(($NUM_PROCS + $NUM_PROCS / 2))

WT_HEIGHT=20
WT_WIDTH=72

CONFIG_FILE=`pwd`/.svmp.config
SHOW_SAVE_DIALOG=no

if [ -r $CONFIG_FILE ] ; then
    source $CONFIG_FILE
fi

########################################################################
# Intro
########################################################################

_MESSAGE="Welcome to the SVMP build script.

You will be asked to choose several configuration options before the build begins.

Press 'Cancel' at any time to quit the build."

if [ ! -r $CONFIG_FILE ] ; then
    whiptail --yesno "$_MESSAGE" $WT_HEIGHT $WT_WIDTH \
         --title "Welcome" \
         --yes-button "Continue" \
         --no-button "Cancel" || exit
fi

########################################################################
# What type of disk image, container, or VM appliance to build?
########################################################################

function not-yet-implemented() {
    whiptail --msgbox --title "Error" \
             "Sorry, the $1 target is not yet implemented." 8 $WT_WIDTH
    exit
}

if [ -z $SVMP_BUILD_TYPE ] ; then
    SVMP_BUILD_TYPE=$(whiptail --radiolist --title "Images to Build" \
        "Choose the image target to build:" $WT_HEIGHT $WT_WIDTH 10 \
        raw        "  Raw disk image with no container format" on \
        vdi        "  Raw disk image in VDI format" off \
        vmdk       "  Raw disk image in VMDK format" off \
        qcow2      "  Raw disk image in QCOW2 format" off \
        ovf-vbox   "  OVF appliance for VirtualBox (TODO)" off \
        ovf-vmware "  OVF appliance for VMware (TODO)" off \
        ovf-xen    "  OVF appliance for XenServer (TODO)" off \
        kvm        "  QEMU-KVM libvirt appliance (TODO)" off \
        aws-ami    "  Amazon Web Services AMI (TODO)" off 3>&1 1>&2 2>&3 ) || exit
    SHOW_SAVE_DIALOG=yes
fi

# Some of these will dictate a particular disk controller type
case $SVMP_BUILD_TYPE in
ovf-vbox)
  SVMP_DISK_TYPE=sdx
  not-yet-implemented "VirtualBox OVF"
  ;;
ovf-vmware)
  SVMP_DISK_TYPE=sdx
  not-yet-implemented "VMware OVF"
  ;;
ovf-xen)
  SVMP_DISK_TYPE=xdx
  not-yet-implemented "XenServer OVF"
  ;;
kvm)
  SVMP_DISK_TYPE=vdx
  not-yet-implemented "KVM libvirt"
  ;;
aws-ami)
  SVMP_DISK_TYPE=xdx
  not-yet-implemented "AWS"
  ;;
*)
  # One of the raw types. We'll need to ask for a disk type.
  SVMP_DISK_TYPE=${SVMP_DISK_TYPE:-}
  ;;
esac

########################################################################
# What disk controller type should fstab expect?
########################################################################

if [ -z $SVMP_DISK_TYPE ] ; then
    SVMP_DISK_TYPE=$(whiptail --menu --title "Drive Type" \
        "Choose the virtual hard drive type to use to configure fstab:" $WT_HEIGHT $WT_WIDTH 4 \
        sdx "    /dev/sdX - SATA / SCSI" \
        hdx "    /dev/hdX - IDE" \
        vdx "    /dev/vdX - QEMU / KVM Virtio" \
        xdx "    /dev/xdX - Xen (also AWS & Rackspace)" 3>&1 1>&2 2>&3 ) || exit
    SHOW_SAVE_DIALOG=yes
fi

########################################################################
# Make an all-in-one image or separate system and data disks?
########################################################################

_MESSAGE="Would you like to build an \"all-in-one\" image?

By default, separate system and data disks are created.

Optionally, an all-in-one image can be made combining /system and /data into a single disk."

if [ -z $SVMP_AIO_BUILD ] ; then
    if whiptail --yesno --title "All-in-one image" --defaultno \
                --yes-button "All-in-one" --no-button "Separate" \
                "$_MESSAGE" $WT_HEIGHT $WT_WIDTH
    then
        SVMP_AIO_BUILD=yes
    else
        SVMP_AIO_BUILD=no
    fi
    SHOW_SAVE_DIALOG=yes
fi

########################################################################
# Add SSH keys?
########################################################################

if [ -z $SVMP_SSH_AUTHORIZED_KEYS ] ; then
    if whiptail --yesno --title "SSH" --defaultno \
                "Enable sshd within the Android VM?" 8 $WT_WIDTH
    then
        # ask for the authorized keys file path
        SVMP_SSH_AUTHORIZED_KEYS=$(whiptail --inputbox --title "SSH" \
            "Enter the FULL path to the SSH authorized keys file to embed:" \
            8 $WT_WIDTH 3>&1 1>&2 2>&3 )
        if [ ! -r $SVMP_SSH_AUTHORIZED_KEYS ] ; then
            echo "ERROR! Cannot read the authorized_keys file \"$SVMP_SSH_AUTHORIZED_KEYS\""
            exit
        fi
    fi
    SHOW_SAVE_DIALOG=yes
fi


########################################################################
# Use custom signing keys for the ROM?
########################################################################

if [ -z $SVMP_USE_CUSTOM_KEYS ] ; then
    if whiptail --yesno --title "Signing Keys" --defaultno \
                "Use custom keys to sign the SVMP ROM image?" 8 $WT_WIDTH
    then
        SVMP_USE_CUSTOM_KEYS=yes
    else
        SVMP_USE_CUSTOM_KEYS=no
    fi
    SHOW_SAVE_DIALOG=yes
fi

_MESSAGE="Enter the certificate info string:

Example:
\"/C=US/ST=State/L=City/O=Corp Name/CN=SVMP/emailAddress=svmp@test.com\""

if [ "$SVMP_USE_CUSTOM_KEYS" == "yes" ] ; then
    # get the certificate info string for generation
    if [ -z "$SVMP_CERT_INFO" ] ; then
     
        # ask for the authorized keys file path
        SVMP_CERT_INFO=$(whiptail --inputbox --title "Signing Keys" \
            "$_MESSAGE" 12 80 3>&1 1>&2 2>&3 )
        SHOW_SAVE_DIALOG=yes
    fi
fi

########################################################################
# Offer to save the settings to environment variables for next time
# (and/or a config file)
########################################################################

_MESSAGE="Would you like to save these settings for later?

The following settings will be saved to $CONFIG_FILE:
    SVMP_BUILD_TYPE = \"$SVMP_BUILD_TYPE\"
    SVMP_DISK_TYPE = \"$SVMP_DISK_TYPE\"
    SVMP_AIO_BUILD = \"$SVMP_AIO_BUILD\"
    SVMP_SSH_AUTHORIZED_KEYS = \"$SVMP_SSH_AUTHORIZED_KEYS\"
    SVMP_USE_CUSTOM_KEYS = \"$SVMP_USE_CUSTOM_KEYS\"
    SVMP_CERT_INFO = \"$SVMP_CERT_INFO\"

Delete the config file to re-enable the configuration menus."

if [ $SHOW_SAVE_DIALOG == "yes" ] ; then
    if whiptail --yesno --title "Save Settings" --defaultno \
             "$_MESSAGE" 20 $WT_WIDTH
    then
        echo "SVMP_BUILD_TYPE=\"$SVMP_BUILD_TYPE\"" > $CONFIG_FILE
        echo "SVMP_DISK_TYPE=\"$SVMP_DISK_TYPE\"" >> $CONFIG_FILE
        echo "SVMP_AIO_BUILD=\"$SVMP_AIO_BUILD\"" >> $CONFIG_FILE
        echo "SVMP_SSH_AUTHORIZED_KEYS=\"$SVMP_SSH_AUTHORIZED_KEYS\"" >> $CONFIG_FILE
        echo "SVMP_USE_CUSTOM_KEYS=\"$SVMP_USE_CUSTOM_KEYS\"" >> $CONFIG_FILE
        echo "SVMP_CERT_INFO=\"$SVMP_CERT_INFO\"" >> $CONFIG_FILE
    fi
fi

########################################################################
# Done with user input. Starting the build.
########################################################################

_MESSAGE="Starting the build."

if [ ! -r $CONFIG_FILE ] ; then
    whiptail --yesno "$_MESSAGE" $WT_HEIGHT $WT_WIDTH \
         --title "Building" \
         --yes-button "Continue" \
         --no-button "Cancel" || exit
fi

########################################################################
# Done with user input.
########################################################################

function do_build () {
  echo
  echo "Starting build."
  echo

  if [ "$SVMP_AIO_BUILD" == "yes" ] ; then
      MAKE_SYSTEM_TARGET="svmp_aio_disk"
      MAKE_DATA_TARGET=
  else
      MAKE_SYSTEM_TARGET="svmp_system_disk"
      MAKE_DATA_TARGET="svmp_data_disk"
  fi

  case $SVMP_BUILD_TYPE in
  raw)
      TGT_SUFFIX=""
      ;;
  vdi)
      TGT_SUFFIX="_vdi"
      ;;
  vmdk)
      TGT_SUFFIX="_vmdk"
      ;;
  qcow2)
      TGT_SUFFIX="_qcow2"
      ;;
  ovf-vbox)
      TGT_SUFFIX="_vmdk"
      ;;
  ovf-vmware)
      TGT_SUFFIX="_vmdk"
      ;;
  ovf-xen)
      TGT_SUFFIX="_vmdk"
      ;;
  kvm)
      TGT_SUFFIX="_qcow2"
      ;;
  aws-ami)
      TGT_SUFFIX=""
      ;;
  esac

  MAKE_SYSTEM_TARGET="${MAKE_SYSTEM_TARGET}${TGT_SUFFIX}"
  if [ $MAKE_DATA_TARGET ] ; then
      MAKE_DATA_TARGET="${MAKE_DATA_TARGET}${TGT_SUFFIX}"
  fi

  MAKE_OPTS="SVMP_DISK_TYPE=$SVMP_DISK_TYPE"
  if [ "$SVMP_AIO_BUILD" == "yes" ] ; then
      MAKE_OPTS="$MAKE_OPTS SVMP_AIO_BUILD=$SVMP_AIO_BUILD"
  fi
  if [ -r $SVMP_SSH_AUTHORIZED_KEYS ] ; then
      MAKE_OPTS="$MAKE_OPTS SVMP_AUTHORIZED_KEYS=$SVMP_SSH_AUTHORIZED_KEYS"
  fi
  if [ $SVMP_DEV_CERTIFICATE ] ; then
      MAKE_OPTS="$MAKE_OPTS SVMP_DEV_CERTIFICATE=\"$SVMP_DEV_CERTIFICATE\""
  fi

  . build/envsetup.sh
  lunch svmp-eng
  rm -f $OUT/root/fstab.svmp
  export INIT_BOOTCHART=true
  echo make $MAKE_SYSTEM_TARGET $MAKE_DATA_TARGET $MAKE_OPTS -j$NUM_PROCS
  make $MAKE_SYSTEM_TARGET $MAKE_DATA_TARGET $MAKE_OPTS -j$NUM_PROCS
}

# This function sets the SVMP_DEV_CERTIFICATE variable used in svmp.mk
# to set the PRODUCT_DEFAULT_DEV_CERTIFICATE
function check_custom_cert () {
  # Environment variables:
  #   SVMP_USE_CUSTOM_KEYS  (GET)

  if [ "$SVMP_USE_CUSTOM_KEYS" == "yes" ]; then
    generate_custom_certs
  else
    export SVMP_DEV_CERTIFICATE=""
  fi

}

function generate_custom_certs () {
  # Environment variables:
  #   SVMP_CERT_INFO  (GET & SET)
  #   SVMP_DEV_CERTIFICATE (SET)

  CERT_DIR="device/mitre/svmp/security"

  # Check if CERT_INFO variable is set
  if [ -z "$SVMP_CERT_INFO" ]; then
      # If empty information is entered then use default AOSP certificates
      echo "Empty certificate information entered. Using default AOSP certificates."
      export SVMP_DEV_CERTIFICATE=""
      return -1
  fi


  # Check if certificates already exist
  if [ -f $CERT_DIR/media.x509.pem ] && [ -f $CERT_DIR/platform.x509.pem ] && [ -f $CERT_DIR/shared.x509.pem ] &&  [ -f $CERT_DIR/relkey.x509.pem ]; then
    # If certificates already exist, validate to ensure they match SVMP_CERT_INFO variable
    CERT_VALID=`openssl x509 -noout -subject -issuer -in "$CERT_DIR"/relkey.x509.pem`
 
    CERT_VALID="${CERT_VALID#subject= }"
    CERT_VALID="${CERT_VALID%%issuer=*}"

    # Trim whitespace for comparison
    CERT_VALID="${CERT_VALID##*( )}" # Trim leading whitespace
    CERT_VALID="${CERT_VALID%%*( )}" # Trim trailing wihitespace
    CERT_VALID=$(echo -n "$CERT_VALID" )

    SVMP_CERT_INFO="${SVMP_CERT_INFO##*( )}"
    SVMP_CERT_INFO="${SVMP_CERT_INFO%%*( )}"
    SVMP_CERT_INFO=$(echo -n "$SVMP_CERT_INFO" )

    echo "SVMP CERT INFO: $SVMP_CERT_INFO "
    #echo "CERT INFO:=$CERT_VALID="
    
    if [ -z "${CERT_VALID#*$SVMP_CERT_INFO}" ]; then
      echo
      echo "Existing relkey cert seems valid and matches SVMP_CERT_INFO."
      echo
      export SVMP_DEV_CERTIFICATE="$CERT_DIR/relkey"
      return 0
    else
      echo
      echo "Existing relkey cert does not match SVMP_CERT_INFO. Removing existing certs and regenerating..."
      echo
      rm $CERT_DIR/*.pk8
      rm $CERT_DIR/*.pem
    fi
  fi
  
  if [ ! -d device/mitre/svmp/security ]; then
    mkdir -p device/mitre/svmp/security
  fi

  echo
  echo "Generating keys for build."
  echo 
   
  if [[ `which openssl` == *out/host/*/bin/openssl ]] ; then
    echo "ERROR. PATH points to the Android build openssl instead of the system openssl."
    echo "       Re-execute $0 in a shell environment that has not been modified by build/envsetup.sh and lunch."
    exit -1
  fi

  ./development/tools/make_key $CERT_DIR/media "$SVMP_CERT_INFO" <<< ""
  ./development/tools/make_key $CERT_DIR/platform "$SVMP_CERT_INFO" <<< ""
  ./development/tools/make_key $CERT_DIR/shared "$SVMP_CERT_INFO" <<< ""
  ./development/tools/make_key $CERT_DIR/relkey "$SVMP_CERT_INFO" <<< "" # Must be named relkey or change svmp.mk to use something else

  export SVMP_DEV_CERTIFICATE="$CERT_DIR/relkey"

  #export PRODUCT_DEFAULT_DEV_CERTIFICATE="device/mitre/svmp/security/relkey"
}

check_custom_cert
do_build
