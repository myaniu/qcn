# note - this is risky so make sure you have backed up your computer first!
#!/bin/bash
chmod -R 755 /System/Library/Extensions/ExarUSBCDCACM.kext
chown -R root:wheel /System/Library/Extensions/ExarUSBCDCACM.kext
rm -R /System/Library/Extensions.kextcache
rm -R /System/Library/Extensions.mkext
