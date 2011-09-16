#!/bin/bash
/bin/rm -f /tmp/qcn-web-backup.tgz
cd /var/www
/bin/nice -n19 /bin/tar -cvzf /tmp/qcn-web-backup.tgz \
  --exclude "boinc/sensor/pid_qcn-web/*" \
  --exclude "boinc/sensor/pid_qcn-web" \
  --exclude "boinc/sensor/log_qcn-web/*" \
  --exclude "boinc/sensor/log_qcn-web" \
  --exclude "boinc/sensor/tmp_qcn-web/*" \
  --exclude "boinc/sensor/tmp_qcn-web" \
  --exclude "boinc/mpitest/*" \
  --exclude "boinc/mpitest" \
  --exclude "boinc/sensor/html/cache/*" \
  --exclude "boinc/sensor/html/cache" \
  --exclude "boinc/sensor/html/stats/*" \
  --exclude "boinc/sensor/html/stats" \
  --exclude "boinc/continual/pid_qcn-web/*" \
  --exclude "boinc/continual/pid_qcn-web" \
  --exclude "boinc/continual/log_qcn-web/*" \
  --exclude "boinc/continual/log_qcn-web" \
  --exclude "boinc/continual/tmp_qcn-web/*" \
  --exclude "boinc/continual/tmp_qcn-web" \
  --exclude "boinc/continual/html/cache/*" \
  --exclude "boinc/continual/html/cache" \
  --exclude "boinc/continual/html/stats/*" \
  --exclude "boinc/continual/html/stats" \
  --exclude "qcn/earthquakes/aichung/*" \
  --exclude "qcn/earthquakes/aichung" \
  --exclude "qcn/earthquakes/view/12*/*" \
  --exclude "qcn/earthquakes/view/12*" \
  --exclude "qcn/earthquakes/view/13*/*" \
  --exclude "qcn/earthquakes/view/13*" \
  --exclude "qcn/earthquakes/13*/*" \
  --exclude "qcn/earthquakes/13*" \
  --exclude "qcn/earthquakes/SAFE*/*" \
  --exclude "qcn/earthquakes/SAFE*" \
  --exclude "qcn/earthquakes/12*/*" \
  --exclude "qcn/earthquakes/12*" \
 boinc/ qcn/ qcnwp/ \
 1>/root/backupweb.log 2>/root/backupweb.err
# note for the following line need to have .ssh id between root & carlgt1 setup
# also make sure that '.ssh/known_hosts' is setup to receive connections
/bin/nice -n19 /usr/bin/scp /tmp/qcn-web-backup.tgz carlgt1@upl-private:
/bin/nice -n19 /bin/mv -f /tmp/qcn-web-backup.tgz /data/QCN/
