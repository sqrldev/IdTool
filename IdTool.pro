#-------------------------------------------------
#
# Project created by QtCreator 2019-09-01T10:51:49
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = IdTool
TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

CONFIG += c++11

SOURCES += \
        blockdesignerdialog.cpp \
        cryptutil.cpp \
        identitymodel.cpp \
        identityparser.cpp \
        itemeditordialog.cpp \
        main.cpp \
        mainwindow.cpp \
        uibuilder.cpp

HEADERS += \
        blockdesignerdialog.h \
        cryptutil.h \
        identitymodel.h \
        identityparser.h \
        itemeditordialog.h \
        mainwindow.h \
        uibuilder.h

FORMS += \
        blockdesignerdialog.ui \
        itemeditordialog.ui \
        mainwindow.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    res.qrc

RC_FILE = IdTool.rc

VERSION = 0.2.0    # major.minor.patch
DEFINES += APP_VERSION=\\\"$$VERSION\\\" \
    SODIUM_STATIC

DISTFILES += \
    CHANGELOG \
    blockdef/1.json \
    blockdef/2.json \
    blockdef/3.json \
    lib/sodium/lib/libsodium.lib \
    lib/sodium/lib/libsodiumd.lib

win32:CONFIG(release, debug|release): LIBS += -L$$PWD/lib/sodium/lib/ -llibsodium
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/lib/sodium/lib/ -llibsodiumd
else:unix: LIBS += -L$$PWD/lib/sodium/lib/ -lsodium

INCLUDEPATH += $$PWD/lib/sodium/include
DEPENDPATH += $$PWD/lib/sodium/include
