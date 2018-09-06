#include "newgenerictype.h"
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

QString NewGenericType::entryName()
{
	return m_typeNameEdit->text();
}

esdbEntry *NewGenericType::createEntry(int id)
{
	genericTypeDesc *g = new genericTypeDesc(m_id);
	return g;
}

void NewGenericType::applyChanges(esdbEntry *ent)
{
	genericTypeDesc *g = (genericTypeDesc *)ent;
	g->name = m_typeNameEdit->text();
}

NewGenericType::NewGenericType(int id, const QString &name, QWidget *parent) :
	EditEntryDialog(QString("Data type"), id, parent)
{
	QVBoxLayout *top = new QVBoxLayout();
	QHBoxLayout *typeName = new QHBoxLayout();
	m_typeNameEdit = new QLineEdit(name);
	typeName->addWidget(new QLabel("Type name"));
	typeName->addWidget(m_typeNameEdit);
	top->addLayout(typeName);
	setup(top);
}
