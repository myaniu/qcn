#source csh.env

#cd ../
foreach d (1?????????)

cd $d
cp event.xy event_safe.xy
cp event.xy event_safe_orig.xy
cd ..
foreach d2 ($d/?)

cd $d2
cp event.xy event_safe.xy
#cp event_safe.xy event.xy
cp event.xy event_safe_orig.xy
/usr/local/bin/php /var/www/qcnwp/earthquakes/inc/revise_magnitude.php
#cp event_safe.xy event.xy
#$PHPPATH $BASEPATH/qcnwp/earthquakes/inc/revise_magnitude.php
cp event.xy ../.
cd ../..
end


end
