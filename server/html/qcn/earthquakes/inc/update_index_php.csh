source csh.env
foreach d (1?????????)
#cp $d/index.php $d/index.php_safe
#cp inc/index_earthquake.php $d/index.php
foreach d2 ($d/?)
#cp $d2/index.php $d2/index.php_safe
#cp inc/index_earthquake_sub.php $d2/index.php
cd $d2
$PHPPATH $BASEPATH/qcn/earthquakes/inc/gmt_map.php
cd ../..
end


end
