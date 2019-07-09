#include "esdbactionbar.h"
#include "loggedinwidget.h"
#include "buttonwaitdialog.h"
#include "buttonwaitwidget.h"

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
        QString action = QString("delete " + m_typeName + " \"") + entry->getTitle() + QString("\"");
		ButtonWaitWidget *buttonWaitWidget = m_parent->beginButtonWait(action, false);
		connect(buttonWaitWidget, SIGNAL(timeout()), this, SLOT(buttonWaitTimeout()));
		connect(buttonWaitWidget, SIGNAL(canceled()), this, SLOT(buttonWaitCanceled()));
        if (!m_parent->beginIDTask(id, LoggedInWidget::ID_TASK_DELETE, INTENT_NONE, this)) {
            m_parent->endButtonWait();
		}
	}
}

void EsdbActionBar::openEntry(esdbEntry *entry)
{
	if (entry) {
		int id = entry->id;
        QString action = "open " + m_typeName.toLower() +  " \"" + entry->getTitle() + "\"";
		ButtonWaitWidget *buttonWaitWidget = m_parent->beginButtonWait(action, false);
		connect(buttonWaitWidget, SIGNAL(timeout()), this, SLOT(buttonWaitTimeout()));
		connect(buttonWaitWidget, SIGNAL(canceled()), this, SLOT(buttonWaitCanceled()));
        if (!m_parent->beginIDTask(id, LoggedInWidget::ID_TASK_READ, INTENT_OPEN_ENTRY, this)) {
            m_parent->endButtonWait();
        }
    }
}

void EsdbActionBar::accessEntry(esdbEntry *entry, int intent, QString message, bool backgroundApp)
{
	QString title = entry->getTitle();
	ButtonWaitWidget *buttonWaitWidget = m_parent->beginButtonWait(message, false);
	connect(buttonWaitWidget, SIGNAL(timeout()), this, SLOT(buttonWaitTimeout()));
	connect(buttonWaitWidget, SIGNAL(canceled()), this, SLOT(buttonWaitCanceled()));
	if (m_parent->beginIDTask(entry->id, LoggedInWidget::ID_TASK_READ, intent, this)) {
#ifndef Q_OS_MACOS
		if (backgroundApp) {
			background();
        }
#endif
	} else {
        m_parent->endButtonWait();
	}
}

void EsdbActionBar::buttonWaitTimeout()
{
    m_parent->endButtonWait();
    m_parent->finishTask();
}

void EsdbActionBar::buttonWaitCanceled()
{
    ::signetdev_cancel_button_wait();
    m_parent->endButtonWait();
    m_parent->finishTask();
}

void EsdbActionBar::idTaskComplete(bool error, int id, esdbEntry *entry, enum LoggedInWidget::ID_TASK task, int intent)
{
	Q_UNUSED(id);
	if (error) {
        m_parent->endButtonWait();
        m_parent->finishTask();
		return;
	}
	if (entry && task == LoggedInWidget::ID_TASK_READ) {
        if (accessEntryComplete(entry, intent)) {
            m_parent->endButtonWait();
            m_parent->finishTask();
        }
    } else {
        m_parent->endButtonWait();
        m_parent->finishTask();
        if (task == LoggedInWidget::ID_TASK_DELETE) {
            deleteEntryComplete(entry);
        }
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
