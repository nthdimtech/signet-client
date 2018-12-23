#include "editgenerictype.h"
#include "generic/generictypedesc.h"
#include <QPushButton>
#include <QLineEdit>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLabel>

#include "buttonwaitdialog.h"
#include "genericfieldseditor.h"

extern "C" {
#include "signetdev/host/signetdev.h"
}

QString EditGenericType::entryName()
{
	return m_typeNameEdit->text();
}

esdbEntry *EditGenericType::createEntry(int id)
{
	genericTypeDesc *g = new genericTypeDesc(id);
	return g;
}

void EditGenericType::undoChanges()
{
	if (m_genericTypeDesc) {
		m_typeNameEdit->setText(m_genericTypeDesc->name);
	}
}

void EditGenericType::copyFromGenericFields(genericTypeDesc *g, const genericFields &gf)
{
	g->fields.clear();
	for (int i = 0; i < gf.fieldCount(); i++) {
		genericField f = gf.getField(i);
		fieldSpec fs;
		fs.name = f.name;
		fs.type = f.value;
		g->fields.push_back(fs);
	}
}

void EditGenericType::copyToGenericFields(const genericTypeDesc *g, genericFields &gf)
{
	gf.clear();
	for (auto fs : g->fields) {
		genericField f;
		f.name = fs.name;
		f.type = QString("type desc");
		f.value = fs.type;
		gf.addField(f);
	}
}

void EditGenericType::applyChanges(esdbEntry *ent)
{
	genericTypeDesc *g = static_cast<genericTypeDesc *>(ent);
	g->name = m_typeNameEdit->text();
	g->typeId = m_typeId;
	genericFields gf;
	copyToGenericFields(g, gf);
	m_genericFieldsEditor->saveFields(gf);
	copyFromGenericFields(g, gf);
}

void EditGenericType::setup(QString name)
{
	QVBoxLayout *top = new QVBoxLayout();
	QHBoxLayout *typeName = new QHBoxLayout();
	m_typeNameEdit = new QLineEdit(name);
	m_genericFieldsEditor = new GenericFieldsEditor(QList<fieldSpec>(), nullptr, true);
	typeName->addWidget(new QLabel("Type name"));
	typeName->addWidget(m_typeNameEdit);
	top->addLayout(typeName);
	top->addWidget(m_genericFieldsEditor);
	EditEntryDialog::setup(top);
	connect(m_genericFieldsEditor, SIGNAL(edited()), this, SLOT(edited()));
	connect(m_typeNameEdit, SIGNAL(textEdited(QString)), this, SLOT(edited()));
	connect(m_typeNameEdit, SIGNAL(textEdited(QString)), this, SLOT(entryNameEdited()));
}

EditGenericType::EditGenericType(int id, u16 typeId, const QString &name, QWidget *parent) :
	EditEntryDialog(QString("Data type"), id, parent),
	m_genericTypeDesc(nullptr),
	m_genericFieldsEditor(nullptr),
	m_typeId(typeId)

{
	setup(name);
}

EditGenericType::EditGenericType(genericTypeDesc *g, QWidget *parent) :
	EditEntryDialog(g->name, g, parent),
	m_genericTypeDesc(nullptr),
	m_genericFieldsEditor(nullptr),
	m_typeId(g->typeId)
{
	setup(g->name);
	genericFields gf;
	copyToGenericFields(g, gf);
	m_genericFieldsEditor->loadFields(gf);
}
