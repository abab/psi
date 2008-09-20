#DEFINES += HISTORY_DEBUG_BACKEND
#DEFINES += HISTORY_DEBUG_MODELS

MOC_DIR     = .build
UI_DIR      = .build
OBJECTS_DIR = .build
RCC_DIR     = .build

QT += testlib sql
CONFIG += debug warn_on
TEMPLATE = app
INCLUDEPATH += $$PWD/../history

# Iris & QCA
include($$PWD/../../iris/iris.pri)
