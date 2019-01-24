#include "genericfieldseditor.h"

#include "genericfieldedit.h"

#include "genericfieldeditfactory.h"

#include <QHBoxLayout>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QFrame>
#include <QComboBox>

GenericFieldsEditor::GenericFieldsEditor(QList<fieldSpec> requiredFieldSpecs,
					 QWidget *parent, bool descEdit) :
	QWidget(parent),
	m_requiredFieldSpecs(requiredFieldSpecs),
	m_descEdit(descEdit)
{
	setLayout(new QVBoxLayout());
	layout()->setAlignment(Qt::AlignTop);
	layout()->setContentsMargins(0,0,0,0);
	m_requiredFieldsWidget = new QWidget();
	m_extraFieldsWidget = new QWidget();
	m_newField = new QDialog();
	m_newFieldAddButton = new QPushButton(QIcon(":/images/plus.png"),"");
	QLayout *newFieldDescLayout = new QHBoxLayout();
	newFieldDescLayout->addWidget(new QLabel("Field Name"));
	m_newFieldNameEdit = new QLineEdit();
	newFieldDescLayout->addWidget(m_newFieldNameEdit);

	connect(m_newFieldNameEdit, SIGNAL(returnPressed()), m_newFieldAddButton, SLOT(click()));

	layout()->addWidget(m_requiredFieldsWidget);
	m_fieldFrame = new QFrame();
	m_fieldFrame->setFrameShape(QFrame::HLine);
	m_newFieldFrame = new QFrame();
	m_newFieldFrame->setFrameShape(QFrame::HLine);
	layout()->addWidget(m_fieldFrame);
	layout()->addWidget(m_extraFieldsWidget);
	layout()->addWidget(m_newFieldFrame);
	layout()->addWidget(m_newField);

	m_newFieldTypeCombo = new QComboBox();
	m_newFieldTypeCombo->addItem("Text", "Text");
	m_newFieldTypeCombo->addItem("Text (Secret)", ".Text");
	m_newFieldTypeCombo->addItem("Text block", "Text block");
	m_newFieldTypeCombo->addItem("Integer", "Integer");

	m_requiredFieldsWidget->setLayout(new QVBoxLayout());
	m_requiredFieldsWidget->layout()->setAlignment(Qt::AlignTop);
	m_requiredFieldsWidget->layout()->setContentsMargins(0,0,0,0);
	m_extraFieldsWidget->setLayout(new QVBoxLayout());
	m_extraFieldsWidget->layout()->setAlignment(Qt::AlignTop);
	m_extraFieldsWidget->layout()->setContentsMargins(0,0,0,0);

	newFieldDescLayout->addWidget(m_newFieldTypeCombo);
	newFieldDescLayout->addWidget(m_newFieldAddButton);
	QLabel *newFieldLabel = new QLabel("New Field");
	newFieldLabel->setStyleSheet("font-weight: bold");
	QVBoxLayout *newFieldLayout = new QVBoxLayout();
	m_newField->setLayout(newFieldLayout);
	newFieldLayout->setContentsMargins(0,0,0,0);
	newFieldLayout->addWidget(newFieldLabel);
	newFieldLayout->addLayout(newFieldDescLayout);
	connect(m_newFieldAddButton, SIGNAL(pressed()),  this, SLOT(addNewFieldUI()));
	connect(m_newFieldNameEdit,  SIGNAL(textEdited(QString)), this, SLOT(newFieldNameEdited(QString)));

	m_fieldFrame->hide();

	for (auto requiredFieldSpec : m_requiredFieldSpecs) {
		auto fieldEdit = createFieldEdit(requiredFieldSpec.name, requiredFieldSpec.type, false);
		m_requiredFieldsWidget->layout()->addWidget(fieldEdit->widget());
	}
}

void GenericFieldsEditor::removeField(QString name)
{
	genericFieldEdit *fieldEdit = *m_fieldEditMap.find(name);
	m_fieldEditMap.remove(name);
	m_extraFields.remove(name);
	m_extraFieldsWidget->layout()->removeWidget(fieldEdit->widget());
	if  (!m_extraFields.size()) {
		m_fieldFrame->hide();
	}
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
	QHBoxLayout *layout = ((QHBoxLayout *)(m_extraFieldsWidget->layout()));
	layout->insertWidget(layout->count(), fieldEdit->widget());
	m_fieldFrame->show();
	fieldEdit->setFocus();
	return fieldEdit;
}

genericFieldEdit *GenericFieldsEditor::createFieldEdit(QString name, QString type, bool canRemove)
{
	auto *fieldEditFactory = genericFieldEditFactory::get();
	genericFieldEdit *fieldEdit = NULL;
	if (m_descEdit) {
		fieldEdit = fieldEditFactory->generate(name, QString("type desc"), canRemove);
	} else {
		fieldEdit = fieldEditFactory->generate(name, type, canRemove);
	}
	m_fieldEditMap.insert(name, fieldEdit);
	if (canRemove) {
		m_extraFields.insert(name, fieldEdit);
	}
	connect(fieldEdit, SIGNAL(edited()), this, SIGNAL(edited()));
	connect(fieldEdit, SIGNAL(remove(QString)), this, SLOT(removeField(QString)));
	return fieldEdit;
}

void GenericFieldsEditor::addNewFieldUI()
{
	QString fieldName = m_newFieldNameEdit->text();
	if (fieldName.size()) {
		m_newFieldNameEdit->clear();
		QString dataType = m_newFieldTypeCombo->currentData().toString();
		if (dataType[0] == '.') {
			dataType.remove(0, 1);
			fieldName.prepend('.');
		}
		addNewField(fieldName, dataType);
		emit edited();
	}
}

void GenericFieldsEditor::loadFields(genericFields &fields)
{
	for (int i = 0; i < fields.fieldCount(); i++) {
		auto genericField = fields.getField(i);
		if (m_fieldEditMap.count(genericField.name)) {
			auto genericFieldEdit = *m_fieldEditMap.find(genericField.name);
			genericFieldEdit->fromString(genericField.value);
		} else {
			auto fieldEdit = addNewField(genericField.name, genericField.type);
			fieldEdit->fromString(genericField.value);
		}
	}
}

void GenericFieldsEditor::saveFields(genericFields &fields)
{
	int i = 0;
	//Update and delete fields
	while (i < fields.fieldCount()) {
		auto field = fields.getField(i);
		if (m_fieldEditMap.count(field.name)) {
			auto fieldEdit = *(m_fieldEditMap.find(field.name));
			field.value = fieldEdit->toString();
			field.type = fieldEdit->type();
			fields.replaceField(i, field);
			i++;
		} else {
			fields.removeField(i);
		}
	}

	//Add new fields
	for (auto fieldEdit : m_fieldEditMap) {
		int i;
		for (i = 0; i < fields.fieldCount(); i++) {
			auto field = fields.getField(i);
			if (fieldEdit->name() == field.name) {
				break;
			}
		}
		if (i == fields.fieldCount()) {
			genericField feild;
			feild.name = fieldEdit->name();
			feild.type = fieldEdit->type();
			feild.value = fieldEdit->toString();
			fields.addField(feild);
		}
	}
}
