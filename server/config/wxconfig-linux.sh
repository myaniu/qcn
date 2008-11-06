#!/bin/bash
# need to reset compiler flags from /etc/profile
rm -rf setup
export CC=/usr/bin/gcc-3.4
export CXX=/usr/bin/g++-3.4
export LDFLAGS=
export CFLAGS=
export CPPFLAGS=
export CXXFLAGS=
mkdir setup
cd setup
make distclean
../configure --disable-compat26 --enable-monolithic --enable-gui --enable-optimise --enable-stl \
--disable-shared --with-libpng --with-libjpeg --with-opengl --with-zlib --with-odbc \
--with-expat --with-libtiff --with-libxpm --with-x11 --disable-sockets
make
echo if it built OK do a sudo make install
