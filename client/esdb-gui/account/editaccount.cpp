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
#include <QCloseEvent>
#include <passwordedit.h>

#include "databasefield.h"
#include "account.h"
#include "buttonwaitdialog.h"
#include "signetapplication.h"
#include "genericfieldseditor.h"

extern "C" {
#include "signetdev/host/signetdev.h"
};

EditAccount::EditAccount(account *acct, QWidget *parent) :
	QDialog(parent),
	m_acct(acct),
	m_buttonDialog(NULL),
	m_signetdevCmdToken(-1),
	m_saveButton(NULL),
	m_undoChangesButton(NULL),
	m_settingFields(false),
	m_changesMade(false),
	m_closeOnSave(false)
{
	setWindowModality(Qt::WindowModal);
	SignetApplication *app = SignetApplication::get();
	connect(app, SIGNAL(signetdevCmdResp(signetdevCmdRespInfo)),
		this, SLOT(signetdevCmdResp(signetdevCmdRespInfo)));

	this->setWindowTitle(acct->acctName);
	m_accountNameEdit = new QLineEdit();

	m_genericFieldsEditor = new GenericFieldsEditor(m_acct->fields,
					QList<fieldSpec>());

	QBoxLayout *account_name_layout = new QBoxLayout(QBoxLayout::LeftToRight);
	account_name_layout->addWidget(new QLabel("Account name"));
	account_name_layout->addWidget(m_accountNameEdit);
	connect(m_accountNameEdit, SIGNAL(textEdited(QString)), this, SLOT(accountNameEdited()));

	m_accountNameWarning = new QLabel();
	m_accountNameWarning->setStyleSheet("QLabel { color : red; }");
	m_accountNameWarning->hide();

	m_usernameField = new DatabaseField("username", 120, NULL);
	m_groupField = new DatabaseField("group", 120, NULL);
	m_emailField = new DatabaseField("email", 120, NULL);
	m_passwordEdit = new PasswordEdit();
	m_browseUrlButton = new QPushButton(QIcon(":/images/browse.png"),"");
	m_browseUrlButton->setToolTip("Browse");
	connect(m_browseUrlButton, SIGNAL(pressed()), this, SLOT(browseUrl()));

	m_urlField = new DatabaseField("URL", 140, m_browseUrlButton);

	connect(m_accountNameEdit, SIGNAL(textEdited(QString)),
		this, SLOT(textEdited()));
	connect(m_passwordEdit, SIGNAL(textEdited(QString)),
		this, SLOT(textEdited()));
	connect(m_usernameField, SIGNAL(textEdited(QString)),
		this, SLOT(textEdited()));
	connect(m_emailField, SIGNAL(textEdited(QString)),
		this, SLOT(textEdited()));
	connect(m_urlField, SIGNAL(textEdited(QString)),
		this, SLOT(textEdited()));
	connect(m_genericFieldsEditor, SIGNAL(edited()),
		this, SLOT(textEdited()));
	connect(m_groupField, SIGNAL(textEdited(QString)),
		this, SLOT(textEdited()));

	m_settingFields = true;
	setAccountValues();
	m_genericFieldsEditor->loadFields();
	m_settingFields = false;

	m_undoChangesButton = new QPushButton("Undo");

	m_saveButton = new QPushButton("&Save");
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
	main_layout->addWidget(m_accountNameWarning);
	main_layout->addWidget(m_groupField);
	main_layout->addWidget(m_usernameField);
	main_layout->addWidget(m_emailField);
	main_layout->addWidget(m_passwordEdit);
	main_layout->addWidget(m_urlField);
	main_layout->addWidget(m_genericFieldsEditor);
	main_layout->addLayout(buttons);
	setLayout(main_layout);

	connect(m_saveButton, SIGNAL(pressed(void)), this, SLOT(savePressed(void)));
	connect(close_button, SIGNAL(pressed(void)), this, SLOT(close(void)));
}

void EditAccount::accountNameEdited()
{
	m_accountNameWarning->hide();
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
		case SIGNETDEV_CMD_UPDATE_UID:
			m_acct->acctName = m_accountNameEdit->text();
			m_acct->userName = m_usernameField->text();
			m_acct->password = m_passwordEdit->password();
			m_acct->url = m_urlField->text();
			m_acct->email = m_emailField->text();
			m_acct->path = m_groupField->text();
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
	m_groupField->setText(m_acct->path);
}

void EditAccount::undoChangesUi()
{
	m_settingFields = true;
	setAccountValues();
	m_genericFieldsEditor->loadFields();
	m_settingFields = false;
	m_saveButton->setDisabled(true);
	m_undoChangesButton->setDisabled(true);
	m_accountNameWarning->hide();
	m_changesMade = false;
}

void EditAccount::textEdited()
{
	if (!m_settingFields) {
		m_saveButton->setDisabled(false);
		m_undoChangesButton->setDisabled(false);
		m_changesMade = true;
	}
}

void EditAccount::closeEvent(QCloseEvent *event)
{
	if (m_changesMade) {
		QMessageBox *box = new QMessageBox(QMessageBox::Question, windowTitle(),
					       "You have made changes. Do you want to save them",
					       QMessageBox::Yes |
					       QMessageBox::No,
					       this);
		int rc = box->exec();
		box->deleteLater();
		if (rc == QMessageBox::Yes) {
			m_closeOnSave = true;
			event->ignore();
			savePressed();
			return;
		}
	}
	event->accept();
}

void EditAccount::savePressed()
{
	if (m_accountNameEdit->text().size() == 0) {
		m_accountNameWarning->setText("The account name cannot be empty");
		m_accountNameWarning->show();
		return;
	}
	m_buttonDialog = new ButtonWaitDialog( "Save account",
					       QString("save changes to account \"") + m_accountNameEdit->text() + QString("\""),
					       this);
	connect(m_buttonDialog, SIGNAL(finished(int)), this, SLOT(editAccountFinished(int)));
	m_buttonDialog->show();

	account acct(m_acct->id);
	acct.acctName = m_accountNameEdit->text();
	acct.userName = m_usernameField->text();
	acct.password = m_passwordEdit->password();
	acct.url = m_urlField->text();
	acct.email = m_emailField->text();
	acct.path = m_groupField->text();
	m_genericFieldsEditor->saveFields();
	acct.fields = m_acct->fields;
	block blk;
	acct.toBlock(&blk);
	::signetdev_update_uid(NULL, &m_signetdevCmdToken,
				   m_acct->id,
				   blk.data.size(),
				   (const u8 *)blk.data.data(),
				   (const u8 *)blk.mask.data());
}

void EditAccount::editAccountFinished(int code)
{
	if (code != QMessageBox::Ok) {
		::signetdev_cancel_button_wait();
	}
	m_buttonDialog->deleteLater();
	m_buttonDialog = NULL;
	m_changesMade = false;
	if (m_closeOnSave) {
		close();
	}
}
