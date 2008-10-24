#DEFINES += HISTORY_DEBUG_BACKEND
#DEFINES += HISTORY_DEBUG_MODELS

MOC_DIR     = _build
UI_DIR      = _build
OBJECTS_DIR = _build
RCC_DIR     = _build

QT += testlib sql
CONFIG += debug warn_on
TEMPLATE = app
INCLUDEPATH += $$PWD/../history

# Iris & QCA - linking problems? tune there
include($$PWD/../../third-party/qca/qca.pri)
include($$PWD/../../iris/iris.pri)
LIBS += -lqca_psi	# no idea why this becomes needed... :/

# I want _really_ clean code
QMAKE_CXXFLAGS_DEBUG += -ansi -pedantic -Wno-long-long -Wold-style-cast 
QMAKE_CXXFLAGS_DEBUG += -Wall -Wextra -Woverloaded-virtual
# QMAKE_CXXFLAGS_DEBUG += -Weffc++ - sadly, tons of "should be initialised" warnings are brain-dead :/
