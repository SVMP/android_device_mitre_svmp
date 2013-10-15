#!/bin/sh

# script gets run with AOSP root as its working dir, so need full path
#OUT_FILE=device/mitre/svmp/fbstream_webrtc

NUM_PROCS=`grep -c ^processor /proc/cpuinfo`
NUM_PROCS=$(($NUM_PROCS + $NUM_PROCS / 2))

echo "Building SVMP"

echo
echo "The WebRTC video streaming that SVMP uses requires a STUN server to function."
echo "If you don't have your own, a list of public STUN servers can be found at"
echo "   http://www.voip-info.org/wiki/view/STUN"
echo
echo "Please enter the hostname or IP address of the server the VM should use: [$SVMP_STUN_HOST]"
read answer
if [ -z $answer ] ; then
  answer=$SVMP_STUN_HOST
fi
if [ -z $answer ] ; then
  echo "You didn't enter a STUN server for SVMP to use. Exiting."
  exit 1
fi
SVMP_STUN_HOST=$answer
export SVMP_STUN_HOST
unset answer

if [ -z $SVMP_STUN_PORT ] ; then
  echo
  echo "Please enter the port the server is operating on [3478]:"
  read SVMP_STUN_PORT
  if [ -z $SVMP_STUN_PORT ] ; then
    SVMP_STUN_PORT=3478
  fi
  export SVMP_STUN_PORT
fi

SVMP_STUN_URL=stun:$SVMP_STUN_HOST:$SVMP_STUN_PORT
echo
echo "The STUN URL you entered is:"
echo $SVMP_STUN_HOST:$SVMP_STUN_PORT
echo
echo "Fbstream will be started with the command line:"
echo "WEBRTC_CONNECT=$SVMP_STUN_URL /system/bin/svmp-fbstream-webrtc2 --server 127.0.0.1"
echo 

read -p "Is this correct? [y/N] " -n 1 -r
echo    # (optional) move to a new line
if [[ $REPLY =~ ^[Yy]$ ]]
then
  echo "Ok. Starting build."
  echo
  rm device/mitre/svmp/fbstream_webrtc
  . build/envsetup.sh
  lunch svmp-eng
  m android_system_disk_vdi android_system_disk_vmdk -j$NUM_PROCS
fi

