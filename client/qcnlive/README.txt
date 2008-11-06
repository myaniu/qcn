NB -- this is obsolete, using Qt-based in the client/qcnlive directory

This directory holds the Mac XCode and Windows projects as well as source code for the "standalone GUI" QCN 
software based on the wxWidgets cross-platform GUI library  (http://wxwidgets.org)

The wxWidgets build settings should enable OpenGL, JPEG, and PNG libraries, and have Unicode char support

This Mac configuration setting for wxWidgets should suffice:

./configure --with-mac --enable-monolithic --enable-unicode --with-opengl --with-expat    --with-libpng --with-libjpeg --with-libtiff --with-libxpm --disable-shared --enable-static    --with-macosx-sdk=/Developer/SDKs/MacOSX10.4u.sdk

Windows should be installed in c:\wxWidgets - the Visual Studio project files are in the zip file in the qcn subversion root directory:

wxwidgets-build-win32-projects.zip


