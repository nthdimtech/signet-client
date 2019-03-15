#ifndef BOOKMARK_H
#define BOOKMARK_H

#include <QString>
#include <QIcon>
#include "esdb.h"

struct bookmark : public esdbEntry {
	QString name;
	QString url;
	void fromBlock(block *blk);
	void toBlock(block *blk) const;
	bookmark(int id_) : esdbEntry(id_, ESDB_TYPE_BOOKMARK, 0, id_, 1)
	{
	}

	void setTitle(const QString &title)
	{
		name = title;
	}

	QString getTitle() const
	{
		return name;
	}

	QString getUrl() const
	{
		return url;
	}

	int matchQuality(const QString &search) const;

	~bookmark() {}
};

#endif // BOOKMARK_H
