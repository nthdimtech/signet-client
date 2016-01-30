#include "editaccount.h"
#include <QBoxLayout>
#include <QLineEdit>
#include <QLabel>
#include <QPushButton>
#include <QMessageBox>
#include <QString>
#include <QCheckBox>
#include <QUrl>
#include <QDesktopServices>
#include <passwordedit.h>

#include "databasefield.h"
#include "account.h"
#include "buttonwaitdialog.h"
#include "signetapplication.h"

extern "C" {
#include "signetdev.h"
}

EditAccount::EditAccount(account *acct, QWidget *parent) :
	QDialog(parent),
	m_acct(acct),
	m_buttonDialog(NULL),
	m_signetdevCmdToken(-1)
{
	setWindowModality(Qt::WindowModal);
	SignetApplication *app = SignetApplication::get();
	connect(app, SIGNAL(signetdevCmdResp(signetdevCmdRespInfo)),
	        this, SLOT(signetdevCmdResp(signetdevCmdRespInfo)));

	this->setWindowTitle(acct->acctName);
	m_accountNameEdit = new QLineEdit();

	QBoxLayout *account_name_layout = new QBoxLayout(QBoxLayout::LeftToRight);
	account_name_layout->addWidget(new QLabel("Account name"));
	account_name_layout->addWidget(m_accountNameEdit);

	m_usernameField = new DatabaseField("username", 120, NULL);
	m_emailField = new DatabaseField("email", 120, NULL);
	m_passwordEdit = new PasswordEdit();
	m_browseUrlButton = new QPushButton(QIcon(":/images/browse.png"),"");
	m_browseUrlButton->setToolTip("Browse");
	connect(m_browseUrlButton, SIGNAL(pressed()), this, SLOT(browseUrl()));

	m_urlField = new DatabaseField("URL", 140, m_browseUrlButton);

	connect(m_accountNameEdit, SIGNAL(textEdited(QString)),
	        this, SLOT(textEdited(QString)));
	connect(m_passwordEdit, SIGNAL(textEdited(QString)),
	        this, SLOT(textEdited(QString)));
	connect(m_usernameField, SIGNAL(textEdited(QString)),
	        this, SLOT(textEdited(QString)));
	connect(m_emailField, SIGNAL(textEdited(QString)),
	        this, SLOT(textEdited(QString)));
	connect(m_urlField, SIGNAL(textEdited(QString)),
	        this, SLOT(textEdited(QString)));


	setAccountValues();

	m_undoChangesButton = new QPushButton("Undo");

	m_saveButton = new QPushButton("Save");
	m_saveButton->setDisabled(true);
	m_saveButton->setDefault(true);

	QPushButton *close_button = new QPushButton("Close");
	m_undoChangesButton->setDisabled(true);
	connect(m_undoChangesButton, SIGNAL(pressed()), this, SLOT(undoChangesUi()));

	QHBoxLayout *buttons = new QHBoxLayout();

	buttons->addWidget(m_saveButton);
	buttons->addWidget(m_undoChangesButton);
	buttons->addWidget(close_button);

	QBoxLayout *main_layout = new QBoxLayout(QBoxLayout::TopToBottom);
	main_layout->setAlignment(Qt::AlignTop);
	main_layout->addLayout(account_name_layout);
	main_layout->addWidget(m_usernameField);
	main_layout->addWidget(m_emailField);
	main_layout->addWidget(m_passwordEdit);
	main_layout->addWidget(m_urlField);
	main_layout->addLayout(buttons);
	setLayout(main_layout);

	connect(m_saveButton, SIGNAL(pressed(void)), this, SLOT(savePressed(void)));
	connect(close_button, SIGNAL(pressed(void)), this, SLOT(closePressed(void)));
}

void EditAccount::signetdevCmdResp(signetdevCmdRespInfo info)
{
	int code = info.resp_code;

	if (m_signetdevCmdToken != info.token) {
		return;
	}
	m_signetdevCmdToken = -1;

	if (m_buttonDialog) {
		m_buttonDialog->done(QMessageBox::Ok);
	}

	switch (code) {
	case OKAY: {
		switch (info.cmd) {
		case SIGNETDEV_CMD_OPEN_ID: {
			account acct(m_acct->id);
			acct.acctName = m_accountNameEdit->text();
			acct.userName = m_usernameField->text();
			acct.password = m_passwordEdit->password();
			acct.url = m_urlField->text();
			acct.email = m_emailField->text();
			block blk;
			acct.toBlock(&blk);
			::signetdev_write_id_async(NULL, &m_signetdevCmdToken,
			                           m_acct->id,
			                           blk.data.size(),
			                           (const u8 *)blk.data.data(),
			                           (const u8 *)blk.mask.data());
		}
		break;
		case SIGNETDEV_CMD_WRITE_ID:
			m_acct->acctName = m_accountNameEdit->text();
			m_acct->userName = m_usernameField->text();
			m_acct->password = m_passwordEdit->password();
			m_acct->url = m_urlField->text();
			m_acct->email = m_emailField->text();
			emit accountChanged(m_acct->id);
			m_saveButton->setDisabled(true);
			m_undoChangesButton->setDisabled(true);
			break;
		}
	}
	break;
	case BUTTON_PRESS_TIMEOUT:
	case BUTTON_PRESS_CANCELED:
		m_saveButton->setDisabled(false);
		m_undoChangesButton->setDisabled(false);
		break;
	case SIGNET_ERROR_DISCONNECT:
	case SIGNET_ERROR_QUIT:
		close();
		break;
	default: {
		emit abort();
	}
	break;
	}
}

void EditAccount::browseUrl()
{
	QUrl url(m_urlField->text());
	QString scheme = url.scheme();
	if (!scheme.size()) {
		url.setScheme("HTTP");
	}
	QDesktopServices::openUrl(url);
}

EditAccount::~EditAccount()
{
}

void EditAccount::setAccountValues()
{
	m_usernameField->setText(m_acct->userName);
	m_accountNameEdit->setText(m_acct->acctName);
	m_emailField->setText(m_acct->email);
	m_passwordEdit->setPassword(m_acct->password);
	m_urlField->setText(m_acct->url);
}

void EditAccount::undoChangesUi()
{
	setAccountValues();
	m_saveButton->setDisabled(true);
	m_undoChangesButton->setDisabled(true);
}

void EditAccount::textEdited(QString s)
{
	Q_UNUSED(s);
	m_saveButton->setDisabled(false);
	m_undoChangesButton->setDisabled(false);
}

void EditAccount::closePressed()
{
	close();
}

void EditAccount::savePressed()
{
	m_buttonDialog = new ButtonWaitDialog( "Open account",
	                                       QString("save changes to account \"") + m_accountNameEdit->text() + QString("\""),
	                                       this);
	connect(m_buttonDialog, SIGNAL(finished(int)), this, SLOT(editAccountFinished(int)));
	m_buttonDialog->show();

	::signetdev_open_id_async(NULL, &m_signetdevCmdToken, m_acct->id);
}

void EditAccount::editAccountFinished(int code)
{
	if (code != QMessageBox::Ok) {
		::signetdev_cancel_button_wait();
	}
	m_buttonDialog->deleteLater();
	m_buttonDialog = NULL;
}
