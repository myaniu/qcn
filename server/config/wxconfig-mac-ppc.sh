#!/bin/bash
# need to reset compiler flags from /etc/profile
export CFLAGS=
export CPPFLAGS=
export CXXFLAGS=
make distclean
export LDFLAGS=
./configure --disable-compat26 --enable-monolithic --enable-gui --enable-optimise --enable-stl --disable-shared --with-libpng --with-libjpeg --with-opengl --with-zlib --with-odbc --with-expat --with-libtiff --with-libxpm
make
echo if it built OK do a sudo make install
