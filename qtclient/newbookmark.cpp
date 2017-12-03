#include "newbookmark.h"

#include <QBoxLayout>
#include <QLineEdit>
#include <QLabel>
#include <QPushButton>

#include "databasefield.h"
#include "buttonwaitdialog.h"

extern "C" {
#include "signetdev/host/signetdev.h"
}

#include "bookmark.h"

#include "signetapplication.h"

NewBookmark::NewBookmark(int id, const QString &name, QWidget *parent) :
	QDialog(parent),
	m_id(id),
	m_signetdevCmdToken(-1)
{
	setWindowModality(Qt::WindowModal);
	SignetApplication *app = SignetApplication::get();
	connect(app, SIGNAL(signetdevCmdResp(signetdevCmdRespInfo)),
		this, SLOT(signetdevCmdResp(signetdevCmdRespInfo)));

	QBoxLayout *nameLayout = new QBoxLayout(QBoxLayout::LeftToRight);
	m_nameField = new QLineEdit(name);
	nameLayout->addWidget(new QLabel("Name"));
	nameLayout->addWidget(m_nameField);

	m_urlField = new DatabaseField("URL", 140);

	QPushButton *createButton = new QPushButton("Create");
	connect(createButton, SIGNAL(pressed()), this, SLOT(createButtonPressed()));
	createButton->setDefault(true);

	setWindowTitle("New bookmark");
	QBoxLayout *layout = new QBoxLayout(QBoxLayout::TopToBottom);
	layout->setAlignment(Qt::AlignTop);
	layout->addLayout(nameLayout);
	layout->addWidget(m_urlField);
	layout->addWidget(createButton);
	setLayout(layout);
}

void NewBookmark::createButtonPressed()
{
	QString action = "add bookmark \"" + m_nameField->text() + "\"";
	m_buttonWaitDialog = new ButtonWaitDialog("Add bookmark", action, this);
	connect(m_buttonWaitDialog, SIGNAL(finished(int)), this, SLOT(addEntryFinished(int)));
	m_buttonWaitDialog->show();

	block blk;
	m_entry = new bookmark(m_id);
	bookmark *bm = (bookmark *)m_entry;
	bm->name = m_nameField->text();
	bm->url = m_urlField->text();

	bm->toBlock(&blk);
	::signetdev_update_uid_async(NULL, &m_signetdevCmdToken,
				   bm->id,
				   blk.data.size(),
				   (const u8 *)blk.data.data(),
				   (const u8 *)blk.mask.data());
}

void NewBookmark::signetdevCmdResp(signetdevCmdRespInfo info)
{
	int code = info.resp_code;

	if (m_signetdevCmdToken != info.token) {
		return;
	}
	m_signetdevCmdToken = -1;

	if (m_buttonWaitDialog) {
		m_buttonWaitDialog->done(QMessageBox::Ok);
	}

	switch (code) {
	case OKAY: {
		switch (info.cmd) {
		case SIGNETDEV_CMD_UPDATE_UID:
			emit entryCreated(m_entry);
			close();
			break;
		}
	}
	break;
	case BUTTON_PRESS_TIMEOUT:
	case BUTTON_PRESS_CANCELED:
	case SIGNET_ERROR_DISCONNECT:
	case SIGNET_ERROR_QUIT:
		break;
	default: {
		emit abort();
		close();
	}
	break;
	}
}

void NewBookmark::addEntryFinished(int code)
{
	if (m_buttonWaitDialog)
		m_buttonWaitDialog->deleteLater();
	m_buttonWaitDialog = NULL;
	if (code != QMessageBox::Ok) {
		::signetdev_cancel_button_wait();
	}
}
