#include "opengeneric.h"
#include <QBoxLayout>
#include <QLineEdit>
#include <QLabel>
#include <QPushButton>
#include <QMessageBox>
#include <QString>
#include <QCheckBox>
#include <QDesktopServices>

#include "databasefield.h"
#include "account.h"
#include "buttonwaitdialog.h"
#include "signetapplication.h"
#include "generic.h"
#include "generictypedesc.h"

extern "C" {
#include "signetdev.h"
}

OpenGeneric::OpenGeneric(generic *generic, genericTypeDesc *typeDesc, QWidget *parent) :
	QDialog(parent),
	m_generic(generic),
	m_typeDesc(typeDesc),
	m_buttonWaitDialog(NULL),
	m_signetdevCmdToken(-1)
{
	setWindowModality(Qt::WindowModal);
	SignetApplication *app = SignetApplication::get();
	connect(app, SIGNAL(signetdev_cmd_resp(signetdevCmdRespInfo)),
	        this, SLOT(signetdevCmdResp(signetdevCmdRespInfo)));

	setWindowTitle(generic->name);
	m_genericNameEdit = new QLineEdit();

	QBoxLayout *nameLayout = new QBoxLayout(QBoxLayout::LeftToRight);
	nameLayout->addWidget(new QLabel("Name"));
	nameLayout->addWidget(m_genericNameEdit);

	//m_username_field = new DatabaseField("username", 120, NULL);

	connect(m_genericNameEdit, SIGNAL(textEdited(QString)),
	        this, SLOT(textEdited(QString)));

	for (auto typeField : typeDesc->fields) {
		DatabaseField *field = new DatabaseField(typeField, 200);
		m_typeFields.push_back(field);
	}
	setGenericValues();

	m_undoChangesButton = new QPushButton("Undo");

	m_saveButton = new QPushButton("Save");
	m_saveButton->setDisabled(true);
	m_saveButton->setDefault(true);

	QPushButton *closeButton = new QPushButton("Close");
	m_undoChangesButton->setDisabled(true);
	connect(m_undoChangesButton, SIGNAL(pressed()), this, SLOT(undoChangesUI()));

	QHBoxLayout *buttons = new QHBoxLayout();

	buttons->addWidget(m_saveButton);
	buttons->addWidget(m_undoChangesButton);
	buttons->addWidget(closeButton);

	QBoxLayout *mainLayout = new QBoxLayout(QBoxLayout::TopToBottom);
	mainLayout->setAlignment(Qt::AlignTop);
	mainLayout->addLayout(nameLayout);
	for (auto dbField : m_typeFields) {
		mainLayout->addWidget(dbField);
	}
	/*
	for (auto dbField : m_extraFields) {
		mainLayout->addWidget(dbField);
	}
	*/
	mainLayout->addLayout(buttons);
	setLayout(mainLayout);

	connect(m_saveButton, SIGNAL(pressed(void)), this, SLOT(savePressed(void)));
	connect(closeButton, SIGNAL(pressed(void)), this, SLOT(closePressed(void)));
}

void OpenGeneric::signetdevCmdResp(signetdevCmdRespInfo info)
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
#if 0
		switch (info.cmd) {
		case SIGNETDEV_CMD_OPEN_ID: {
			account acct(m_acct->id);
			acct.acct_name = m_account_name_edit->text();
			acct.user_name = m_username_field->m_field_edit->text();
			acct.password = m_password_edit->password();
			acct.url = m_url_field->m_field_edit->text();
			acct.email = m_email_field->m_field_edit->text();
			block blk;
			acct.to_block(&blk);
			::signetdev_write_id_async(NULL, &m_signetdev_cmd_token,
			                           m_acct->id,
			                           blk.data.size(),
			                           (const u8 *)blk.data.data(),
			                           (const u8 *)blk.mask.data());
		}
		break;
		case SIGNETDEV_CMD_WRITE_ID:
			m_acct->acct_name = m_account_name_edit->text();
			m_acct->user_name = m_username_field->m_field_edit->text();
			m_acct->password = m_password_edit->password();
			m_acct->url = m_url_field->m_field_edit->text();
			m_acct->email = m_email_field->m_field_edit->text();
			emit accountChanged(m_acct->id);
			m_save_button->setDisabled(true);
			m_undo_changes_button->setDisabled(true);
			break;
		}
#endif
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

OpenGeneric::~OpenGeneric()
{
}

void OpenGeneric::setGenericValues()
{
	m_genericNameEdit->setText(m_generic->name);
	for (auto genericField : m_generic->fields) {
		bool matched = false;
		for (auto typeField : m_typeFields) {
			if (typeField->name() == genericField.name) {
				typeField->setText(genericField.value);
				matched = true;
				break;
			}
		}
		for (auto typeField : m_extraFields) {
			if (typeField->name() == genericField.name) {
				typeField->setText(genericField.value);
				matched = true;
				break;
			}
		}
		if (!matched) {
			DatabaseField *typeField = new DatabaseField(genericField.name, 140);
			typeField->setText(genericField.value);
			m_extraFields.push_back(typeField);
		}
	}
}

void OpenGeneric::undoChangesUI()
{
	setGenericValues();
	m_saveButton->setDisabled(true);
	m_undoChangesButton->setDisabled(true);
}

void OpenGeneric::textEdited(QString s)
{
	Q_UNUSED(s);
	m_saveButton->setDisabled(false);
	m_undoChangesButton->setDisabled(false);
}

void OpenGeneric::closePressed()
{
	close();
}

void OpenGeneric::savePressed()
{
	m_buttonWaitDialog = new ButtonWaitDialog( "Open " + m_typeDesc->name,
	        QString("save changes to " + m_typeDesc->name.toLower() + " \"") + m_genericNameEdit->text() + QString("\""),
	        this);
	connect(m_buttonWaitDialog, SIGNAL(finished(int)), this, SLOT(saveGenericFinished(int)));
	m_buttonWaitDialog->show();

	::signetdev_open_id_async(NULL, &m_signetdevCmdToken, m_generic->id);
}

void OpenGeneric::saveGenericFinished(int code)
{
	if (code != QMessageBox::Ok) {
		::signetdev_cancel_button_wait();
	}
	m_buttonWaitDialog->deleteLater();
	m_buttonWaitDialog = NULL;
}
