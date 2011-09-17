#/bin/csh
source ../inc/csh.env

set dir = $argv[1]"/"
set dis_max = $argv[2]
set dis_min = $argv[3]

cd $dir


$SACPATH/bin/sac << EOF
r */*X.sac
fileid off
qdp off
xvport 0.1 0.5
ylim $dis_min $dis_max
color on inc on list blue black
title ' Event: '
xlabel 'time [sec]'
ylabel 'Distance [km]'
begg sgf
p2
endg
q
EOF

$SACPATH/bin/sgftops f001.sgf $dir\waveform.ps
$GMTPATH/bin/ps2raster $dir\waveform.ps -D$dir -A -Tj -P

rm $dir*.sgf
rm *.ps
#rm temp.txt
