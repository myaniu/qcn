#!/bin/bash
# need to reset compiler flags from /etc/profile
rm -rf setup
export LDFLAGS=
export CFLAGS=
export CPPFLAGS=
export CXXFLAGS=
mkdir setup
cd setup
make distclean
../configure --with-macosx-sdk=/Developer/SDKs/MacOSX10.4u.sdk --with-macosx-version-min=10.4 --disable-compat26 --enable-monolithic --enable-gui --enable-optimise --enable-stl --disable-shared --with-libpng --with-libjpeg --with-opengl --with-zlib --with-odbc --with-expat --with-libtiff --with-libxpm
make
echo if it built OK do a sudo make install
