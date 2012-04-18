export MARCH=i386
export CXX=/Developer/usr/bin/g++-4.2
export CC=/Developer/usr/bin/gcc-4.2
export MACOSX_DEPLOYMENT_TARGET=10.5
export SDKROOT="/Developer/SDKs/MacOSX10.5.sdk"
export LDFLAGS="-L/Developer/Panda3D/lib -lp3framework -lpanda -lpandafx -lpandaexpress -lp3dtoolconfig -lp3dtool -lp3pystub -lp3direct -L$SDKROOT/usr/lib/ -isysroot $SDKROOT -Wl,-syslibroot,$SDKROOT -arch $MARCH"
export CPPFLAGS="-O2 -I$SDKROOT/usr/include -I$SDKROOT/usr/local/include -isysroot $SDKROOT -arch $MARCH"
export CFLAGS="-DHAVE_POSIX_STRERROR_R=1 -O2 -I$SDKROOT/usr/include -I$SDKROOT/usr/local/include -isysroot $SDKROOT -arch $MARCH"
export CXXFLAGS="-I/usr/include/python2.6 -fPIC -O2 -I/Developer/Panda3D/include -I$SDKROOT/usr/include -I$SDKROOT/usr/local/include -isysroot $SDKROOT -arch $MARCH"
$CXX $CXXFLAGS $LDFLAGS panda.cpp -o test

