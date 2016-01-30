#-------------------------------------------------
#
# Project created by QtCreator 2016-07-17T05:31:40
#
#-------------------------------------------------

QT       += core gui network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = signet
TEMPLATE = app
QMAKE_CFLAGS += -DUSE_RAW_HID -msse4.1
QMAKE_CXXFLAGS += -std=c++11 -DUSE_RAW_HID -msse4.1

win32 {
LIBS += -lhid -lsetupapi
}

macx {
LIBS += -framework CoreFoundation
}

SOURCES += main.cpp\
    mainwindow.cpp \
    loginwindow.cpp \
    newaccount.cpp \
    editaccount.cpp \
    account.cpp \
    aspectratiopixmaplabel.cpp \
    loggedinwidget.cpp \
    changemasterpassword.cpp \
    searchlistbox.cpp \
    buttonwaitdialog.cpp \
    searchfilteredit.cpp \
    databasefield.cpp \
    esdb.cpp \
    systemtray.cpp \
    passwordedit.cpp \
    qtsingleapplication/src/qtlocalpeer.cpp \
    qtsingleapplication/src/qtsinglecoreapplication.cpp \
    qtsingleapplication/src/qtsingleapplication.cpp \
    qtsingleapplication/src/qtlockedfile.cpp \
    ../scrypt/crypto_scrypt_smix_sse2.c \
    ../scrypt/crypto_scrypt_smix.c \
    ../scrypt/crypto_scrypt.c \
    ../scrypt/insecure_memzero.c \
    ../scrypt/sha256.c \
    ../scrypt/warnp.c \
    ../device_interface/signetdev.c \
    signetapplication.cpp \
    keygeneratorthread.cpp \
    esdbmodel.cpp \
    bookmark.cpp \
    resetdevice.cpp \
    esdbaccountmodule.cpp \
    esdbbookmarkmodule.cpp \
    esdbtypemodule.cpp \
    esdbactionbar.cpp \
    accountactionbar.cpp \
    bookmarkactionbar.cpp \
    newbookmark.cpp \
    esdbgenericmodule.cpp \
    generic.cpp \
    generictypedesc.cpp \
    genericactionbar.cpp \
    newgeneric.cpp \
    opengeneric.cpp

win32 {
SOURCES += ../device_interface/rawhid/hid_WINDOWS.c
SOURCES += ../device_interface/signetdev_win32.c
SOURCES += qtsingleapplication/src/qtlockedfile_win.cpp
}

unix {
SOURCES += qtsingleapplication/src/qtlockedfile_unix.cpp
}

macx {
SOURCES += ../device_interface/signetdev_osx.c
ICON = images/signet.icns
}

unix:!macx {
SOURCES += ../device_interface/signetdev_linux.c
}

HEADERS  += mainwindow.h \
    loginwindow.h \
    newaccount.h \
    account.h \
    editaccount.h \
    aspectratiopixmaplabel.h \
    loggedinwidget.h \
    changemasterpassword.h \
    searchlistbox.h \
    buttonwaitdialog.h \
    searchfilteredit.h \
    databasefield.h \
    ../common/common.h \
    ../device_interface/signetdev.h \
    esdb.h \
    systemtray.h \
    passwordedit.h \
    qtsingleapplication/src/qtlocalpeer.h \
    qtsingleapplication/src/QtLockedFile \
    qtsingleapplication/src/qtsinglecoreapplication.h \
    qtsingleapplication/src/QtSingleApplication \
    qtsingleapplication/src/qtsingleapplication.h \
    ../scrypt/crypto_scrypt_smix_sse2.h \
    ../scrypt/crypto_scrypt_smix.h \
    ../scrypt/crypto_scrypt.h \
    ../scrypt/insecure_memzero.h \
    ../scrypt/sha256.h \
    ../scrypt/warnp.h \
    ../device_interface/signetdev.h \
    signetapplication.h \
    keygeneratorthread.h \
    ../device_interface/signetdev_priv.h \
    esdbmodel.h \
    bookmark.h \
    resetdevice.h \
    esdbtypemodule.h \
    esdbaccountmodule.h \
    esdbbookmarkmodule.h \
    esdbactionbar.h \
    accountactionbar.h \
    bookmarkactionbar.h \
    newbookmark.h \
    eddbgenericmodule.h \
    generic.h \
    generictypedesc.h \
    genericactionbar.h \
    newgeneric.h \
    opengeneric.h

win32 {
HEADERS += ../device_interface/rawhid/hid.h
}

INCLUDEPATH+=../common
INCLUDEPATH+=../device_interface
INCLUDEPATH+=../scrypt
INCLUDEPATH+=qtsingleapplication/src

win32 {
INCLUDEPATH+=$$PWD/../device_interface/rawhid
RC_FILE = signet.rc
}

RESOURCES = resources.qrc

DISTFILES += \
    signet.rc
