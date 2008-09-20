include(../common.pri)
TARGET = backend_test
SOURCES += backendtest.cpp

HEADERS += $$PWD/../../history/backend.h \
	    $$PWD/../../history/xep82datetime.h
SOURCES += $$PWD/../../history/backend.cpp \
	    $$PWD/../../history/xep82datetime.cpp
