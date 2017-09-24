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
#include "newgeneric.h"
#include "opengeneric.h"
#include "generic.h"
#include "generictypedesc.h"

GenericActionBar::GenericActionBar(esdbTypeModule *module, genericTypeDesc *typeDesc, LoggedInWidget *parent) :
	m_parent(parent),
	m_buttonWaitDialog(NULL),
	m_module(module),
	m_typeDesc(typeDesc),
	m_newEntryDlg(NULL),
	m_browseButton(NULL)
{
	QHBoxLayout *l = new QHBoxLayout();
	l->setAlignment(Qt::AlignLeft);
	l->setContentsMargins(0,0,0,0);
	setLayout(l);

	if (module->hasUrl()) {
		m_browseButton = addButton("Browse", ":/images/browse.png");
		connect(m_browseButton, SIGNAL(pressed()), this, SLOT(browseUrlUI()));
	}
	QPushButton *button = addButton("Open", ":/images/open.png");
	connect(button, SIGNAL(pressed()), this, SLOT(openEntryUI()));

	button = addButton("Delete", ":/images/delete.png");
	connect(button, SIGNAL(pressed()), this, SLOT(deleteEntryUI()));
}

QPushButton *GenericActionBar::addButton(const QString &tooltip, const QString &imagePath)
{
	QIcon icn = QIcon(imagePath);
	QPushButton *button = new QPushButton(icn, "");
	button->setToolTip(tooltip);
	button->setAutoDefault(true);
	layout()->addWidget(button);
	m_allButtons.push_back(button);
	return button;
}

void GenericActionBar::selectEntry(esdbEntry *entry)
{
	m_selectedEntry = entry;
	bool enable = entry != NULL;
	bool browse_enable = enable;
	if (enable) {
		QUrl url(entry->getUrl());
		if (!url.isValid() || url.isEmpty()) {
			browse_enable = false;
		}
	}
	for (auto x : m_allButtons) {
		if (x != m_browseButton) {
			x->setEnabled(enable);
		} else {
			x->setEnabled(browse_enable);
		}
	}
}

void GenericActionBar::browseUrl(esdbEntry *entry)
{
	if (entry) {
		QUrl url(entry->getUrl());
		if (!url.scheme().size()) {
			url.setScheme("HTTP");
		}
		QDesktopServices::openUrl(url);
	}
}

void GenericActionBar::openEntry(esdbEntry *entry)
{
	if (entry) {
		int id = entry->id;
		m_buttonWaitDialog = new ButtonWaitDialog(
		    "Open " + m_typeDesc->name.toLower(),
		    "Open " + m_typeDesc->name.toLower() +  " \"" + entry->getTitle() + "\"",
		    m_parent);
		connect(m_buttonWaitDialog, SIGNAL(finished(int)), this, SLOT(openEntryFinished(int)));
		m_buttonWaitDialog->show();
		m_parent->beginIDTask(id, LoggedInWidget::ID_TASK_READ, OPEN_ACCOUNT);
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
	m_newEntryDlg = new NewGeneric(id, m_typeDesc, name, this);
	QObject::connect(m_newEntryDlg, SIGNAL(entryCreated(esdbEntry *)), this, SLOT(entryCreated(esdbEntry *)));
	QObject::connect(m_newEntryDlg, SIGNAL(finished(int)), this, SLOT(newEntryFinished(int)));
	m_newEntryDlg->show();
}

#include "generictypedesc.h"

void GenericActionBar::openEntryUI()
{
	esdbEntry * entry = selectedEntry();
	openEntry(entry);
}

void GenericActionBar::openEntryFinished(int code)
{
	m_buttonWaitDialog->deleteLater();
	m_buttonWaitDialog = NULL;
	if (code != QMessageBox::Ok) {
		::signetdev_cancel_button_wait();
	}
	m_parent->finishTask(true);
}

void GenericActionBar::deleteEntryUI()
{
	esdbEntry *entry = selectedEntry();
	if (entry) {
		m_parent->selectEntry(NULL);
		int id = entry->id;
		m_buttonWaitDialog = new ButtonWaitDialog("Delete " + m_typeDesc->name,
			QString("delete " + m_typeDesc->name + " \"") + entry->getTitle() + QString("\""),
			m_parent);
		connect(m_buttonWaitDialog, SIGNAL(finished(int)), this, SLOT(deleteEntryFinished(int)));
		m_buttonWaitDialog->show();
		m_parent->beginIDTask(id, LoggedInWidget::ID_TASK_DELETE, NONE);
	}
}

void GenericActionBar::deleteEntryFinished(int code)
{
	if (code != QMessageBox::Ok) {
		::signetdev_cancel_button_wait();
	}
	m_buttonWaitDialog->deleteLater();
	m_buttonWaitDialog = NULL;
	m_parent->finishTask();
}

void GenericActionBar::getEntryDone(esdbEntry *entry, int intent)
{
	if (entry) {
		switch (intent) {
		case OPEN_ACCOUNT: {
			generic *g = static_cast<generic *>(entry);
			if (m_buttonWaitDialog) {
				m_buttonWaitDialog->done(QMessageBox::Ok);
			}
			OpenGeneric *og = new OpenGeneric(g, m_typeDesc, m_parent);
			connect(og, SIGNAL(abort()), this, SIGNAL(abort()));
			connect(og, SIGNAL(accountChanged(int)), m_parent, SLOT(entryChanged(int)));
			connect(og, SIGNAL(finished(int)), og, SLOT(deleteLater()));
			og->show();
		}
		break;
		}
	}
}

int GenericActionBar::esdbType()
{
	return ESDB_TYPE_GENERIC;
}

void GenericActionBar::idTaskComplete(int id, int intent)
{
	Q_UNUSED(id);
	Q_UNUSED(intent);
	if (m_buttonWaitDialog)
		m_buttonWaitDialog->done(QMessageBox::Ok);

}

void GenericActionBar::newEntryFinished(int)
{
	m_newEntryDlg->deleteLater();
	m_newEntryDlg = NULL;
	m_parent->finishTask(false);
}

void GenericActionBar::entryCreated(esdbEntry *entry)
{
	m_parent->entryCreated(this, entry);
}
