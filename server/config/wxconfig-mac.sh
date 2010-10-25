#!/bin/bash
export CC=gcc-4.0
export CXX=g++-4.0
rm -rf osx-build
mkdir osx-build
cd osx-build
../configure --enable-monolithic --disable-shared --with-opengl --enable-universal_binary --with-macosx-sdk=/Developer/SDKs/MacOSX10.4u.sdk
make
