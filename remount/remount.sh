#!/system/bin/sh
mount | grep /data
mount_check=$?
while [ $mount_check -ne 0 ]; do
   sleep 5
   ls /dev/block/ | grep [v,s,h]d | grep -v [v,s,h]da | grep [v,s,h]d[b-z]
   result=$?
   if [ $result -eq 0 ]; then
      reboot
   fi
done


