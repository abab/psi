include(../common.pri)
TARGET = models_test
SOURCES += modelstest.cpp

HEADERS += $$PWD/../../history/backend.h \
	    $$PWD/../../history/model_historyitem.h \
	    $$PWD/../../history/model_basehistorymodel.h \
	    $$PWD/../../history/models.h \
	    $$PWD/../../history/xep82datetime.h

SOURCES += $$PWD/../../history/backend.cpp \
	    $$PWD/../../history/model_historyitem.cpp \
	    $$PWD/../../history/model_basehistorymodel.cpp \
	    $$PWD/../../history/models.cpp \
	    $$PWD/../../history/xep82datetime.cpp
