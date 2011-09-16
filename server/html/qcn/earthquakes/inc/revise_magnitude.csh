source csh.env
foreach d (12????????)

cd $d
#cp event.xy event_safe.xy
cd ..
foreach d2 ($d/?)

cd $d2
cp event_safe.xy event.xy
$PHPPATH $BASEPATH/qcn/earthquakes/inc/revise_magnitude.php
cp event.xy ../.
cd ../..
end


end
