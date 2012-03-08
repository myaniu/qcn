#/usr/bin/env bash
if [ -e ../inc/bash.env ]; then
    source ../inc/bash.env
elif [ -e inc/bash.env ]; then
    source inc/bash.env
fi

# for security test that dir is a subdirectory of BASEPATH (and that BASEPATH is set)
MYDIR=$1
MYDIS=$2

QCNDIR=$BASEPATH/qcnwp/earthquakes/view

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
div 1000000000
add $MYDIS
write over
q
EOF

rm -f *.zip
