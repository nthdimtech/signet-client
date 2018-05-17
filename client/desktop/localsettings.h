#ifndef LOCALSETTINGS_H
#define LOCALSETTINGS_H

#include <QVector>
#include <QMap>
#include <QString>
#include <QDateTime>

extern "C" {
#include "signetdev/host/signetdev.h"
}

typedef QVector<struct signetdev_key> keyboardLayout;

struct localSettings {
	bool localBackups;
	QString localBackupPath;
	int localBackupInterval;
	bool removableBackups;
	QString removableBackupPath;
	QString removableBackupVolume;
	int removableBackupInterval;
	QDateTime lastRemoveableBackup;
	QDateTime lastUpdatePrompt;
	QMap<QString, keyboardLayout> keyboardLayouts;
	QString activeKeyboardLayout;
	QByteArray windowGeometry;
	bool minimizeToTray;
};

#endif // LOCALSETTINGS_H
