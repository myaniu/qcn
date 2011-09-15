#/bin/csh

WEB_BASE_DIR = "$WEB_BASE_DIR"
GMT_DIR = "/usr/local/gmt"

cd $WEB_BASE_DIR/qcn/rt_image/inc

set trig_file_l = "$WEB_BASE_DIR/qcn/rt_image/rt_triggers_LTN.xyz"
set trig_file_d = "$WEB_BASE_DIR/qcn/rt_image/rt_triggers_DTN.xyz"

set topo = "$GMT_DIR/share/topo/topo_coarse.grd"
set cpt  = "$WEB_BASE_DIR/qcn/rt_image/inc/GMT_gray.cpt"
set proj = "-JM4"
set bounds = "-R-180/180/-70/70"
set outdir = "$WEB_BASE_DIR/qcn/rt_image/images"
set psfile = "$WEB_BASE_DIR/qcn/rt_image/images/rt_triggers_base.ps"
set psfilel = "$WEB_BASE_DIR/qcn/rt_image/images/rt_triggers_ltn.ps"
set psfiled = "$WEB_BASE_DIR/qcn/rt_image/images/rt_triggers_dtn.ps"
set cptfile = "$WEB_BASE_DIR/qcn/rt_image/inc/int.cpt"
set flag1   = "-K -P -X4 -Y4 "
set flag2   = "-K -O"
set flag3   = "-O"

if (! -e $psfile) then

$GMT_DIR/bin/psbasemap $bounds $proj -B60f60/30f30:horizontal:WSne -P -K --ANNOT_FONT_SIZE_PRIMARY=5 --FRAME_WIDTH=0.02i --TICK_LENGTH=0.01i --TICK_PEN=0.125p $flag1 > $psfile

$GMT_DIR/bin/grdimage $topo -C$cpt $proj $bounds $flag2 >> $psfile
$GMT_DIR/bin/pscoast -Dc $proj $bounds -W0.25p/0 $flag2 >> $psfile

$GMT_DIR/bin/gmtset ANNOT_FONT_SIZE_PRIMARY 9p

endif

cp $psfile $psfilel
cp $psfile $psfiled

$GMT_DIR/bin/psxy $trig_file_l $proj $bounds $flag3 -St0.1 -W0.125 -C$cptfile >> $psfilel
$GMT_DIR/bin/ps2raster $psfilel -D$outdir -A -P -Tj

$GMT_DIR/bin/psxy $trig_file_d $proj $bounds $flag3 -St0.1 -W0.125 -C$cptfile >> $psfiled
$GMT_DIR/bin/ps2raster $psfiled -D$outdir -A -P -Tj

