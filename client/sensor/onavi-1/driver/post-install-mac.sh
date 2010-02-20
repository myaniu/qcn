# note - this is risky so make sure you have backed up your computer first!
#!/bin/bash
chmod -fR 755 /System/Library/Extensions/ExarUSBCDCACM.kext
chown -fR root:wheel /System/Library/Extensions/ExarUSBCDCACM.kext
rm -fR /System/Library/Extensions.kextcache
rm -fR /System/Library/Extensions.mkext
