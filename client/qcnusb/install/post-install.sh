#!/bin/sh
LQCN="/Library/QCN"
LCTL="/bin/launchctl"
PLIST="/Library/LaunchDaemons/edu.stanford.qcn.qcnusb.plist"
if [ -e $LQCN/qcnusb ] && [ -e $PLIST ] && [ -e $LCTL ];
then
  $LCTL load $PLIST
else
  exit 1
fi
