#include "bookmarkactionbar.h"
#include "esdb.h"
#include "bookmark.h"
#include "newbookmark.h"

#include <QDesktopServices>
#include <QUrl>
#include <QPushButton>
#include <QHBoxLayout>

#include "esdbtypemodule.h"
#include "loggedinwidget.h"
#include "buttonwaitdialog.h"

BookmarkActionBar::BookmarkActionBar(esdbTypeModule *module, LoggedInWidget *parent) :
	m_parent(parent),
	m_buttonWaitDialog(NULL),
	m_newEntryDlg(NULL),
	m_module(module),
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

QPushButton *BookmarkActionBar::addButton(const QString &tooltip, const QString &imagePath)
{
	QIcon icn = QIcon(imagePath);
	QPushButton *button = new QPushButton(icn, "");
	button->setToolTip(tooltip);
	button->setAutoDefault(true);
	layout()->addWidget(button);
	m_allButtons.push_back(button);
	return button;
}

//Need:
// 1) Browseable flag
// 2) List of non-browse buttons

void BookmarkActionBar::selectEntry(esdbEntry *entry)
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

void BookmarkActionBar::browseUrl(esdbEntry *entry)
{
	if (entry) {
		QUrl url(entry->getUrl());
		if (!url.scheme().size()) {
			url.setScheme("HTTP");
		}
		QDesktopServices::openUrl(url);
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
	m_newEntryDlg = new NewBookmark(id, name, this);
	QObject::connect(m_newEntryDlg, SIGNAL(entryCreated(esdbEntry *)), this, SLOT(entryCreated(esdbEntry *)));
	QObject::connect(m_newEntryDlg, SIGNAL(finished(int)), this, SLOT(newEntryFinished(int)));
	m_newEntryDlg->show();
}

void BookmarkActionBar::openEntryUI()
{
}

void BookmarkActionBar::deleteEntryUI()
{
	esdbEntry *entry = selectedEntry();
	if (entry) {
		m_parent->selectEntry(NULL);
		int id = entry->id;
		m_buttonWaitDialog = new ButtonWaitDialog("Delete bookmark",
			QString("delete bookmark \"") + entry->getTitle() + QString("\""),
			m_parent);
		connect(m_buttonWaitDialog, SIGNAL(finished(int)), this, SLOT(deleteEntryFinished(int)));
		m_buttonWaitDialog->show();
		m_parent->beginIDTask(id, LoggedInWidget::ID_TASK_DELETE, NONE, this);
	}
}

void BookmarkActionBar::deleteEntryFinished(int code)
{
	if (code != QMessageBox::Ok) {
		::signetdev_cancel_button_wait();
	}
	m_buttonWaitDialog->deleteLater();
	m_buttonWaitDialog = NULL;
	m_parent->finishTask();
}

void BookmarkActionBar::getEntryDone(esdbEntry *entry, int intent)
{
	Q_UNUSED(entry);
	Q_UNUSED(intent);
}

void BookmarkActionBar::idTaskComplete(int id, int task, int intent)
{
	Q_UNUSED(id);
	Q_UNUSED(intent);
	Q_UNUSED(task);
	if (m_buttonWaitDialog)
		m_buttonWaitDialog->done(QMessageBox::Ok);

}

void BookmarkActionBar::newEntryFinished(int)
{
	m_newEntryDlg->deleteLater();
	m_newEntryDlg = NULL;
	m_parent->finishTask(false);
}

void BookmarkActionBar::entryCreated(esdbEntry *entry)
{
	m_parent->entryCreated("bookmark", entry);
}
