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
#include "typedesceditor.h"

extern "C" {
#include "signetdev/host/signetdev.h"
}


EditGeneric::EditGeneric(generic *generic, genericTypeDesc *typeDesc, QWidget *parent) :
	EditEntryDialog(typeDesc->name, generic, parent),
	m_generic(generic),
	m_typeDesc(typeDesc),
	m_fields(generic->fields)
{
	setup(generic->name);
	m_settingFields = true;
	m_genericFieldsEditor->loadFields(m_fields);
	m_settingFields = false;
}


EditGeneric::EditGeneric(genericTypeDesc *typeDesc, int id, QString entryName, QWidget *parent) :
	EditEntryDialog(typeDesc->name, id, parent),
	m_generic(NULL),
	m_typeDesc(typeDesc)

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
	m_genericFieldsEditor->saveFields(g->fields);
}

void EditGeneric::undoChanges()
{
	if (m_generic) {
		m_genericNameEdit->setText(m_generic->name);
		m_genericFieldsEditor->loadFields(m_fields);
	}
}

esdbEntry *EditGeneric::createEntry(int id)
{
	generic *g = new generic(id);
	g->typeName = m_typeDesc->name;
	return g;
}

void EditGeneric::setup(QString name)
{
	m_genericNameEdit = new QLineEdit(name);

	QBoxLayout *nameLayout = new QBoxLayout(QBoxLayout::LeftToRight);
	nameLayout->addWidget(new QLabel("Name"));
	nameLayout->addWidget(m_genericNameEdit);

	connect(m_genericNameEdit, SIGNAL(textEdited(QString)), this, SLOT(edited()));
	connect(m_genericNameEdit, SIGNAL(textEdited(QString)), this, SLOT(entryNameEdited()));

	m_genericFieldsEditor = new GenericFieldsEditor(m_typeDesc->fields);
	connect(m_genericFieldsEditor, SIGNAL(edited()), this, SLOT(edited()));

	m_settingFields = true;
	m_genericFieldsEditor->loadFields(m_fields);
	m_settingFields = false;

	QBoxLayout *mainLayout = new QBoxLayout(QBoxLayout::TopToBottom);
	mainLayout->setAlignment(Qt::AlignTop);
	mainLayout->addLayout(nameLayout);
	mainLayout->addWidget(m_genericFieldsEditor);
	EditEntryDialog::setup(mainLayout);
}

EditGeneric::~EditGeneric()
{
}
