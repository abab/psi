DEFINES += HISTORY_DEBUG_BACKEND
DEFINES += HISTORY_DEBUG_MODELS

RESOURCES += $$PWD/icons.qrc

FORMS += $$PWD/dialog.ui

HEADERS += $$PWD/backend.h \ 
			$$PWD/xep82datetime.h \		# FIXME move this out from 'history' dir
			$$PWD/model_historyitem.h \
                        $$PWD/model_base.h \
                        $$PWD/model_collections.h \
                        $$PWD/dialog.h

SOURCES += $$PWD/backend.cpp \ 
			$$PWD/xep82datetime.cpp	\	# FIXME move this out from 'history' dir
			$$PWD/model_historyitem.cpp \
                        $$PWD/model_base.cpp \
                        $$PWD/model_collections.cpp \
                        $$PWD/dialog.cpp
