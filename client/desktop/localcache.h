#ifndef LOCALCACHE_H
#define LOCALCACHE_H

#include <QByteArray>
#include <QDateTime>

struct localCache {
	QDateTime lastRemoveableBackup;
	QDateTime lastUpdatePrompt;
	QByteArray windowGeometry;
};

#endif // LOCALCACHE_H
