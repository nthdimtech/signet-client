#include "genericfieldeditfactory.h"
#include "linefieldedit.h"
#include "integerfieldedit.h"
#include "textblockfieldedit.h"
#include "typedescedit.h"

genericFieldEditFactory *genericFieldEditFactory::s_singleton = NULL;

genericFieldEditFactory::genericFieldEditFactory()
{

}

genericFieldEdit *genericFieldEditFactory::generate(const QString &fieldName, const QString &typeName, bool canRemove, QWidget *parent)
{
	Q_UNUSED(typeName);
	if (!typeName.compare("type desc", Qt::CaseInsensitive)) {
		return new typeDescEdit(fieldName,canRemove, parent);
	} else if (!typeName.compare("text", Qt::CaseInsensitive)) {
		return new lineFieldEdit(fieldName, canRemove, parent);
	} else if (!typeName.compare("integer", Qt::CaseInsensitive)) {
		return new integerFieldEdit(fieldName, canRemove, parent);
	} else if (!typeName.compare("text block", Qt::CaseInsensitive)) {
		return new textBlockFieldEdit(fieldName, canRemove, parent);
	} else {
		return new lineFieldEdit(fieldName, canRemove, parent);
	}
}

genericFieldEditFactory *genericFieldEditFactory::get()
{
	if (s_singleton == nullptr) {
		s_singleton =  new genericFieldEditFactory();
	}
	return s_singleton;
}
