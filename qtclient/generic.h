#ifndef GENERIC_H
#define GENERIC_H

#include <QString>
#include <QList>
#include "esdb.h"
#include "genericfields.h"

struct block;

struct generic : public esdbEntry {
	QString typeName;
	QString name;
	genericFields fields;

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
