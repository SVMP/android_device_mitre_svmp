#!/bin/sh

# script gets run with AOSP root as its working dir, so need full path
OUT_FILE=device/mitre/svmp/fbstream_webrtc

if [ -z $SVMP_STUN_HOST ] ; then
  echo "Error. No STUN server set. Check the SVMP_STUN_HOST envvar."
  exit 1
fi

if [ -z $SVMP_STUN_PORT ] ; then
  SVMP_STUN_PORT=3478
fi

STUNURL=stun:$SVMP_STUN_HOST:$SVMP_STUN_PORT

echo "Fbstream will be started with the command line:"
echo "WEBRTC_CONNECT=$STUNURL /system/bin/svmp-fbstream --server 127.0.0.1"

cat > $OUT_FILE <<EOF
#!/system/bin/sh

export WEBRTC_CONNECT=$STUNURL
/system/bin/svmp-fbstream --server 127.0.0.1

EOF
chmod 755 $OUT_FILE
