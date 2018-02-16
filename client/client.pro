#-------------------------------------------------
#
# Project created by QtCreator 2016-07-17T05:31:40
#
#-------------------------------------------------

QT       += core gui widgets network websockets concurrent

unix:!macx {
QT += x11extras
}

TARGET = signet
TEMPLATE = app
QMAKE_CFLAGS += -std=c99 -msse4.1
QMAKE_CXXFLAGS += -std=c++11 -msse4.1 -DQTCSV_STATIC_LIB

macx {
ICON = images/signet.icns
LIBS += -framework CoreFoundation
LIBS += /usr/local/lib/libgcrypt.a /usr/local/lib/libgpg-error.a -lz
INCLUDEPATH+=/usr/local/include
QMAKE_LFLAGS += -L/usr/local/lib
}

unix:!macx {
LIBS += -lgcrypt -lgpg-error -lz -lX11
}

win32 {
QMAKE_LFLAGS = -static
LIBS += -lhid -lsetupapi -lz -lgcrypt -lgpg-error
}

INCLUDEPATH+=../qtcsv/include
INCLUDEPATH+=../qtcsv

SOURCES += ../signet-base/signetdev/host/signetdev.c \
    import/entryrenamedialog.cpp \
    ../qtcsv/sources/contentiterator.cpp \
    ../qtcsv/sources/reader.cpp \
    ../qtcsv/sources/stringdata.cpp \
    ../qtcsv/sources/variantdata.cpp \
    import/csvimporter.cpp \
    import/csvimportconfigure.cpp

unix {
HEADERS += ../signet-base/signetdev/host/signetdev_unix.h \
    import/passimporter.h \
    import/passimportunlockdialog.h \
SOURCES += ../signet-base/signetdev/host/signetdev_unix.c \
    import/passimporter.cpp \
    import/passimportunlockdialog.cpp \
}

win32 {
SOURCES += ../signet-base/signetdev/host/rawhid/hid_WINDOWS.c \
        ../signet-base/signetdev/host/signetdev_win32.c
}

macx {
SOURCES += ../signet-base/signetdev/host/signetdev_osx.c
HEADERS += ../signet-base/signetdev/host/signetdev_osx.h
}

unix:!macx {
SOURCES += ../signet-base/signetdev/host/signetdev_linux.c
}

SOURCES += main.cpp \
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
    opengeneric.cpp \
    genericfields.cpp \
    genericfieldedit.cpp \
    genericfieldeditfactory.cpp \
    genericfieldseditor.cpp \
    linefieldedit.cpp \
    integerfieldedit.cpp \
    textblockfieldedit.cpp \
    typedescedit.cpp \
    typedesceditor.cpp \
    signetdevserver.cpp \
    signetdevserverconnection.cpp \
    about.cpp \
    keyboardlayouttester.cpp \
    settingsdialog.cpp \
    ../keepassx/src/keys/CompositeKey.cpp \
    ../keepassx/src/keys/PasswordKey.cpp \
    ../keepassx/src/keys/FileKey.cpp \
    ../keepassx/src/core/AutoTypeAssociations.cpp \
    ../keepassx/src/core/Config.cpp \
    ../keepassx/src/core/Database.cpp \
    ../keepassx/src/core/DatabaseIcons.cpp \
    ../keepassx/src/core/Endian.cpp \
    ../keepassx/src/core/Entry.cpp \
    ../keepassx/src/core/EntryAttachments.cpp \
    ../keepassx/src/core/EntryAttributes.cpp \
    ../keepassx/src/core/EntrySearcher.cpp \
    ../keepassx/src/core/FilePath.cpp \
    ../keepassx/src/core/Group.cpp \
    ../keepassx/src/core/InactivityTimer.cpp \
    ../keepassx/src/core/Metadata.cpp \
    ../keepassx/src/core/SignalMultiplexer.cpp \
    ../keepassx/src/core/TimeDelta.cpp \
    ../keepassx/src/core/TimeInfo.cpp \
    ../keepassx/src/core/ToDbExporter.cpp \
    ../keepassx/src/core/Tools.cpp \
    ../keepassx/src/core/Translator.cpp \
    ../keepassx/src/core/Uuid.cpp \
    ../keepassx/src/crypto/Crypto.cpp \
    ../keepassx/src/crypto/CryptoHash.cpp \
    ../keepassx/src/crypto/Random.cpp \
    ../keepassx/src/crypto/SymmetricCipher.cpp \
    ../keepassx/src/crypto/SymmetricCipherGcrypt.cpp \
    ../keepassx/src/format/KeePass2Reader.cpp \
    ../keepassx/src/streams/HashedBlockStream.cpp \
    ../keepassx/src/streams/LayeredStream.cpp \
    ../keepassx/src/streams/qtiocompressor.cpp \
    ../keepassx/src/streams/StoreDataStream.cpp \
    ../keepassx/src/streams/SymmetricCipherStream.cpp \
    ../keepassx/src/format/CsvExporter.cpp \
    ../keepassx/src/format/KeePass1Reader.cpp \
    ../keepassx/src/format/KeePass2RandomStream.cpp \
    ../keepassx/src/format/KeePass2Repair.cpp \
    ../keepassx/src/format/KeePass2Writer.cpp \
    ../keepassx/src/format/KeePass2XmlReader.cpp \
    ../keepassx/src/format/KeePass2XmlWriter.cpp \
    import/keepassunlockdialog.cpp \
    import/databaseimporter.cpp \
    import/databaseimportcontroller.cpp \
    import/keepassimporter.cpp

win32 {
SOURCES += qtsingleapplication/src/qtlockedfile_win.cpp
}

unix {
SOURCES += qtsingleapplication/src/qtlockedfile_unix.cpp /mingw64/qt5-static/bin/qmake.exe ../client/client.pro CONFIG=release
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
    signetapplication.h \
    keygeneratorthread.h \
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
    opengeneric.h \
    genericfields.h \
    genericfieldedit.h \
    genericfieldeditfactory.h \
    genericfieldseditor.h \
    linefieldedit.h \
    integerfieldedit.h \
    textblockfieldedit.h \
    typedescedit.h \
    typedesceditor.h \
    signetdevserver.h \
    signetdevserverconnection.h \
    about.h \
    keyboardlayouttester.h \
    localsettings.h \
    settingsdialog.h \
    ../keepassx/src/core/AutoTypeAssociations.h \
    ../keepassx/src/core/Config.h \
    ../keepassx/src/core/Database.h \
    ../keepassx/src/core/DatabaseIcons.h \
    ../keepassx/src/core/Endian.h \
    ../keepassx/src/core/Entry.h \
    ../keepassx/src/core/EntryAttachments.h \
    ../keepassx/src/core/EntryAttributes.h \
    ../keepassx/src/core/EntrySearcher.h \
    ../keepassx/src/core/Exporter.h \
    ../keepassx/src/core/FilePath.h \
    ../keepassx/src/core/Global.h \
    ../keepassx/src/core/Group.h \
    ../keepassx/src/core/InactivityTimer.h \
    ../keepassx/src/core/ListDeleter.h \
    ../keepassx/src/core/Metadata.h \
    ../keepassx/src/core/SignalMultiplexer.h \
    ../keepassx/src/core/TimeDelta.h \
    ../keepassx/src/core/TimeInfo.h \
    ../keepassx/src/core/ToDbExporter.h \
    ../keepassx/src/core/Tools.h \
    ../keepassx/src/core/Translator.h \
    ../keepassx/src/core/Uuid.h \
    ../keepassx/src/crypto/Crypto.h \
    ../keepassx/src/crypto/CryptoHash.h \
    ../keepassx/src/crypto/Random.h \
    ../keepassx/src/crypto/SymmetricCipher.h \
    ../keepassx/src/crypto/SymmetricCipherBackend.h \
    ../keepassx/src/crypto/SymmetricCipherGcrypt.h \
    ../keepassx/src/format/KeePass2.h \
    ../keepassx/src/format/KeePass2Reader.h \
    ../keepassx/src/streams/HashedBlockStream.h \
    ../keepassx/src/streams/LayeredStream.h \
    ../keepassx/src/streams/qtiocompressor.h \
    ../keepassx/src/streams/StoreDataStream.h \
    ../keepassx/src/streams/SymmetricCipherStream.h \
    ../keepassx/src/format/CsvExporter.h \
    ../keepassx/src/format/KeePass1.h \
    ../keepassx/src/format/KeePass1Reader.h \
    ../keepassx/src/format/KeePass2RandomStream.h \
    ../keepassx/src/format/KeePass2Repair.h \
    ../keepassx/src/format/KeePass2Writer.h \
    ../keepassx/src/format/KeePass2XmlReader.h \
    ../keepassx/src/format/KeePass2XmlWriter.h \
    ../keepassx/src/keys/CompositeKey_p.h \
    ../keepassx/src/keys/CompositeKey.h \
    ../keepassx/src/keys/FileKey.h \
    ../keepassx/src/keys/Key.h \
    ../keepassx/src/keys/PasswordKey.h \
    qtsingleapplication/src/qtlockedfile.h \
    esdbgenericmodule.h \
    import/keepassunlockdialog.h \
    import/databaseimporter.h \
    import/databaseimportcontroller.h \
    import/keepassimporter.h \
    import/entryrenamedialog.h \
    import/csvimporter.h \
    ../qtcsv/sources/contentiterator.h \
    ../qtcsv/sources/filechecker.h \
    ../qtcsv/sources/symbols.h \
    import/csvimportconfigure.h

INCLUDEPATH+=../scrypt
INCLUDEPATH+=qtsingleapplication/src
INCLUDEPATH+=../keepassx/src
INCLUDEPATH+=../src
INCLUDEPATH += ../signet-base

win32 {
RC_FILE = signet.rc
}

RESOURCES = resources.qrc

DISTFILES += signet.rc \
    images/logos/fandango_orig.png \
    images/logos/indiegogo_orig.png \
    images/logos/instagram_orig.png \
    images/logos/qt_orig.png \
    images/logos/slack_orig.png \
    images/signet.iconset/icon_128x128.png \
    images/signet.iconset/icon_16x16.png \
    images/signet.iconset/icon_256x256.png \
    images/signet.iconset/icon_32x32.png \
    images/signet.iconset/icon_512x512.png \
    images/signet.iconset/icon_64x64.png \
    images/chase_bank.png \
    images/clipboard.svg \
    images/signet.ico \
    images/button_press.xcf \
    images/keyboard.xcf \
    images/signet.xcf \
    images/vault.xcf \
    images/vault_open.xcf

FORMS += \
    mainwindow.ui
