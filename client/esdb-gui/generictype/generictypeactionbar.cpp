#include "generictypeactionbar.h"
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLineEdit>
#include <QDialog>
#include <QPushButton>

#include "editgenerictype.h"
#include "loggedinwidget.h"
#include "generictypedesc.h"
#include "buttonwaitdialog.h"

void GenericTypeActionBar::selectedEntry(esdbEntry *entry)
{
	Q_UNUSED(entry);
}

void GenericTypeActionBar::defaultAction(esdbEntry *entry)
{
	Q_UNUSED(entry);
}

void GenericTypeActionBar::newInstanceUI(int id, const QString &name)
{
	int typeId = m_parent->getUnusedTypeId();
	if (typeId >= 0) {
		EditGenericType *t = new EditGenericType(id, typeId, name, this);
		QObject::connect(t, SIGNAL(entryCreated(esdbEntry *)), this, SLOT(entryCreated(esdbEntry *)));
		t->exec();
		m_parent->finishTask(false);
		t->deleteLater();
	} else {
		//TODO: handle error
	}
}

GenericTypeActionBar::GenericTypeActionBar(LoggedInWidget *parent, esdbTypeModule *module, bool writeEnabled, bool typeEnabled) :
	EsdbActionBar(parent, "Data type", writeEnabled, typeEnabled),
	m_module(static_cast<esdbGenericModule *>(module))
{
	QPushButton *button;
	button = addDeleteButton();
	connect(button, SIGNAL(pressed()), SLOT(deletePressed()));

	button = addOpenButton();
	connect(button, SIGNAL(pressed()), this, SLOT(openEntryUI()));
}

void GenericTypeActionBar::deletePressed()
{
	deleteEntry();
}

void GenericTypeActionBar::entryCreated(esdbEntry *entry)
{
	m_parent->entryCreated(m_module->name(), entry);
}

void GenericTypeActionBar::openEntryUI()
{
	openEntry(EsdbActionBar::selectedEntry());
}

void GenericTypeActionBar::accessEntryComplete(esdbEntry *entry, int intent)
{
	switch (intent) {
	case INTENT_OPEN_ENTRY: {
		genericTypeDesc *g = static_cast<genericTypeDesc *>(entry);
		if (m_buttonWaitDialog) {
			m_buttonWaitDialog->done(QMessageBox::Ok);
		}
		EditGenericType *ogt = new EditGenericType(g, m_parent);
		connect(ogt, SIGNAL(abort()), this, SIGNAL(abort()));
		connect(ogt, SIGNAL(entryChanged(int)), m_parent, SLOT(entryChanged(int)));
		connect(ogt, SIGNAL(finished(int)), ogt, SLOT(deleteLater()));
		ogt->show();
	} break;
	}
}
