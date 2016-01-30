#ifndef ESDBTYPEDATA_H
#define ESDBTYPEDATA_H

#include <QVariant>
#include <QObject>
#include <QString>
#include <QList>
#include <map>

class LoggedInWidget;
struct esdbEntry;
class EsdbModel;
class QPushButton;
class EsdbActionBar;
struct block;

struct esdbTypeModule {
	QString m_name;
public:
	virtual EsdbActionBar *newActionBar();
	virtual esdbEntry *decodeEntry(int id, int revision, esdbEntry *prev, struct block *blk) const = 0;

	esdbTypeModule(const QString &name);

	QString name()
	{
		return m_name;
	}

	virtual bool hasUrl()
	{
		return true;
	}

	QString getGroup(esdbEntry *)
	{
		return QString();
	}
};

#endif // ESDBTYPEDATA_H
