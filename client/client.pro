#-------------------------------------------------
#
# Project created by QtCreator 2016-07-17T05:31:40
#
#-------------------------------------------------

QT       += core gui widgets network concurrent

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

#
# Note: Use this fragment instead of the one above when making a static build
#
#unix:!macx{
#LIBS += -l:libgcrypt.a -l:libgpg-error.a -l:libz.a -lX11
#}

win32 {
QMAKE_LFLAGS = -static
LIBS += -lhid -lsetupapi -lz -lgcrypt -lgpg-error
}

#
# Signetdev sources/headers
#

SOURCES += ../signet-base/signetdev/host/signetdev.c

unix {
HEADERS += ../signet-base/signetdev/host/signetdev_unix.h \
    import/passimporter.h \
    import/passimportunlockdialog.h
SOURCES += ../signet-base/signetdev/host/signetdev_unix.c \
    import/passimporter.cpp \
    import/passimportunlockdialog.cpp
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

#
# ESDB data sources
#
SOURCES += esdb/esdb.cpp \
    esdb/esdbtypemodule.cpp \
    esdb/account/account.cpp \
    esdb/account/esdbaccountmodule.cpp \
    esdb/bookmark/bookmark.cpp \
    esdb/bookmark/esdbbookmarkmodule.cpp \
    esdb/generic/generic.cpp \
    esdb/generic/generictypedesc.cpp \
    esdb/generic/esdbgenericmodule.cpp \
    esdb/generic/genericfields.cpp


#
# ESDB GUI sources
#
SOURCES += esdb-gui/esdbmodel.cpp \
    esdb-gui/databasefield.cpp \
    esdb-gui/passwordedit.cpp \
    esdb-gui/esdbactionbar.cpp \
    esdb-gui/linefieldedit.cpp \
    esdb-gui/integerfieldedit.cpp \
    esdb-gui/textblockfieldedit.cpp \
    esdb-gui/genericfieldedit.cpp \
    esdb-gui/genericfieldeditfactory.cpp \
    esdb-gui/genericfieldseditor.cpp \
    esdb-gui/typedescedit.cpp \
    esdb-gui/typedesceditor.cpp \
    esdb-gui/account/newaccount.cpp \
    esdb-gui/account/editaccount.cpp \
    esdb-gui/account/accountactionbar.cpp \
    esdb-gui/bookmark/bookmarkactionbar.cpp \
    esdb-gui/bookmark/newbookmark.cpp \
    esdb-gui/generic/genericactionbar.cpp \
    esdb-gui/generic/newgeneric.cpp \
    esdb-gui/generic/opengeneric.cpp

#
# Importer sources
#
SOURCES += import/entryrenamedialog.cpp \
    import/keepassunlockdialog.cpp \
    import/databaseimporter.cpp \
    import/databaseimportcontroller.cpp \
    import/keepassimporter.cpp \
    import/csvimporter.cpp \
    import/csvimportconfigure.cpp

#
# Main applicaiton sources
#
SOURCES += main.cpp \
    mainwindow.cpp \
    loginwindow.cpp \
    aspectratiopixmaplabel.cpp \
    loggedinwidget.cpp \
    changemasterpassword.cpp \
    searchlistbox.cpp \
    buttonwaitdialog.cpp \
    searchfilteredit.cpp \
    systemtray.cpp \
    signetapplication.cpp \
    keygeneratorthread.cpp \
    resetdevice.cpp \
    about.cpp \
    keyboardlayouttester.cpp \
    settingsdialog.cpp

#
# External headers
#
SOURCES += ../qtcsv/sources/contentiterator.cpp \
    ../qtcsv/sources/reader.cpp \
    ../qtcsv/sources/stringdata.cpp \
    ../qtcsv/sources/variantdata.cpp \
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
    ../keepassx/src/format/KeePass2XmlWriter.cpp

win32 {
SOURCES += qtsingleapplication/src/qtlockedfile_win.cpp
}

unix {
SOURCES += qtsingleapplication/src/qtlockedfile_unix.cpp
}

#
# ESDB headers
#
HEADERS += esdb/esdb.h \
    esdb/esdbtypemodule.h \
    esdb/account/account.h \
    esdb/account/esdbaccountmodule.h \
    esdb/bookmark/bookmark.h \
    esdb/bookmark/esdbbookmarkmodule.h \
    esdb/generic/generic.h \
    esdb/generic/genericfields.h \
    esdb/generic/generictypedesc.h \
    esdb/generic/esdbgenericmodule.h

#
# ESDB GUI headers
#
HEADERS +=  esdb-gui/esdbmodel.h \
    esdb-gui/databasefield.h \
    esdb-gui/passwordedit.h \
    esdb-gui/esdbactionbar.h \
    esdb-gui/linefieldedit.h \
    esdb-gui/integerfieldedit.h \
    esdb-gui/textblockfieldedit.h \
    esdb-gui/genericfieldedit.h \
    esdb-gui/genericfieldeditfactory.h \
    esdb-gui/genericfieldseditor.h \
    esdb-gui/typedescedit.h \
    esdb-gui/typedesceditor.h \
    esdb-gui/account/newaccount.h \
    esdb-gui/account/editaccount.h \
    esdb-gui/account/accountactionbar.h \
    esdb-gui/bookmark/bookmarkactionbar.h \
    esdb-gui/bookmark/newbookmark.h \
    esdb-gui/generic/genericactionbar.h \
    esdb-gui/generic/newgeneric.h \
    esdb-gui/generic/opengeneric.h

#
# Importer headers
#
HEADERS += import/keepassunlockdialog.h \
    import/databaseimporter.h \
    import/databaseimportcontroller.h \
    import/keepassimporter.h \
    import/entryrenamedialog.h \
    import/csvimporter.h \
    import/csvimportconfigure.h

HEADERS  += mainwindow.h \
    loginwindow.h \
    aspectratiopixmaplabel.h \
    loggedinwidget.h \
    changemasterpassword.h \
    searchlistbox.h \
    buttonwaitdialog.h \
    searchfilteredit.h \
    systemtray.h \
    signetapplication.h \
    keygeneratorthread.h \
    resetdevice.h \
    about.h \
    keyboardlayouttester.h \
    localsettings.h \
    settingsdialog.h

HEADERS += ../qtcsv/sources/contentiterator.h \
    ../qtcsv/sources/filechecker.h \
    ../qtcsv/sources/symbols.h \
    qtsingleapplication/src/qtlocalpeer.h \
    qtsingleapplication/src/QtLockedFile \
    qtsingleapplication/src/qtsinglecoreapplication.h \
    qtsingleapplication/src/QtSingleApplication \
    qtsingleapplication/src/qtsingleapplication.h \
    qtsingleapplication/src/qtlockedfile.h \
    ../scrypt/crypto_scrypt_smix_sse2.h \
    ../scrypt/crypto_scrypt_smix.h \
    ../scrypt/crypto_scrypt.h \
    ../scrypt/insecure_memzero.h \
    ../scrypt/sha256.h \
    ../scrypt/warnp.h \
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
    ../keepassx/src/keys/PasswordKey.h

#
# Include path
#
INCLUDEPATH+=../qtcsv/include \
        ../qtcsv \
        ../scrypt \
        qtsingleapplication/src \
        ../keepassx/src \
        ../src \
        ../signet-base \
        esdb \
        esdb/account \
        esdb/bookmark \
        esdb/generic \
        esdb-gui/ \
        esdb-gui/account \
        esdb-gui/bookmark \
        esdb-gui/generic

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
