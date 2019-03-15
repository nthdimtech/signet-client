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
#include "generictypeactionbar.h"
#include <QMessageBox>

void GenericTypeActionBar::defaultAction(esdbEntry *entry)
{
	Q_UNUSED(entry);
}

void GenericTypeActionBar::newInstanceUI(int id, const QString &name)
{
	int typeId = m_parent->getUnusedTypeId();
	if (typeId >= 0) {
		EditGenericType *t = new EditGenericType(id, static_cast<u16>(typeId), name, this);
		connect(t, SIGNAL(entryCreated(esdbEntry *)), this, SLOT(entryCreated(esdbEntry *)));
		connect(t, SIGNAL(finished(int)), this, SLOT(editFinished()));
		t->setAttribute(Qt::WA_DeleteOnClose);
		t->show();
	} else {
		//TODO: handle error
	}
}

void GenericTypeActionBar::editFinished()
{
	m_parent->finishTask(false);
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
	auto entry = selectedEntry();
	if (!entry)
		return;

	auto entryMap = m_parent->typeNameToEntryMap(entry->getTitle());
	if (!entryMap)
		return;

	int numEntriesOfType = entryMap->size();
	if (numEntriesOfType > 0) {
		QMessageBox *box = new QMessageBox(QMessageBox::Warning,
		                                   "Delete datatype",
		                                   "There are still entries with this data type. If you delete this data type these entries will move to the 'Misc' group.\n\n Delete this data type?",
		                                   QMessageBox::Yes | QMessageBox::No,
		                                   this);
		box->setWindowModality(Qt::WindowModal);
		box->setAttribute(Qt::WA_DeleteOnClose);
		box->show();
	} else {
		deleteEntry();
	}
}

void GenericTypeActionBar::deleteConfirmDialogFinished(int rc)
{
	if (rc == QMessageBox::Yes) {
		deleteEntry();
	}
}

void GenericTypeActionBar::deleteEntryComplete(esdbEntry *entry)
{
	Q_UNUSED(entry);
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
	}
	break;
	}
}
