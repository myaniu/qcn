@call "C:\Program Files\Microsoft Visual Studio 8\VC\vcvarsall.bat"
@c:
@cd \qt
@set PATH=%PATH%;c:\qt\bin
nmake confclean
REM @configure -platform win32-msvc -dsp -qt-libmng -qt-libpng -qt-libjpeg -qmake -plugin-sql-mysql -plugin-sql-odbc -saveconfig qt-conf
@configure -debug-and-release -static -no-fast -exceptions -stl -plugin-sql-mysql -no-qt3support -platform win32-msvc2003 -system-zlib -system-libpng -system-libjpeg -vcproj
@echo 'You may have to move zlib.h and zconf.h to "C:\Program Files\Microsoft Visual Studio 8\VC\PlatformSDK\Include"'
@echo 'Also move zlibd.lib and zlib.lib to "C:\Program Files\Microsoft Visual Studio 8\VC\PlatformSDK\lib"'
@echo 'Then edit the Makefile.Release and Makefile.Debug to add zlib or zlibd to the LIBS line as necessary'
nmake
