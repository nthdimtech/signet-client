#include "generictypeactionbar.h"
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLineEdit>
#include <QDialog>
#include <QPushButton>

#include "editgenerictype.h"
#include "loggedinwidget.h"

void GenericTypeActionBar::selectedEntry(esdbEntry *entry)
{

}

void GenericTypeActionBar::defaultAction(esdbEntry *entry)
{

}

void GenericTypeActionBar::newInstanceUI(int id, const QString &name)
{
	EditGenericType *t = new EditGenericType(id, name, this);
	QObject::connect(t, SIGNAL(entryCreated(esdbEntry *)), this, SLOT(entryCreated(esdbEntry *)));
	t->exec();
	m_parent->finishTask(false);
	t->deleteLater();
}

GenericTypeActionBar::GenericTypeActionBar(LoggedInWidget *parent, esdbTypeModule *module, bool writeEnabled, bool typeEnabled) : m_module(static_cast<esdbGenericModule *>(module)),
      EsdbActionBar(parent, "Data type", writeEnabled, typeEnabled)
{
	QPushButton *button;
	button = addDeleteButton();
	connect(button, SIGNAL(pressed()), SLOT(deletePressed()));
}

void GenericTypeActionBar::deletePressed()
{
	deleteEntry();
}

void GenericTypeActionBar::entryCreated(esdbEntry *entry)
{
	m_parent->entryCreated(m_module->name(), entry);
}
