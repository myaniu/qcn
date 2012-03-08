#/usr/bin/env bash
if [ -e ../inc/bash.env ]; then
    source ../inc/bash.env
elif [ -e inc/bash.env ]; then
    source inc/bash.env
fi

# for security test that dir is a subdirectory of BASEPATH (and that BASEPATH is set)
MYDIR=$1
DIS_MAX=$2
DIS_MIN=$3
QCNDIR=$BASEPATH/qcnwp/earthquakes/view

# test that directories don't exist, aren't blank, and are in the proper place
if [ -z $MYDIR ] || [ -z $BASEPATH ] || [ ! -e $MYDIR ] || [ `echo $MYDIR | grep -c $QCNDIR` -eq 0 ]; then
  echo "Invalid directory passed in: '$QCNDIR' not part of '$MYDIR'"
  exit
fi

cd $MYDIR
#$UNZIP_CMD -o *.zip
#r *Z.sac *Y.sac *X.sac

$SACPATH/bin/sac << EOF
r */*X.sac
fileid off
qdp off
xvport 0.1 0.5
ylim $DIS_MIN $DIS_MAX
color on inc on list blue black
title ' Event: '
xlabel 'time [sec]'
ylabel 'Distance [km]'
begg sgf
p2
endg
q
EOF

$SACPATH/bin/sgftops f001.sgf $MYDIR/waveform.ps
$GMTPATH/bin/ps2raster $MYDIR/waveform.ps -D$MYDIR -A -Tj -P

rm -f $MYDIR/*.sgf
rm -f $MYDIR/*.ps
rm -f $MYDIR/temp.txt
