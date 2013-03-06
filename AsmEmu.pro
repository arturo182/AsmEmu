QT       += core gui widgets

TARGET    = AsmEmu
TEMPLATE  = app
DESTDIR   = bin

QMAKE_CXXFLAGS += -std=c++11
QMAKE_LFLAGS   += -static-libgcc -static-libstdc++

SOURCES  += main.cpp \
            mainwindow.cpp \
            asmhighlighter.cpp \
            codeedit.cpp \
            memoryview.cpp \
            virtualmachine.cpp

HEADERS  += mainwindow.h \
            asmhighlighter.h \
            codeedit.h \
            memoryview.h \
            virtualmachine.h \
    contants.h

FORMS     = mainwindow.ui

RESOURCES = res.qrc

RC_FILE   = res.rc

TRANSLATIONS = trans/asmemu_pl.ts trans/asmemu_untranslated.ts
