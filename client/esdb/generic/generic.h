#ifndef GENERIC_H
#define GENERIC_H

#include <QString>
#include <QList>
#include "esdb.h"
#include "genericfields.h"

struct block;

struct generic_1 : public esdbEntry {
	QString typeName;
	QString name;
	genericFields_1 fields;

	void fromBlock(block *blk);
	generic_1(int id_) : esdbEntry(id_, ESDB_TYPE_GENERIC, 0, id_, 1) {}
	~generic_1() {}

};

struct generic_2 : public esdbEntry {
	QString typeName;
	QString name;
	genericFields_2 fields;

	void fromBlock(block *blk);
	generic_2(int id_) : esdbEntry(id_, ESDB_TYPE_GENERIC, 1, id_, 1) {}
	~generic_2() {}
	void upgrade(const generic_1 &g)
	{
		typeName = g.typeName;
		name = g.name;
		fields.upgrade(g.fields);
	}
};

struct generic_3 : public esdbEntry {
	QString typeName;
	QString name;
	genericFields fields;

	generic_3(int id_) : esdbEntry(id_, ESDB_TYPE_GENERIC, 2, id_, 1) {}
	~generic_3() {}
	void fromBlock(block *blk);

	void upgrade(const generic_2 &g)
	{
		typeName = g.typeName;
		name = g.name;
		fields.upgrade(g.fields);
	}
};

struct generic : public esdbEntry {
	QString name;
	genericFields fields;
	u16 typeId;

	void fromBlock(block *blk);
	void toBlock(block *blk) const;
	generic(int id_) : esdbEntry(id_, ESDB_TYPE_GENERIC, 3, id_, 1) { }

	QString getTitle() const {
		return name;
	}
	void setTitle(const QString &title) {
		name = title;
	}
	QString getUrl() const {
		return QString();
	}

	int matchQuality(const QString &search) const;

	void upgrade(const generic_3 &g)
	{
		typeId = 0;
		name = g.name;
		fields = g.fields;
	}

	void getFields(QVector<genericField> &fields_) const;

	~generic() {}
};

#endif // GENERIC_H
