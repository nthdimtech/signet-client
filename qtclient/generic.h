#ifndef GENERIC_H
#define GENERIC_H

#include <QString>
#include <QList>
#include "esdb.h"

struct block;

struct genericField {
	QString name;
	QString value;
};

struct generic : public esdbEntry {
	QString typeName;
	QString name;
	QList<genericField> fields;

	void fromBlock(block *blk);
	void toBlock(block *blk);
	generic(int id_) : esdbEntry(id_, ESDB_TYPE_GENERIC, 0, id_, 1)
	{
	}

	QString getTitle()
	{
		return name;
	}

	QString getUrl()
	{
		return QString();
	}

	int matchQuality(const QString &search);

	~generic() {}
};

#endif // GENERIC_H
