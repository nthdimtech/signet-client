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

struct genericTypeDesc {
	QString name;
	QList<fieldSpec> fields;
};

#endif // GENERICTYPEDESC_H
