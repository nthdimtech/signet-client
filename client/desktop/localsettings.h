#ifndef LOCALSETTINGS_H
#define LOCALSETTINGS_H

#include <QVector>
#include <QMap>
#include <QString>

extern "C" {
#include "signetdev/host/signetdev.h"
}

typedef QVector<struct signetdev_key> keyboardLayout;

struct localSettings {
    bool browserPluginSupport;
	bool localBackups;
	QString localBackupPath;
	int localBackupInterval;
	bool removableBackups;
	QString removableBackupPath;
	QString removableBackupVolume;
	int removableBackupInterval;
	QMap<QString, keyboardLayout> keyboardLayouts;
	QString activeKeyboardLayout;
	bool minimizeToTray;
};

#endif // LOCALSETTINGS_H
