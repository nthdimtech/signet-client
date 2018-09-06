#include "generictypeactionbar.h"
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLineEdit>
#include <QDialog>
#include <QPushButton>

#include "newgenerictype.h"
#include "loggedinwidget.h"

void GenericTypeActionBar::selectedEntry(esdbEntry *entry)
{

}

void GenericTypeActionBar::defaultAction(esdbEntry *entry)
{

}

void GenericTypeActionBar::newInstanceUI(int id, const QString &name)
{
	NewGenericType *t = new NewGenericType(id, name, this);
	t->exec();
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
