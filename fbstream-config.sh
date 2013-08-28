#!/bin/sh

# script gets run with AOSP root as its working dir, so need full path
OUT_FILE=device/mitre/svmp/fbstream_webrtc

echo "The WebRTC video streaming that SVMP uses requires a STUN server to function."
echo "If you don't have your own, a list of public STUN servers can be found at"
echo "   http://www.voip-info.org/wiki/view/STUN"
echo

echo "Please enter the hostname or IP address of the server the VM should use:"
read HOST

echo
echo "Please enter the port the server is operating on [3478]:"
read PORT
if [ -z $PORT ] ; then
  PORT=3478
fi

STUNURL=stun:$HOST:$PORT

echo
echo "The STUN URL you entered is:"
echo $HOST:$PORT
echo
echo "Fbstream will be started with the command line:"
echo "WEBRTC_CONNECT=$STUNURL /system/bin/svmp-fbstream-webrtc2 --server 127.0.0.1"
echo 
echo "If this isn't correct cancel the build now."
echo "Continuing in 10 seconds."
sleep 10

cat > $OUT_FILE <<EOF
#!/system/bin/sh

WEBRTC_CONNECT=$STUNURL
/system/bin/svmp-fbstream-webrtc2 --server 127.0.0.1

EOF
chmod 755 $OUT_FILE
