# DEFINES += HISTORY_DEBUG_BACKEND
DEFINES += HISTORY_DEBUG_MODELS

RESOURCES += $$PWD/icons.qrc

HEADERS += $$PWD/meta.h

FORMS += $$PWD/dialog.ui

HEADERS += $$PWD/backend.h \ 
			$$PWD/models.h \
			$$PWD/dialog.h \
#			$$PWD/archivetask.h \
			$$PWD/rsm.h \				# FIXEM move this out from 'history' dir
			$$PWD/xep82datetime.h		# FIXME move this out from 'history' dir

SOURCES += $$PWD/backend.cpp \ 
			$$PWD/models.cpp \
			$$PWD/dialog.cpp \
#			$$PWD/archivetask.cpp \
			$$PWD/rsm.cpp \				# FIXEM move this out from 'history' dir
			$$PWD/xep82datetime.cpp		# FIXME move this out from 'history' dir
