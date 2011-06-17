#/bin/csh
cd /var/www/qcn/rt_image/inc

set trig_file_l = "/var/www/qcn/rt_image/rt_triggers_LTN.xyz"
set trig_file_d = "/var/www/qcn/rt_image/rt_triggers_DTN.xyz"

set gmt  = "/usr/local/gmt/"
set topo = "/usr/local/gmt/share/topo/topo_coarse.grd"
set cpt  = "/var/www/qcn/rt_image/inc/GMT_gray.cpt"
set proj = "-JM4"
set bounds = "-R-180/180/-70/70"
set outdir = "/var/www/qcn/rt_image/images"
set psfile = "/var/www/qcn//rt_image/images/rt_triggers_base.ps"
set psfilel = "/var/www/qcn/rt_image/images/rt_triggers_ltn.ps"
set psfiled = "/var/www/qcn/rt_image/images/rt_triggers_dtn.ps"
set cptfile = "/var/www/qcn/rt_image/inc/int.cpt"
set flag1   = "-K -P -X4 -Y4 "
set flag2   = "-K -O"
set flag3   = "-O"

if (! -e $psfile) then

$gmt/bin/psbasemap $bounds $proj -B60f60/30f30:horizontal:WSne -P -K --ANNOT_FONT_SIZE_PRIMARY=5 --FRAME_WIDTH=0.02i --TICK_LENGTH=0.01i --TICK_PEN=0.125p $flag1 > $psfile

$gmt/bin/grdimage $topo -C$cpt $proj $bounds $flag2 >> $psfile
/usr/local/gmt/bin/pscoast -Dc $proj $bounds -W0.25p/0 $flag2 >> $psfile

$gmt/bin/gmtset ANNOT_FONT_SIZE_PRIMARY 9p

endif

cp $psfile $psfilel
cp $psfile $psfiled

/usr/local/gmt/bin/psxy $trig_file_l $proj $bounds $flag3 -St0.1 -W0.125 -C$cptfile >> $psfilel
/usr/local/gmt/bin/ps2raster $psfilel -D$outdir -A -P -Tj

/usr/local/gmt/bin/psxy $trig_file_d $proj $bounds $flag3 -St0.1 -W0.125 -C$cptfile >> $psfiled
/usr/local/gmt/bin/ps2raster $psfiled -D$outdir -A -P -Tj

