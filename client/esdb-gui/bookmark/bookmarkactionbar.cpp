#include "bookmarkactionbar.h"
#include "esdb.h"
#include "bookmark.h"
#include "editbookmark.h"

#include <QDesktopServices>
#include <QUrl>
#include <QPushButton>
#include <QHBoxLayout>

#include "esdbtypemodule.h"
#include "loggedinwidget.h"
#include "buttonwaitdialog.h"

BookmarkActionBar::BookmarkActionBar(esdbTypeModule *module, LoggedInWidget *parent, bool writeEnabled, bool typeEnabled) :
	EsdbActionBar(parent, "Account", writeEnabled, typeEnabled),
	m_newEntryDlg(NULL),
	m_module(module),
	m_browseButton(NULL)
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

//Need:
// 1) Browseable flag
// 2) List of non-browse buttons

void BookmarkActionBar::entrySelected(esdbEntry *entry)
{
	if (m_browseButton) {
		QUrl url(entry->getUrl());
		m_browseButton->setEnabled(entry && url.isValid() && !url.isEmpty());
	}
}

void BookmarkActionBar::defaultAction(esdbEntry *entry)
{
	browseUrl(entry);
}

void BookmarkActionBar::browseUrlUI()
{
	browseUrl(selectedEntry());
}

void BookmarkActionBar::newInstanceUI(int id, const QString &name)
{
	m_newEntryDlg = new EditBookmark(id, name, this);
	QObject::connect(m_newEntryDlg, SIGNAL(entryCreated(esdbEntry *)), this, SLOT(entryCreated(esdbEntry *)));
	QObject::connect(m_newEntryDlg, SIGNAL(finished(int)), this, SLOT(newEntryFinished(int)));
	m_newEntryDlg->show();
}

int BookmarkActionBar::esdbType()
{
	return ESDB_TYPE_BOOKMARK;
}

void BookmarkActionBar::openEntryUI()
{
	openEntry(selectedEntry());
}

void BookmarkActionBar::accessEntryComplete(esdbEntry *entry, int intent)
{
	bookmark *b = static_cast<bookmark *>(entry);
	switch (intent) {
	case INTENT_OPEN_ENTRY: {
		EditBookmark *eb = new EditBookmark(b, m_parent);
		connect(eb, SIGNAL(abort()), this, SIGNAL(abort()));
		connect(eb, SIGNAL(accountChanged(int)), m_parent, SLOT(entryChanged(int)));
		connect(eb, SIGNAL(finished(int)), eb, SLOT(deleteLater()));
		eb->show();
	} break;
	}
}

void BookmarkActionBar::deleteEntryUI()
{
	deleteEntry();
}

void BookmarkActionBar::newEntryFinished(int)
{
	m_newEntryDlg->deleteLater();
	m_newEntryDlg = NULL;
	m_parent->finishTask(false);
}

void BookmarkActionBar::entryCreated(esdbEntry *entry)
{
	m_parent->entryCreated("Bookmarks", entry);
}
