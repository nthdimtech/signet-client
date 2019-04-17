#include "esdbactionbar.h"
#include "loggedinwidget.h"
#include "buttonwaitdialog.h"

#include <QPushButton>
#include <QIcon>
#include <QString>
#include <QBoxLayout>
#include <QDesktopServices>

EsdbActionBar::EsdbActionBar(LoggedInWidget *parent, QString typeName, bool writeEnabled, bool typeEnabled) : QWidget(parent),
	m_parent(parent),
	m_writeEnabled(writeEnabled),
	m_typeEnabled(typeEnabled),
	m_typeName(typeName),
	m_selectedEntry(nullptr)
{
	QHBoxLayout *l = new QHBoxLayout();
	l->setAlignment(Qt::AlignLeft);
	l->setContentsMargins(0,0,0,0);
	setLayout(l);
}

QPushButton *EsdbActionBar::addButton(const QString &tooltip, const QString &imagePath, bool writeOp, bool typeOp)
{
	QIcon icn = QIcon(imagePath);
	QPushButton *button = new QPushButton(icn, "");
	button->setDisabled((writeOp && !m_writeEnabled) || (typeOp && !m_typeEnabled));
	button->setToolTip(tooltip);
	button->setAutoDefault(true);
	layout()->addWidget(button);
	m_allButtons.push_back(button);
	return button;
}

QPushButton *EsdbActionBar::addButton(QPushButton *button, bool writeOp, bool typeOp)
{
	button->setAutoDefault(true);
	button->setDisabled((writeOp && !m_writeEnabled) || (typeOp && !m_typeEnabled));
	layout()->addWidget(button);
	m_allButtons.push_back(button);
	return button;
}

void EsdbActionBar::deleteEntry()
{
	esdbEntry *entry = selectedEntry();
	if (entry) {
		m_parent->selectEntry(nullptr);
		int id = entry->id;
		m_buttonWaitDialog = new ButtonWaitDialog("Delete " + m_typeName,
		                QString("delete " + m_typeName + " \"") + entry->getTitle() + QString("\""),
		                m_parent);
		connect(m_buttonWaitDialog, SIGNAL(finished(int)), this, SLOT(deleteEntryFinished(int)));
		if (m_parent->beginIDTask(id, LoggedInWidget::ID_TASK_DELETE, INTENT_NONE, this)) {
			if (m_buttonWaitDialog)
				m_buttonWaitDialog->show();
		} else {
			m_buttonWaitDialog->deleteLater();
			m_buttonWaitDialog = nullptr;
		}
	}
}

void EsdbActionBar::openEntry(esdbEntry *entry)
{
	if (entry) {
		int id = entry->id;
		m_buttonWaitDialog = new ButtonWaitDialog(
		        "Open " + m_typeName.toLower(),
		        "open " + m_typeName.toLower() +  " \"" + entry->getTitle() + "\"",
		        m_parent);
		connect(m_buttonWaitDialog, SIGNAL(finished(int)), this, SLOT(openEntryFinished(int)));
		if (m_parent->beginIDTask(id, LoggedInWidget::ID_TASK_READ, INTENT_OPEN_ENTRY, this)) {
			if (m_buttonWaitDialog)
				m_buttonWaitDialog->show();
		} else {
			m_buttonWaitDialog->deleteLater();
			m_buttonWaitDialog = nullptr;
		}
	}
}

void EsdbActionBar::accessEntry(esdbEntry *entry, int intent, QString message, bool backgroundApp, bool deselect)
{
	QString title = entry->getTitle();
	m_buttonWaitDialog = new ButtonWaitDialog(title, message, m_parent);
	m_accessDeselect = deselect;
	connect(m_buttonWaitDialog, SIGNAL(finished(int)), this, SLOT(accessAccountFinished(int)));
	if (m_parent->beginIDTask(entry->id, LoggedInWidget::ID_TASK_READ, intent, this)) {
		m_buttonWaitDialog->show();

		if (backgroundApp) {
			background();
		}
		m_buttonWaitDialog->show();
	} else {
		m_buttonWaitDialog->deleteLater();
		m_buttonWaitDialog = nullptr;
	}

}

void EsdbActionBar::accessAccountFinished(int code)
{
	if (m_buttonWaitDialog) {
		m_buttonWaitDialog->deleteLater();
		m_buttonWaitDialog = nullptr;
	}
	if (code != QMessageBox::Ok) {
		::signetdev_cancel_button_wait();
	}
	m_parent->finishTask(m_accessDeselect);
}

void EsdbActionBar::openEntryFinished(int code)
{
	m_buttonWaitDialog->deleteLater();
	m_buttonWaitDialog = nullptr;
	if (code != QMessageBox::Ok) {
		::signetdev_cancel_button_wait();
	}
	m_parent->finishTask(true);
}

void EsdbActionBar::idTaskComplete(bool error, int id, esdbEntry *entry, enum LoggedInWidget::ID_TASK task, int intent)
{
	Q_UNUSED(id);
	if (error) {
		if (m_buttonWaitDialog)
			m_buttonWaitDialog->done(QMessageBox::Ok);
		return;
	}
	if (entry && task == LoggedInWidget::ID_TASK_READ) {
		if (intent != INTENT_TYPE_ENTRY && m_buttonWaitDialog)
			m_buttonWaitDialog->done(QMessageBox::Ok);
		accessEntryComplete(entry, intent);
	}
	if (entry && task == LoggedInWidget::ID_TASK_DELETE) {
		deleteEntryComplete(entry);
	} else {
		if (m_buttonWaitDialog)
			m_buttonWaitDialog->done(QMessageBox::Ok);
	}
}

void EsdbActionBar::selectEntry(esdbEntry *entry)
{
	m_selectedEntry = entry;
	for (auto x : m_allButtons) {
		x->setEnabled(m_selectedEntry);
	}
	entrySelected(entry);
}

void EsdbActionBar::browseUrl(esdbEntry *entry)
{
	if (entry) {
		QUrl url(entry->getUrl());
		if (!url.scheme().size()) {
			url.setScheme("HTTP");
		}
		QDesktopServices::openUrl(url);
	}
}

void EsdbActionBar::deleteEntryFinished(int code)
{
	if (code != QMessageBox::Ok) {
		::signetdev_cancel_button_wait();
	}
	m_buttonWaitDialog->deleteLater();
	m_buttonWaitDialog = nullptr;
	m_parent->finishTask();
}
