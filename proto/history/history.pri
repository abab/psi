DEFINES += HISTORY_DEBUG_BACKEND
DEFINES += HISTORY_DEBUG_MODELS

RESOURCES += $$PWD/icons.qrc

FORMS += $$PWD/dialog.ui

HEADERS += $$PWD/backend.h \ 
			$$PWD/xep82datetime.h \		# FIXME move this out from 'history' dir
			$$PWD/model_historyitem.h \
			$$PWD/model_basehistorymodel.h \
			$$PWD/models.h \
			$$PWD/dialog.h
#			$$PWD/archivetask.h \
#			$$PWD/rsm.h					# FIXEM move this out from 'history' dir

SOURCES += $$PWD/backend.cpp \ 
			$$PWD/xep82datetime.cpp	\	# FIXME move this out from 'history' dir
			$$PWD/model_historyitem.cpp \
			$$PWD/model_basehistorymodel.cpp \
			$$PWD/models.cpp \
			$$PWD/dialog.cpp
#			$$PWD/archivetask.cpp \
#			$$PWD/rsm.cpp				# FIXEM move this out from 'history' dir
