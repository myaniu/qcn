#############################################################################
# Makefile for building: QCNLive.app/Contents/MacOS/QCNLive
# Generated by qmake (2.01a) (Qt 4.7.0) on: Mon Nov 22 18:33:45 2010
# Project:  QCNLive.pro
# Template: app
# Command: /usr/bin/qmake -o QCNLive.xcodeproj/project.pbxproj QCNLive.pro
#############################################################################

MOC       = /Developer/Tools/Qt/moc
UIC       = /Developer/Tools/Qt/uic
LEX       = flex
LEXFLAGS  = 
YACC      = yacc
YACCFLAGS = -d
DEFINES       = -D_USE_NTPDATE_EXEC_ -DFTGL_LIBRARY_STATIC -DQCNLIVE -DGRAPHICS_PROGRAM -DAPP_GRAPHICS -D_ZLIB -DQCN -D_THREAD_SAFE -DCURL_STATICLIB -D_ZLIB -DQT_OPENGL_LIB -DQT_GUI_LIB -DQT_CORE_LIB -DQT_SHARED
INCPATH       = -I/usr/local/Qt4.7/mkspecs/macx-xcode -I. -I/Library/Frameworks/QtCore.framework/Versions/4/Headers -I/usr/include/QtCore -I/Library/Frameworks/QtGui.framework/Versions/4/Headers -I/usr/include/QtGui -I/Library/Frameworks/QtOpenGL.framework/Versions/4/Headers -I/usr/include/QtOpenGL -I/usr/include -I../../../qcn -I../../jpeg-6b -I../../zlib-1.2.5 -I../../curl-7.18.2/include -I../../ftgl-2.1.3/include -I../../freetype-2.3.9/include -I../main -I../sensor -I../util -I../../../boinc/lib -I../../../boinc/api -I../../../boinc/zip -I../graphics -I/System/Library/Frameworks/OpenGL.framework/Versions/A/Headers -I/System/Library/Frameworks/AGL.framework/Headers -I. -I/usr/local/include -I/System/Library/Frameworks/CarbonCore.framework/Headers -F/Library/Frameworks
DEL_FILE  = rm -f
MOVE      = mv -f

IMAGES = 
PARSERS =
preprocess: $(PARSERS) compilers
clean preprocess_clean: parser_clean compiler_clean

parser_clean:
check: first

mocclean: compiler_moc_header_clean compiler_moc_source_clean

mocables: compiler_moc_header_make_all compiler_moc_source_make_all

compilers: ./moc_qcnqt.cpp ./moc_glwidget.cpp ./moc_myframe.cpp\
	 ./moc_dlgsettings.cpp ./moc_dlgmakequake.cpp
compiler_objective_c_make_all:
compiler_objective_c_clean:
compiler_moc_header_make_all: moc_qcnqt.cpp moc_glwidget.cpp moc_myframe.cpp moc_dlgsettings.cpp moc_dlgmakequake.cpp
compiler_moc_header_clean:
	-$(DEL_FILE) moc_qcnqt.cpp moc_glwidget.cpp moc_myframe.cpp moc_dlgsettings.cpp moc_dlgmakequake.cpp
moc_qcnqt.cpp: ../graphics/qcn_graphics.h \
		../util/qcn_curl.h \
		myframe.h \
		qcnqt.h \
		qcnlive_define.h \
		glwidget.h \
		qcnqt.h
	/Developer/Tools/Qt/moc $(DEFINES) $(INCPATH) -D__APPLE__ -D__GNUC__ qcnqt.h -o moc_qcnqt.cpp

moc_glwidget.cpp: qcnqt.h \
		../graphics/qcn_graphics.h \
		../util/qcn_curl.h \
		myframe.h \
		glwidget.h \
		qcnlive_define.h \
		glwidget.h
	/Developer/Tools/Qt/moc $(DEFINES) $(INCPATH) -D__APPLE__ -D__GNUC__ glwidget.h -o moc_glwidget.cpp

moc_myframe.cpp: qcnqt.h \
		../graphics/qcn_graphics.h \
		../util/qcn_curl.h \
		myframe.h \
		glwidget.h \
		qcnlive_define.h \
		myframe.h
	/Developer/Tools/Qt/moc $(DEFINES) $(INCPATH) -D__APPLE__ -D__GNUC__ myframe.h -o moc_myframe.cpp

moc_dlgsettings.cpp: qcnqt.h \
		../graphics/qcn_graphics.h \
		../util/qcn_curl.h \
		myframe.h \
		glwidget.h \
		qcnlive_define.h \
		../sensor/csensor.h \
		dlgsettings.h
	/Developer/Tools/Qt/moc $(DEFINES) $(INCPATH) -D__APPLE__ -D__GNUC__ dlgsettings.h -o moc_dlgsettings.cpp

moc_dlgmakequake.cpp: qcnqt.h \
		../graphics/qcn_graphics.h \
		../util/qcn_curl.h \
		myframe.h \
		glwidget.h \
		qcnlive_define.h \
		dlgmakequake.h
	/Developer/Tools/Qt/moc $(DEFINES) $(INCPATH) -D__APPLE__ -D__GNUC__ dlgmakequake.h -o moc_dlgmakequake.cpp

compiler_rcc_make_all:
compiler_rcc_clean:
compiler_image_collection_make_all: qmake_image_collection.cpp
compiler_image_collection_clean:
	-$(DEL_FILE) qmake_image_collection.cpp
compiler_moc_source_make_all:
compiler_moc_source_clean:
compiler_rez_source_make_all:
compiler_rez_source_clean:
compiler_uic_make_all:
compiler_uic_clean:
compiler_yacc_decl_make_all:
compiler_yacc_decl_clean:
compiler_yacc_impl_make_all:
compiler_yacc_impl_clean:
compiler_lex_make_all:
compiler_lex_clean:
compiler_clean: compiler_moc_header_clean 

