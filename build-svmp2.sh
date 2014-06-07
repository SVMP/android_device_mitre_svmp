#!/bin/bash

NUM_PROCS=`grep -c ^processor /proc/cpuinfo`
NUM_PROCS=$(($NUM_PROCS + $NUM_PROCS / 2))

#CERT_INFO="/C=US/ST=VIRGINIA/L=McLean/O=MITRE Corporation/CN=SVMP/emailAddress=svmp@mitre.org"

echo "Building SVMP"

BUILD_TYPE="SVMP_BUILD_TYPE=$SVMP_BUILD_TYPE"
if [ -z $SVMP_BUILD_TYPE ] ; then
  read -p "Build VM to use virtio block devices? (e.g., for qemu-kvm): [y/N]" -n 1 -r
  if [[ $REPLY =~ ^[Yy]$ ]]
  then
    BUILD_TYPE='SVMP_BUILD_TYPE=virtio'
  fi
fi

echo

AUTHORIZED_KEYS=""
if [ -z $SVMP_AUTHORIZED_KEYS ] ; then
    echo "WARNING! SVMP_AUTHORIZED_KEYS is not set, not copying authorized_keys to VM image (SSH access will be unavailable)"
else
    echo "SVMP_AUTHORIZED_KEYS is set, copying $SVMP_AUTHORIZED_KEYS to authorized_keys in VM image"
    AUTHORIZED_KEYS="SVMP_AUTHORIZED_KEYS=$SVMP_AUTHORIZED_KEYS"
fi

function do_build () {
  echo
  echo "Starting build."
  echo
  . build/envsetup.sh
  lunch svmp-eng
  rm -f $OUT/root/fstab.svmp
  export INIT_BOOTCHART=true
  make svmp_system_disk svmp_data_disk \
    svmp_system_disk_vmdk  svmp_data_disk_vmdk \
    svmp_system_disk_vdi   svmp_data_disk_vdi \
    svmp_system_disk_qcow2 svmp_data_disk_qcow2 \
    -j$NUM_PROCS $BUILD_TYPE $AUTHORIZED_KEYS
}

# This function sets the SVMP_DEV_CERTIFICATE variable used in svmp.mk
# to set the PRODUCT_DEFAULT_DEV_CERTIFICATE
function check_custom_cert () {
  # Environment variables:
  #   SVMP_USE_CUSTOM_KEYS  (GET)
  #   SVMP_DEV_CERTIFICATE  (SET)

  if [ -z $SVMP_USE_CUSTOM_KEYS ]; then
    read -p "Use custom keys for SVMP build [y/N]? " -n 1 -r
    if [[ $REPLY =~ ^[Yy]$ ]]
    then
      SVMP_USE_CUSTOM_KEYS="yes"
    else
      SVMP_USE_CUSTOM_KEYS="no"
    fi
  fi
  echo
  echo "SVMP USE CUSTOM KEYS: $SVMP_USE_CUSTOM_KEYS"

  if [[ $SVMP_USE_CUSTOM_KEYS =~ ^[yY] ]]; then
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
    echo
    echo "To generate Enter Certificate info in the following format:"
    echo "  /C=US/ST=Virginia/L=Moseley/O=Test Corporation/CN=SVMP/emailAddress=svmp@test.com"
    echo

    read -p "Enter Certificate information: " SVMP_CERT_INFO

    # If empty information is entered then use default AOSP certificates
    if [ -z "$SVMP_CERT_INFO" ]; then
      echo "Empty certificate information entered. Using default AOSP certificates."
      export SVMP_DEV_CERTIFICATE=""
      return -1
    fi
  fi


  # Check if certificates already exist
  if [ -f $CERT_DIR/media.x509.pem ] && [ -f $CERT_DIR/platform.x509.pem ] && [ -f $CERT_DIR/shared.x509.pem ] &&  [ -f $CERT_DIR/relkey.x509.pem ]; then
    # If certificates already exist, validate to ensure they match SVMP_CERT_INFO variable
    CERT_VALID=`openssl x509 -noout -subject -issuer -in "$CERT_DIR"/relkey.x509.pem`
 
    CERT_VALID="${CERT_VALID#subject= }"
    CERT_VALID="${CERT_VALID%%issuer=*}"

    # Trim whitespace for compariosn
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
   
  local OLD_PATH=$PATH
  PATH="/usr/bin:/usr/local/bin:$PATH"
  ./development/tools/make_key $CERT_DIR/media "$SVMP_CERT_INFO" <<< ""
  ./development/tools/make_key $CERT_DIR/platform "$SVMP_CERT_INFO" <<< ""
  ./development/tools/make_key $CERT_DIR/shared "$SVMP_CERT_INFO" <<< ""
  ./development/tools/make_key $CERT_DIR/relkey "$SVMP_CERT_INFO" <<< "" # Must be named relkey or change svmp.mk to use something else
  PATH=$OLD_PATH

  export SVMP_DEV_CERTIFICATE="$CERT_DIR/relkey"

  #export PRODUCT_DEFAULT_DEV_CERTIFICATE="device/mitre/svmp/security/relkey"
}

check_custom_cert
do_build
