#include "bookmarkactionbar.h"
#include "esdb.h"

#include <QDesktopServices>
#include <QUrl>
#include <QPushButton>
#include <QHBoxLayout>

#include "esdbtypemodule.h"
#include "loggedinwidget.h"
#include "buttonwaitdialog.h"
#include "genericactionbar.h"
#include "editgeneric.h"
#include "generic.h"
#include "generictypedesc.h"

GenericActionBar::GenericActionBar(LoggedInWidget *parent, esdbTypeModule *module, genericTypeDesc *typeDesc, bool writeEnabled, bool typeEnabled) :
	EsdbActionBar(parent, typeDesc->name, writeEnabled, typeEnabled),
	m_module(module),
	m_typeDesc(typeDesc),
	m_newEntryDlg(nullptr),
	m_browseButton(nullptr)
{
	if (module->hasUrl()) {
		m_browseButton = addBrowseButton();
		connect(m_browseButton, SIGNAL(pressed()), this, SLOT(browseUrlUI()));
	}
	QPushButton *button;
	button = addOpenButton();
	connect(button, SIGNAL(pressed()), this, SLOT(openEntryUI()));

	button = addDeleteButton();
	connect(button, SIGNAL(pressed()), this, SLOT(deleteEntryUI()));
}

void GenericActionBar::entrySelected(esdbEntry *entry)
{
	if (m_browseButton) {
		QUrl url(entry->getUrl());
		m_browseButton->setEnabled(entry && url.isValid() && !url.isEmpty());
	}
}

void GenericActionBar::defaultAction(esdbEntry *entry)
{
	openEntry(entry);
}

void GenericActionBar::browseUrlUI()
{
	browseUrl(selectedEntry());
}

void GenericActionBar::newInstanceUI(int id, const QString &name)
{
	m_newEntryDlg = new EditGeneric(m_typeDesc, id, name, this);
	QObject::connect(m_newEntryDlg, SIGNAL(entryCreated(esdbEntry *)), this, SLOT(entryCreated(esdbEntry *)));
	QObject::connect(m_newEntryDlg, SIGNAL(finished(int)), this, SLOT(newEntryFinished(int)));
	m_newEntryDlg->show();
}

void GenericActionBar::openEntryUI()
{
	openEntry(selectedEntry());
}

void GenericActionBar::deleteEntryUI()
{
	deleteEntry();
}

void GenericActionBar::accessEntryComplete(esdbEntry *entry, int intent)
{
	switch (intent) {
	case INTENT_OPEN_ENTRY: {
		generic *g = static_cast<generic *>(entry);
		if (m_buttonWaitDialog) {
			m_buttonWaitDialog->done(QMessageBox::Ok);
		}
		EditGeneric *og = new EditGeneric(g, m_typeDesc, m_parent);
		connect(og, SIGNAL(abort()), this, SIGNAL(abort()));
		connect(og, SIGNAL(accountChanged(int)), m_parent, SLOT(entryChanged(int)));
		connect(og, SIGNAL(finished(int)), og, SLOT(deleteLater()));
		og->show();
	} break;
	}
}

int GenericActionBar::esdbType()
{
	return ESDB_TYPE_GENERIC;
}

void GenericActionBar::newEntryFinished(int)
{
	m_newEntryDlg->deleteLater();
	m_newEntryDlg = NULL;
	m_parent->finishTask(false);
}

void GenericActionBar::entryCreated(esdbEntry *entry)
{
	m_parent->entryCreated(m_module->name(), entry);
}
