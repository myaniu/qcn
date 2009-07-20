#!/bin/sh
#kill any existing qcnusb process - must be su, use signal 15 (SIGTERM) for clean shutdown
for X in `ps acx | grep -i qcnusb | awk {'print $1'}`; do
  kill -15 $X;
done
launchctl unload -w /Library/LaunchDaemons/edu.stanford.qcn.qcnusb.plist
rm -f /Library/LaunchDaemons/edu.stanford.qcn.qcnusb.plist
rm -f /tmp/qcn*
