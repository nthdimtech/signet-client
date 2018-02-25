#include "genericfieldseditor.h"

#include "genericfieldedit.h"

#include "genericfieldeditfactory.h"

#include <QHBoxLayout>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QFrame>
#include <QComboBox>

GenericFieldsEditor::GenericFieldsEditor(genericFields &fields,
					 QList<fieldSpec> requiredFieldSpecs,
					 QWidget *parent) :
	QWidget(parent),
	m_fields(fields),
	m_requiredFieldSpecs(requiredFieldSpecs)
{
	setLayout(new QVBoxLayout());
	layout()->setAlignment(Qt::AlignTop);
	layout()->setContentsMargins(0,0,0,0);
	requiredFieldsWidget = new QWidget();
	extraFieldsWidget = new QWidget();
	newFeild = new QWidget();
	newFeildAddButton = new QPushButton(QIcon(":/images/plus.png"),"");
	newFeildNameEdit = new QLineEdit();

	layout()->addWidget(requiredFieldsWidget);
	QFrame *f = new QFrame();
	f->setFrameShape(QFrame::HLine);
	layout()->addWidget(f);
	layout()->addWidget(extraFieldsWidget);

	newFieldTypeCombo = new QComboBox();
	newFieldTypeCombo->addItem("Text");
	newFieldTypeCombo->addItem("Text block");
	newFieldTypeCombo->addItem("Integer");

	requiredFieldsWidget->setLayout(new QVBoxLayout());
	requiredFieldsWidget->layout()->setAlignment(Qt::AlignTop);
	requiredFieldsWidget->layout()->setContentsMargins(0,0,0,0);
	extraFieldsWidget->setLayout(new QVBoxLayout());
	extraFieldsWidget->layout()->setAlignment(Qt::AlignTop);
	extraFieldsWidget->layout()->setContentsMargins(0,0,0,0);
	newFeild->setLayout(new QHBoxLayout());
	newFeild->layout()->setContentsMargins(0,0,0,0);
	newFeild->layout()->addWidget(newFeildNameEdit);
	newFeild->layout()->addWidget(newFieldTypeCombo);
	newFeild->layout()->addWidget(newFeildAddButton);
	connect(newFeildAddButton, SIGNAL(pressed()),  this, SLOT(addNewFieldUI()));
	connect(newFeildNameEdit,  SIGNAL(textEdited(QString)), this, SLOT(newFieldNameEdited(QString)));

	for (auto requiredFieldSpec : m_requiredFieldSpecs) {
		auto fieldEdit = createFieldEdit(requiredFieldSpec.name, requiredFieldSpec.type, false);
		requiredFieldsWidget->layout()->addWidget(fieldEdit->widget());
	}
	extraFieldsWidget->layout()->addWidget(newFeild);
}

void GenericFieldsEditor::removeField(QString name)
{
	genericFieldEdit *fieldEdit = *m_fieldEditMap.find(name);
	m_fieldEditMap.remove(name);
	m_extraFields.removeAll(fieldEdit);
	extraFieldsWidget->layout()->removeWidget(fieldEdit->widget());
	fieldEdit->widget()->deleteLater();
	emit edited();
}

void GenericFieldsEditor::newFieldNameEdited(QString val)
{
	Q_UNUSED(val);
}

genericFieldEdit *GenericFieldsEditor::addNewField(QString name, QString type)
{
	if (!type.size()) {
		type = QString("text");
	}
	auto fieldEdit = createFieldEdit(name, type, true);
	QHBoxLayout *layout = ((QHBoxLayout *)(extraFieldsWidget->layout()));
	layout->insertWidget(layout->count() - 1, fieldEdit->widget());
	return fieldEdit;
}

genericFieldEdit *GenericFieldsEditor::createFieldEdit(QString name, QString type, bool canRemove)
{
	auto *fieldEditFactory = genericFieldEditFactory::get();
	auto fieldEdit = fieldEditFactory->generate(name, type, canRemove);
	m_fieldEditMap.insert(name, fieldEdit);
	connect(fieldEdit, SIGNAL(edited()), this, SIGNAL(edited()));
	connect(fieldEdit, SIGNAL(remove(QString)), this, SLOT(removeField(QString)));
	return fieldEdit;
}

void GenericFieldsEditor::addNewFieldUI()
{
	QString fieldName = newFeildNameEdit->text();
	if (fieldName.size()) {
		newFeildNameEdit->clear();
		addNewField(fieldName, newFieldTypeCombo->currentText());
		emit edited();
	}
}

void GenericFieldsEditor::loadFields()
{
	for (int i = 0; i < m_fields.fieldCount(); i++) {
		auto genericField = m_fields.getField(i);
		if (m_fieldEditMap.count(genericField.name)) {
			auto genericFieldEdit = *m_fieldEditMap.find(genericField.name);
			genericFieldEdit->fromString(genericField.value);
		} else {
			auto fieldEdit = addNewField(genericField.name, genericField.type);
			fieldEdit->fromString(genericField.value);
		}
	}
}

void GenericFieldsEditor::saveFields()
{
	int i = 0;
	//Update and delete fields
	while (i < m_fields.fieldCount()) {
		auto field = m_fields.getField(i);
		if (m_fieldEditMap.count(field.name)) {
			auto fieldEdit = *(m_fieldEditMap.find(field.name));
			field.value = fieldEdit->toString();
			field.type = fieldEdit->type();
			m_fields.replaceField(i, field);
			i++;
		} else {
			m_fields.removeField(i);
		}
	}

	//Add new fields
	for (auto fieldEdit : m_fieldEditMap) {
		int i;
		for (i = 0; i < m_fields.fieldCount(); i++) {
			auto field = m_fields.getField(i);
			if (fieldEdit->name() == field.name) {
				break;
			}
		}
		if (i == m_fields.fieldCount()) {
			genericField feild;
			feild.name = fieldEdit->name();
			feild.type = fieldEdit->type();
			feild.value = fieldEdit->toString();
			m_fields.addField(feild);
		}
	}
}
