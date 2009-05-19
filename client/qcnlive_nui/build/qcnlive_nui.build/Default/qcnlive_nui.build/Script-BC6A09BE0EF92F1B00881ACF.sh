#!/bin/sh
rsync -rC ./resources/ --exclude ".*" "$BUILT_PRODUCTS_DIR/$UNLOCALIZED_RESOURCES_FOLDER_PATH"
../../../nui3/tools/make_rc.py resources resource.rc
echo 0 ICON \"resources/icon.ico\" >> resource.rc

