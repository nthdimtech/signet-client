#ifndef TYPEDESCEDITOR_H
#define TYPEDESCEDITOR_H

#include "genericfieldseditor.h"

#include <QObject>

class TypeDescEditor : public GenericFieldsEditor
{
	genericFieldEdit *createFieldEdit(QString name, QString type, bool canRemove);
public:
	TypeDescEditor(genericFields &fields,
			QList<fieldSpec> requiredFieldSpecs,
			QWidget *parent = NULL);
};

#endif // TYPEDESCEDITOR_H
