#make sure you have a backup first just in case!
sudo cp -r ExarUSBCDCACM.kext /System/Library/Extensions
sudo chmod -R 755 /System/Library/Extensions/ExarUSBCDCACM.kext
sudo chown -R root:wheel /System/Library/Extensions/ExarUSBCDCACM.kext
sudo rm -R /System/Library/Extensions.kextcache
sudo rm -R /System/Library/Extensions.mkext
sudo reboot
