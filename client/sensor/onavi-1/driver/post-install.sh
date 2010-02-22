#!/bin/sh
if [ -f /System/Library/Extensions/ExarUSBCDCACM.kext ]
then
  chmod -fR 755 /System/Library/Extensions/ExarUSBCDCACM.kext
  chown -fR root:wheel /System/Library/Extensions/ExarUSBCDCACM.kext
fi
if [ -f /System/Library/Extensions.kextcache ]
then
  rm -fR /System/Library/Extensions.kextcache
fi
if [ -f /System/Library/Extensions.mkext ]
then
  rm -fR /System/Library/Extensions.mkext
fi
