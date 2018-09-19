#include "editgenerictype.h"
#include "generic/generictypedesc.h"
#include <QPushButton>
#include <QLineEdit>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLabel>

#include "buttonwaitdialog.h"

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

void EditGenericType::applyChanges(esdbEntry *ent)
{
	genericTypeDesc *g = (genericTypeDesc *)ent;
	g->name = m_typeNameEdit->text();
}

EditGenericType::EditGenericType(int id, const QString &name, QWidget *parent) :
	EditEntryDialog(QString("Data type"), id, parent),
	m_genericTypeDesc(NULL)
{
	QVBoxLayout *top = new QVBoxLayout();
	QHBoxLayout *typeName = new QHBoxLayout();
	m_typeNameEdit = new QLineEdit(name);
	typeName->addWidget(new QLabel("Type name"));
	typeName->addWidget(m_typeNameEdit);
	top->addLayout(typeName);
	setup(top);
	connect(m_typeNameEdit, SIGNAL(textEdited(QString)), this, SLOT(edited()));
	connect(m_typeNameEdit, SIGNAL(textEdited(QString)), this, SLOT(entryNameEdited()));

}
