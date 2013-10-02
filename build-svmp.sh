#!/bin/bash

NUM_PROCS=`grep -c ^processor /proc/cpuinfo`
NUM_PROCS=$(($NUM_PROCS + $NUM_PROCS / 2))

echo "Building SVMP"

BUILD_TYPE="SVMP_BUILD_TYPE=$SVMP_BUILD_TYPE"
if [ -z $SVMP_BUILD_TYPE ] ; then
  read -p "Build VM to use virtio block devices? (e.g., for qemu-kvm): [y/N]" -n 1 -r
  if [[ $REPLY =~ ^[Yy]$ ]]
  then
    BUILD_TYPE='SVMP_BUILD_TYPE=virtio'
  fi
fi

function do_build () {
  echo "Ok. Starting build."
  echo
  . build/envsetup.sh
  lunch svmp-eng
  rm $OUT/root/fstab.svmp
  export INIT_BOOTCHART=true
  m android_system_disk_vdi android_system_disk_vmdk android_data_disk_vdi android_data_disk_vmdk -j$NUM_PROCS $BUILD_TYPE
}

do_build
