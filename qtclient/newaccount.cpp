#include "newaccount.h"
#include <QBoxLayout>
#include <QLineEdit>
#include <QLabel>
#include <QPushButton>
#include <QCheckBox>
#include <QSpinBox>
#include<QClipboard>
#include<QApplication>
#include <random>

#include "account.h"
#include "buttonwaitdialog.h"
#include "databasefield.h"
#include "passwordedit.h"
#include "signetapplication.h"
#include "genericfieldseditor.h"

extern "C" {
#include "signetdev.h"
}

NewAccount::NewAccount(int id, const QString &name, QWidget *parent) : QDialog(parent),
	m_wait_dialog(NULL),
	m_acct(NULL),
	m_id(id),
	m_signetdev_cmd_token(-1)
{
	setWindowModality(Qt::WindowModal);
	SignetApplication *app = SignetApplication::get();
	connect(app, SIGNAL(signetdevCmdResp(signetdevCmdRespInfo)),
		this, SLOT(signetdev_cmd_resp(signetdevCmdRespInfo)));

	setWindowTitle("New account");

	m_genericFieldsEditor = new GenericFieldsEditor(m_fields,
					QList<fieldSpec>());

	QBoxLayout *layout = new QBoxLayout(QBoxLayout::TopToBottom);
	layout->setAlignment(Qt::AlignTop);

	QBoxLayout *account_name_layout = new QBoxLayout(QBoxLayout::LeftToRight);
	m_account_name_field = new QLineEdit(name);
	connect(m_account_name_field, SIGNAL(textEdited(QString)), this, SLOT(account_name_edited(QString)));

	account_name_layout->addWidget(new QLabel("Account name"));
	account_name_layout->addWidget(m_account_name_field);

	m_account_name_warning = new QLabel();
	m_account_name_warning->setStyleSheet("QLabel { color : red; }");
	m_account_name_warning->hide();

	m_username_field = new DatabaseField("username", 140, NULL);
	connect(m_username_field, SIGNAL(editingFinished()), this,
		SLOT(username_editing_finished()));

	m_url_field = new DatabaseField("URL", 140);

	m_email_field = new DatabaseField("email", 140);

	QPushButton *create_button = new QPushButton("Create");
	connect(create_button, SIGNAL(pressed()), this, SLOT(create_button_pressed()));
	create_button->setDefault(true);

	m_password_edit = new PasswordEdit();

	layout->addLayout(account_name_layout);
	layout->addWidget(m_account_name_warning);
	layout->addWidget(m_username_field);
	layout->addWidget(m_email_field);
	layout->addWidget(m_password_edit);
	layout->addWidget(m_url_field);
	layout->addWidget(m_genericFieldsEditor);
	layout->addWidget(create_button);

	setLayout(layout);

	if (name.size()) {
		m_username_field->setFocus();
	}
}

void NewAccount::account_name_edited(QString str)
{
	Q_UNUSED(str);
	m_account_name_warning->hide();
}

void NewAccount::create_button_pressed()
{
	QString action = "add account \"" + m_account_name_field->text() + "\"";

	if (m_account_name_field->text().size() == 0) {
		m_account_name_warning->setText("You must give the account a name");
		m_account_name_warning->show();
		return;
	}

	m_wait_dialog = new ButtonWaitDialog("Add account", action, this);
	connect(m_wait_dialog, SIGNAL(finished(int)), this, SLOT(add_account_finished(int)));
	m_wait_dialog->show();
	::signetdev_open_id_async(NULL, &m_signetdev_cmd_token, m_id);
}

void NewAccount::signetdev_cmd_resp(signetdevCmdRespInfo info)
{
	int code = info.resp_code;

	if (m_signetdev_cmd_token != info.token) {
		return;
	}
	m_signetdev_cmd_token = -1;

	if (m_wait_dialog) {
		m_wait_dialog->done(QMessageBox::Ok);
	}

	switch (code) {
	case OKAY: {
		switch (info.cmd) {
		case SIGNETDEV_CMD_OPEN_ID: {
			block blk;
			m_acct = new account(m_id);
			m_acct->acctName = m_account_name_field->text();
			m_acct->userName = m_username_field->text();
			m_acct->password = m_password_edit->password();
			m_acct->url = m_url_field->text();
			m_acct->email = m_email_field->text();
			m_genericFieldsEditor->saveFields();
			m_acct->fields = m_fields;
			m_acct->toBlock(&blk);
			::signetdev_write_id_async(NULL, &m_signetdev_cmd_token,
						   m_acct->id,
						   blk.data.size(),
						   (const u8 *)blk.data.data(),
						   (const u8 *)blk.mask.data());
		}
		break;
		case SIGNETDEV_CMD_WRITE_ID:
			emit accountCreated(m_acct);
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

void NewAccount::username_editing_finished()
{
	if (m_email_field->text().isEmpty() && is_email(m_username_field->text())) {
		m_email_field->setText(m_username_field->text());
	}
}

void NewAccount::add_account_finished(int code)
{
	if (m_wait_dialog)
		m_wait_dialog->deleteLater();
	m_wait_dialog = NULL;
	if (code != QMessageBox::Ok) {
		::signetdev_cancel_button_wait();
	}
}
