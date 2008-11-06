#!/bin/bash
#converts png files to scaled by $1 xpm files

MIN_ARGS=1

if [ $# -lt $MIN_ARGS ]
then
    echo "Usage: ./conv.sh scale --- converts all png files in directory to "scale" width/height xpm icon files"
    exit 1
fi

#for i in `ls *.png`;
#do
#    convert -scale $1 $i $i.xpm
#done
rm *.xpm
convert -scale $1 cube.png xpm_icon_cube.xpm
convert -scale $1 absolute.png xpm_icon_absolute.xpm
convert -scale $1 scaled.png xpm_icon_scaled.xpm
convert -scale $1 one.png xpm_icon_one.xpm
convert -scale $1 ten.png xpm_icon_ten.xpm
convert -scale $1 twod.png xpm_icon_twod.xpm
convert -scale $1 threed.png xpm_icon_threed.xpm
convert -scale $1 sixty.png xpm_icon_sixty.xpm
convert -scale $1 blank.png xpm_icon_blank.xpm
convert -scale $1 camera.png xpm_icon_camera.xpm
convert -scale $1 earth.png xpm_icon_earth.xpm
convert -scale $1 ff.png xpm_icon_ff.xpm
convert -scale $1 moon.png xpm_icon_moon.xpm
convert -scale $1 nospin.png xpm_icon_nospin.xpm
convert -scale $1 sync.png xpm_icon_sync.xpm
convert -scale $1 pause.png xpm_icon_pause.xpm
convert -scale $1 play.png xpm_icon_play.xpm
convert -scale $1 record.png xpm_icon_record.xpm
convert -scale $1 rw.png xpm_icon_rw.xpm
convert -scale $1 spin.png xpm_icon_spin.xpm
convert -scale $1 stop.png xpm_icon_stop.xpm
convert -scale $1 sun.png xpm_icon_sun.xpm
convert -scale $1 usgs.png xpm_icon_usgs.xpm
cat *.xpm > ../icons.h
