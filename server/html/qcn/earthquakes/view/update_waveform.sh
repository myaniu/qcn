#/bin/csh
source ../inc/csh.env

set dir = $argv[1]
set dis = $argv[2]

cd $dir

#echo "<p>$dir $dis"

$UNZIP_CMD -o *.zip 


$SACPATH/bin/sac << EOF
r *Z.sac *Y.sac *X.sac
rmean
div 1000000000
add $dis
write over
q
EOF

rm *.zip
#rm temp.txt

