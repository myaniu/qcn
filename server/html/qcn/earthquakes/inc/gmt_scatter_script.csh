source csh.env
set GMT      = $GMTPATH/bin
set BOUNDS   = "-R0/100/0/100"
set OUTDIR   = $BASEPATH/qcn/earthquakes/1285960443/A
set T_SCAT   = $OUTDIR/t_scatter.xy
set X1Y1     = "-X3 -Y10" 
set PROJ     = "-JX4i/4i" 
set FLAGS1   = "-K -P " 
set FLAGS2   = "-K -O " 
set FLAGS3   = "-O " 
set PSFILE   = $OUTDIR/t_scatter.ps
set B        = "-B10g10" 
$GMT/psxy $T_SCAT $BOUNDS $PROJ $FLAGS1 -Sc0.75 -G255 -W1p/255/0/0 >> $PSFILE 
$GMT/psxy         $BOUNDS $PROJ $FLAGS3 -m -W1p/175 << EOF >> $PSFILE 
  0.0,  0.0
100.0,100.0
EOF
$GMT/ps2raster $PSFILE -D$OUTDIR -A -P -Tj 
