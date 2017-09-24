#include "genericfieldeditfactory.h"
#include "linefieldedit.h"
#include "integerfieldedit.h"
#include "textblockfieldedit.h"
#include "typedescedit.h"

genericFieldEditFactory *genericFieldEditFactory::s_singleton = NULL;

genericFieldEditFactory::genericFieldEditFactory()
{

}

genericFieldEdit *genericFieldEditFactory::generate(const QString &fieldName, const QString &typeName, bool canRemove)
{
	Q_UNUSED(typeName);
	if (!typeName.compare("type desc", Qt::CaseInsensitive)) {
		return new typeDescEdit(fieldName,canRemove);
	} else if (!typeName.compare("text", Qt::CaseInsensitive)) {
		return new lineFieldEdit(fieldName, canRemove);
	} else if (!typeName.compare("integer", Qt::CaseInsensitive)) {
		return new integerFieldEdit(fieldName, canRemove);
	} else if (!typeName.compare("text block", Qt::CaseInsensitive)) {
		return new textBlockFieldEdit(fieldName, canRemove);
	} else {
		return new lineFieldEdit(fieldName, canRemove);
	}
}

genericFieldEditFactory *genericFieldEditFactory::get()
{
	if (s_singleton == NULL) {
		s_singleton =  new genericFieldEditFactory();
	}
	return s_singleton;
}
