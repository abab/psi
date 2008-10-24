QT += xml sql
# to ensure that new code will not use legacy stuff
QT -= qt3support

MOC_DIR = _build
UI_DIR = _build
OBJECTS_DIR = _build
RCC_DIR = _build

CONFIG += debug warn_on

TEMPLATE = app
TARGET = proto
include($$PWD/history/history.pri)
SOURCES += main.cpp

# Iris & QCA - linking problems? tune there
include($$PWD/../third-party/qca/qca.pri)
include($$PWD/../iris/iris.pri)
LIBS += -lqca_psi	# no idea why this becomes needed... :/

# I want _really_ clean code
QMAKE_CXXFLAGS_DEBUG += -ansi -pedantic -Wno-long-long -Wold-style-cast 
QMAKE_CXXFLAGS_DEBUG += -Wall -Wextra -Woverloaded-virtual
# QMAKE_CXXFLAGS_DEBUG += -Weffc++ - sadly, tons of "should be initialised" warnings are brain-dead :/
