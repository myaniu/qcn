#!/bin/sh
LCTL="/bin/launchctl"
PLIST="/Library/LaunchDaemons/edu.stanford.qcn.qcnusb.plist"
if [ -e $PLIST ] && [ -e $LCTL ];
then
  $LCTL load $PLIST
fi
