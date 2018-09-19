#include "typedesceditor.h"

#include "genericfieldseditor.h"

#include "genericfieldedit.h"

#include "genericfieldeditfactory.h"

TypeDescEditor::TypeDescEditor(genericFields &fields,
				QList<fieldSpec> requiredFieldSpecs,
				QWidget *parent) :
			GenericFieldsEditor(requiredFieldSpecs, parent)
{

}

genericFieldEdit *TypeDescEditor::createFieldEdit(QString name, QString type, bool canRemove)
{
	auto *fieldEditFactory = genericFieldEditFactory::get();
	auto fieldEdit = fieldEditFactory->generate(name, "type desc", canRemove);
	fieldEdit->fromString(type);
	m_fieldEditMap.insert(name, fieldEdit);
	connect(fieldEdit, SIGNAL(edited()), this, SIGNAL(edited()));
	connect(fieldEdit, SIGNAL(remove(QString)), this, SLOT(removeField(QString)));
	return fieldEdit;
}
