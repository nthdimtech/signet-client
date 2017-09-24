#ifndef GENERICFIELDEDITFACTORY_H
#define GENERICFIELDEDITFACTORY_H

#include <QString>

class genericFieldEdit;

class genericFieldEditFactory
{
	static genericFieldEditFactory *s_singleton;
public:
	genericFieldEditFactory();
	genericFieldEdit *generate(const QString &fieldName, const QString &typeName,
				   bool canRemove = true);
	static genericFieldEditFactory *get();
};

#endif // GENERICFIELDEDITFACTORY_H
