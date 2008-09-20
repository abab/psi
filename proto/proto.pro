QT += xml sql
# to ensure that new code will not use legacy stuff
QT -= qt3support

MOC_DIR = .build
UI_DIR = .build
OBJECTS_DIR = .build
RCC_DIR = .build

CONFIG += debug \
    warn_on

TEMPLATE = app
TARGET = proto
include($$PWD/history/history.pri)
SOURCES += main.cpp

# Iris & QCA
include($$PWD/../third-party/qca/qca.pri)
include($$PWD/../iris/iris.pri)
