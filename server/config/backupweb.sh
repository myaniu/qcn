#!/bin/bash
/bin/rm -f /tmp/qcn-web-backup.tgz
cd /var/www/
/bin/nice -n19 /bin/tar -cvzf /tmp/qcn-web-backup.tgz \
  --exclude "/var/www/boinc/sensor/pid_qcn-web/*" \
  --exclude "/var/www/boinc/sensor/log_qcn-web/*" \
  --exclude "/var/www/boinc/sensor/tmp_qcn-web/*" \
  --exclude "/var/www/boinc/sensor/html/cache/*" \
  --exclude "/var/www/boinc/sensor/html/stats/*" \
  --exclude "/var/www/boinc/continual/pid_qcn-web/*" \
  --exclude "/var/www/boinc/continual/log_qcn-web/*" \
  --exclude "/var/www/boinc/continual/tmp_qcn-web/*" \
  --exclude "/var/www/boinc/continual/html/cache/*" \
  --exclude "/var/www/boinc/continual/html/stats/*" \
 boinc/ qcn/ \
 1>/root/backupweb.log 2>/root/backupweb.err
# note for the following line need to have .ssh id between root & carlgt1 setup
# also make sure that '.ssh/known_hosts' is setup to receive connections
/bin/nice -n19 /usr/bin/scp /tmp/qcn-web-backup.tgz carlgt1@upl-private:
/bin/nice -n19 /bin/mv -f /tmp/qcn-web-backup.tgz /data/QCN/
