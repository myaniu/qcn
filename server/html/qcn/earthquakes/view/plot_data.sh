#/usr/bin/env bash
if [ -e ../inc/bash.env ]; then
    source ../inc/bash.env
elif [ -e inc/bash.env ]; then
    source inc/bash.env
fi

# for security test that dir is a subdirectory of BASEPATH (and that BASEPATH is set)
MYDIR=$1
QCNDIR=$BASEPATH/qcn/earthquakes/view

# test that directories don't exist, aren't blank, and are in the proper place
if [ -z $MYDIR ] || [ -z $BASEPATH ] || [ ! -e $MYDIR ] || [ `echo $MYDIR | grep -c $QCNDIR` -eq 0 ]; then 
  echo "Invalid directory passed in: '$QCNDIR' not part of '$MYDIR'"
  exit
fi

cd $MYDIR
$UNZIP_CMD -o *.zip 
$SACPATH/bin/sac << EOF
r *Z.sac *Y.sac *X.sac
rmean
xvport 0.1 0.5
qdp off
fileid l ur
fileid t l kcmpnm
bp n 2 co 0.05 10.0
color on inc on list blue y g BACKGROUND black
setbb ds '( concatenate Station:' &1,kstnm ' Date:'&1,kzdate ' Time:' &,kztime ')'
title ' ( concatenate ' %ds% ')'
xlabel 'time [sec]'
ylabel 'Acceleration [nm/s/s]'
begg sgf
p1
endg sgf
q
EOF

$SACPATH/bin/sgftops f001.sgf waveform.ps
$GMTPATH/bin/ps2raster waveform.ps -D$MYDIR -A -Tj -P

rm -f $MYDIR/*.zip
rm -f $MYDIR/*.sac
rm -f $MYDIR/*.sgf
rm -f $MYDIR/*.ps
rm -f $MYDIR/temp.txt
