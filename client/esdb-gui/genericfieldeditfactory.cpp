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
	QString typeName_ = typeName;
	bool secretField = false;
	if (typeName_[0] == '.') {
		typeName_.remove(0,1);
		secretField = true;
	}
	if (!typeName_.compare("type desc", Qt::CaseInsensitive)) {
		return new typeDescEdit(fieldName, canRemove, secretField, parent);
	} else if (!typeName_.compare("text", Qt::CaseInsensitive)) {
		return new lineFieldEdit(fieldName, canRemove, secretField, parent);
	} else if (!typeName_.compare("integer", Qt::CaseInsensitive)) {
		return new integerFieldEdit(fieldName, canRemove, secretField, parent);
	} else if (!typeName_.compare("text block", Qt::CaseInsensitive)) {
		return new textBlockFieldEdit(fieldName, canRemove, secretField, parent);
	} else {
		return new lineFieldEdit(fieldName, canRemove, secretField, parent);
	}
}

genericFieldEditFactory *genericFieldEditFactory::get()
{
	if (s_singleton == nullptr) {
		s_singleton =  new genericFieldEditFactory();
	}
	return s_singleton;
}
