#ifndef GENERICTYPEDESC_H
#define GENERICTYPEDESC_H

#include "esdb.h"

struct fieldSpec {
	QString name;
	QString type;
	fieldSpec(QString name_, QString type_) :
		name(name_),
		type(type_)
	{

	}
	fieldSpec() {}
};

struct genericTypeDesc : public esdbEntry {
	QString name;
	QList<fieldSpec> fields;

	void fromBlock(block *blk);
	void toBlock(block *blk);
	genericTypeDesc(int id_) : esdbEntry(id_, ESDB_TYPE_GENERIC_TYPE_DESC, 0, id_, 1)
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

	~genericTypeDesc() {}
};

#endif // GENERICTYPEDESC_H
