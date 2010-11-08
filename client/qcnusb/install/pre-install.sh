#!/bin/sh
#kill any existing qcnusb process - must be su, use signal 15 (SIGTERM) for clean shutdown
for X in `ps acx | grep -i qcnusb | awk {'print $1'}`; do
  kill -15 $X;
done
LCTL="/bin/launchctl"
PLIST="/Library/LaunchDaemons/edu.stanford.qcn.qcnusb.plist"
if [ -e $PLIST ] && [ -e $LCTL ];
then
  $LCTL unload $PLIST
fi
rm -f $PLIST
rm -f /tmp/qcn*
