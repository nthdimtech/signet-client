#ifndef ESDBTYPEDATA_H
#define ESDBTYPEDATA_H

#include <QVariant>
#include <QObject>
#include <QString>
#include <QList>
#include <map>
#include <QVector>

class LoggedInWidget;
struct esdbEntry;
class EsdbModel;
class QPushButton;
class EsdbActionBar;
struct block;

#include "genericfields.h"

struct esdbTypeModule {
	QString m_name;
protected:
	QVector<QStringList::const_iterator> aliasMatch(const QVector<QStringList> &aliasedFields, const QStringList &fields) const;
	QVector<QString> aliasMatchValues(const QVector<QStringList> &aliasedFields, const QVector<QStringList::const_iterator> &aliasMatched, const QVector<genericField> &fields, genericFields *genFields) const;
public:
	virtual EsdbActionBar *newActionBar();
	virtual esdbEntry *decodeEntry(int id, int revision, esdbEntry *prev, struct block *blk) const = 0;

	virtual esdbEntry *decodeEntry(const QVector<genericField> &fields, bool aliasMatch = true) const {
		Q_UNUSED(fields);
		Q_UNUSED(aliasMatch);
		return NULL;
	}

	esdbTypeModule(const QString &name);
	virtual ~esdbTypeModule()
	{
	}

	QString name() const
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
