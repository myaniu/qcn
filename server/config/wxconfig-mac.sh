#!/bin/bash
mkdir osx-build
cd osx-build
../configure --enable-monolithic --disable-shared --with-opengl --enable-universal_binary --with-macosx-sdk=/Developer/SDKs/MacOSX10.4u.sdk
make
