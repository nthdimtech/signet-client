#include "cleartextpasswordeditor.h"
#include "linefieldedit.h"
#include "buttonwaitdialog.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QLabel>

#include "passwordedit.h"

cleartextPasswordEditor::cleartextPasswordEditor(int index, struct cleartext_pass *p, QWidget *parent) :
	QDialog(parent),
	m_index(index),
	m_pass(p),
	m_signetdevCmdToken(-1),
	m_buttonWaitDialog(NULL)
{
	SignetApplication *app = SignetApplication::get();
	connect(app, SIGNAL(signetdevCmdResp(signetdevCmdRespInfo)),
		this, SLOT(signetdevCmdResp(signetdevCmdRespInfo)));
	setWindowModality(Qt::WindowModal);
	QVBoxLayout *l = new QVBoxLayout();
	QLabel *heading = new QLabel("Password slot #" + QString::number(index + 1));
	l->addWidget(heading);
	m_nameEdit = new lineFieldEdit("Name", false);
	m_passwordEdit = new PasswordEdit();
	l->addWidget(m_nameEdit->widget());
	l->addWidget(m_passwordEdit);
	m_deleteButton = new QPushButton("Delete");
	m_saveButton = new QPushButton("Save");
	QPushButton *closeButton = new QPushButton("Close");
	QHBoxLayout *buttons = new QHBoxLayout();
	buttons->addWidget(m_saveButton);
	buttons->addWidget(m_deleteButton);
	buttons->addWidget(closeButton);

	m_nameEdit->fromString(QString::fromUtf8(p->name_utf8));
	m_passwordEdit->setPassword(QString::fromUtf8(p->password_utf8));

	m_saveButton->setDisabled(true);
	connect(m_nameEdit, SIGNAL(edited()), this, SLOT(edited()));
	connect(m_passwordEdit, SIGNAL(textEdited(QString)), this, SLOT(edited()));
	connect(closeButton, SIGNAL(clicked(bool)), this, SLOT(close()));
	connect(m_saveButton, SIGNAL(pressed()), this, SLOT(savePressed()));
	connect(m_deleteButton, SIGNAL(pressed()), this, SLOT(deletePressed()));
	l->addLayout(buttons);
	setLayout(l);
}

void cleartextPasswordEditor::edited()
{
	m_saveButton->setDisabled(false);
}

void cleartextPasswordEditor::deletePressed()
{
	m_pass->format = 0xff;
	m_pass->scancode_entries = 0;
	memset(m_pass->name_utf8, 0, CLEARTEXT_PASS_NAME_SIZE);
	memset(m_pass->password_utf8, 0, CLEARTEXT_PASS_PASS_SIZE);
	memset(m_pass->scancodes, 0, CLEARTEXT_PASS_SCANCODE_ENTRIES*2);
	::signetdev_write_cleartext_password(NULL, &m_signetdevCmdToken, m_index, m_pass);
	m_buttonWaitDialog = new ButtonWaitDialog("Delete password slot", "delete password slot", this, false);
	connect(m_buttonWaitDialog, SIGNAL(finished(int)), this, SLOT(buttonWaitFinished(int)));
	m_buttonWaitDialog->show();
}

void cleartextPasswordEditor::savePressed()
{
	QString name = m_nameEdit->toString();
	QString password = m_passwordEdit->password();
	QByteArray nameUTF8 = name.toUtf8();
	QByteArray passwordUTF8 = password.toUtf8();

	m_pass->format = 1;
	m_pass->scancode_entries = 0;

	QMessageBox *warn;

	if (nameUTF8.size() > (CLEARTEXT_PASS_NAME_SIZE - 1)) {
		warn = SignetApplication::messageBoxWarn("Save password slot", "Name must be less than " +
						  QString::number((CLEARTEXT_PASS_NAME_SIZE - 1)) +
						  " characters", this);
		warn->exec();
		return;
	}

	if (passwordUTF8.size() > (CLEARTEXT_PASS_PASS_SIZE - 1)) {
		warn = SignetApplication::messageBoxWarn("Save password slot", "Password must be less than " +
						  QString::number((CLEARTEXT_PASS_PASS_SIZE - 1)) +
						  " characters", this);
		warn->exec();
		return;
	}

	strncpy(m_pass->name_utf8, nameUTF8.data(), CLEARTEXT_PASS_NAME_SIZE - 1);
	strncpy(m_pass->password_utf8, passwordUTF8.data(), CLEARTEXT_PASS_PASS_SIZE - 1);

	int out_len = CLEARTEXT_PASS_SCANCODE_ENTRIES;

	int rc = signetdev_to_scancodes_w((const u16 *)password.data(), password.length(), (u16 *)m_pass->scancodes, &out_len);

	switch (rc) {
	case 1:
		warn = SignetApplication::messageBoxWarn("Save password slot",
							 "Password is too long",
							 this);
		warn->exec();
		return;
	case 2:
		warn = SignetApplication::messageBoxWarn("Save password slot",
							 "Password contains characters not found in keyboard layout",
							 this);
		warn->exec();
		return;
	}

	m_pass->scancode_entries = out_len;
	m_buttonWaitDialog = new ButtonWaitDialog("Save password slot", "save password slot", this, false);
	connect(m_buttonWaitDialog, SIGNAL(finished(int)), this, SLOT(buttonWaitFinished(int)));
	m_buttonWaitDialog->show();
	::signetdev_write_cleartext_password(NULL, &m_signetdevCmdToken, m_index, m_pass);
}

void cleartextPasswordEditor::signetdevCmdResp(signetdevCmdRespInfo info)
{
	if (info.token != m_signetdevCmdToken) {
		return;
	}
	if (m_buttonWaitDialog) {
		m_buttonWaitDialog->done(0);
	}
	close();
}

void cleartextPasswordEditor::buttonWaitFinished(int result)
{
	if (result != QMessageBox::Ok) {
		::signetdev_cancel_button_wait();
	}
	if (m_buttonWaitDialog)
		m_buttonWaitDialog->deleteLater();
	m_buttonWaitDialog = NULL;
}
