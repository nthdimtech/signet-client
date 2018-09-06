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
	QString group;
	QList<fieldSpec> fields;
	void fromBlock(block *blk);
	void toBlock(block *blk) const;
	QString getTitle() const {
		return name;
	}
	QString getPath() const {
		return group;
	}
	genericTypeDesc(int id) : esdbEntry(id, ESDB_TYPE_GENERIC_TYPE_DESC, 1, id, 1) {

	}
	void getFields(QVector<genericField> &fields_) const;
};

#endif // GENERICTYPEDESC_H
