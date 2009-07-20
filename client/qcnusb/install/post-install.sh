#!/bin/sh
#run qcnusb
launchctl load -w /Library/LaunchDaemons/edu.stanford.qcn.qcnusb.plist
nohup /Library/QCN/qcnusb 1>/tmp/qcnusb.out 2>/tmp/qcnusb.err &
