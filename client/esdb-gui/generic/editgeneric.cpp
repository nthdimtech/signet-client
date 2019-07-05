#include "editgeneric.h"
#include <QBoxLayout>
#include <QLineEdit>
#include <QLabel>
#include <QPushButton>
#include <QMessageBox>
#include <QString>
#include <QCheckBox>
#include <QDesktopServices>
#include <QCloseEvent>

#include "databasefield.h"
#include "account.h"
#include "buttonwaitdialog.h"
#include "signetapplication.h"
#include "generic.h"
#include "generictypedesc.h"
#include "genericfieldseditor.h"
#include "groupdatabasefield.h"

extern "C" {
#include "signetdev/host/signetdev.h"
}

EditGeneric::EditGeneric(generic *generic, genericTypeDesc *typeDesc, QStringList groupList, QWidget *parent) :
	EditEntryDialog(typeDesc->name, generic, parent),
	m_generic(generic),
	m_typeDesc(typeDesc),
    m_fields(generic->fields),
	m_groupList(groupList)
{
	setup(generic->name);

	bool hasNotes = false;
	for (int i = 0; i < m_fields.fieldCount(); i++) {
		if (!m_fields.getField(i).name.compare(QString("Notes"), Qt::CaseInsensitive)) {
			hasNotes = true;
			break;
		}
	}
	if (!hasNotes) {
		genericField f;
		f.name = QString("Notes");
		f.type = QString("text block");
		f.value = QString();
		m_fields.addField(f);
	}

	m_settingFields = true;
	setValues();
	m_settingFields = false;
}


EditGeneric::EditGeneric(genericTypeDesc *typeDesc, int id, QString entryName, QStringList groupList, QWidget *parent) :
	EditEntryDialog(typeDesc->name, id, parent),
	m_generic(nullptr),
	m_typeDesc(typeDesc),
	m_groupList(groupList)

{
	setup(entryName);
}

QString EditGeneric::entryName()
{
	return m_genericNameEdit->text();
}

void EditGeneric::applyChanges(esdbEntry *e)
{
	generic *g = static_cast<generic *>(e);
	g->name = entryName();
	g->fields = m_fields;
	g->typeId = m_typeDesc->typeId;
	g->path = m_groupField->text();
	m_genericFieldsEditor->saveFields(g->fields);
}

void EditGeneric::undoChanges()
{
	if (m_generic) {
		setValues();
	}
}

esdbEntry *EditGeneric::createEntry(int id)
{
	generic *g = new generic(id);
	g->typeId = m_typeDesc->typeId;
	return g;
}

void EditGeneric::setValues()
{
	m_genericNameEdit->setText(m_generic->name);
	m_genericFieldsEditor->loadFields(m_fields);
	m_groupField->setText(m_generic->path);
}

void EditGeneric::setup(QString name)
{
	m_genericNameEdit = new QLineEdit(name);
	m_genericNameEdit->setReadOnly(SignetApplication::get()->isDeviceEmulated());

	m_groupField = new GroupDatabaseField(120, m_groupList, nullptr);

	QBoxLayout *nameLayout = new QBoxLayout(QBoxLayout::LeftToRight);
	nameLayout->addWidget(new QLabel("Name"));
	nameLayout->addWidget(m_genericNameEdit);

	connect(m_genericNameEdit, SIGNAL(textEdited(QString)), this, SLOT(edited()));
	connect(m_genericNameEdit, SIGNAL(textEdited(QString)), this, SLOT(entryNameEdited()));
	connect(m_groupField, SIGNAL(textEdited(QString)), this, SLOT(edited()));

	QList<fieldSpec> requiredGenericFields = m_typeDesc->fields;

	if (!m_generic) {
		bool hasNotes = false;
		for (auto f : m_typeDesc->fields) {
			if (!f.name.compare(QString("Notes"), Qt::CaseInsensitive)) {
				hasNotes = true;
				break;
			}
		}
		if (!hasNotes) {
			requiredGenericFields.push_back(fieldSpec(QString("Notes"), QString("text block")));
		}
	}

	m_genericFieldsEditor = new GenericFieldsEditor(requiredGenericFields);
	connect(m_genericFieldsEditor, SIGNAL(edited()), this, SLOT(edited()));

	m_settingFields = true;
	m_genericFieldsEditor->loadFields(m_fields);
	m_settingFields = false;

	QBoxLayout *mainLayout = new QBoxLayout(QBoxLayout::TopToBottom);
	mainLayout->setAlignment(Qt::AlignTop);
	mainLayout->addLayout(nameLayout);
	mainLayout->addWidget(m_groupField);
	mainLayout->addWidget(m_genericFieldsEditor);
	EditEntryDialog::setup(mainLayout);
}

EditGeneric::~EditGeneric()
{
}
