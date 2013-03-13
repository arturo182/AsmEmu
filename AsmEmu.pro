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
            virtualmachine.cpp \
            compiler.cpp \
    evaluator.cpp

HEADERS  += mainwindow.h \
            asmhighlighter.h \
            codeedit.h \
            memoryview.h \
            virtualmachine.h \
            contants.h \
            compiler.h \
    evaluator.h

FORMS     = mainwindow.ui

RESOURCES = res.qrc

RC_FILE   = res.rc

TRANSLATIONS = trans/asmemu_pl.ts trans/asmemu_untranslated.ts
