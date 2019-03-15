#include "cleartextpasswordeditor.h"
#include "linefieldedit.h"
#include "buttonwaitdialog.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QCloseEvent>

#include "passwordedit.h"

cleartextPasswordEditor::cleartextPasswordEditor(int index, struct cleartext_pass *p, QWidget *parent) :
	QDialog(parent),
	m_index(index),
	m_pass(p),
	m_signetdevCmdToken(-1),
	m_buttonWaitDialog(NULL),
	m_changesMade(false)
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
	m_saveButton = new QPushButton("Save");
	QPushButton *closeButton = new QPushButton("Close");
	QHBoxLayout *buttons = new QHBoxLayout();
	buttons->addWidget(m_saveButton);
	buttons->addWidget(closeButton);

	m_nameEdit->fromString(QString::fromUtf8(p->name_utf8));
	m_passwordEdit->setPassword(QString::fromUtf8(p->password_utf8));

	m_saveButton->setDisabled(true);
	connect(m_nameEdit, SIGNAL(edited()), this, SLOT(edited()));
	connect(m_passwordEdit, SIGNAL(textEdited(QString)), this, SLOT(edited()));
	connect(closeButton, SIGNAL(clicked(bool)), this, SLOT(close()));
	connect(m_saveButton, SIGNAL(pressed()), this, SLOT(savePressed()));
	l->addLayout(buttons);
	setLayout(l);
}

void cleartextPasswordEditor::edited()
{
	m_changesMade = true;
	m_saveButton->setDisabled(false);
}

void cleartextPasswordEditor::savePressed()
{
	QString name = m_nameEdit->toString();
	QString password = m_passwordEdit->password();
	QByteArray nameUTF8 = name.toUtf8();
	QByteArray passwordUTF8 = password.toUtf8();

	m_passNext = *m_pass;

	m_passNext.format = 1;
	m_passNext.scancode_entries = 0;

	if (nameUTF8.size() > (CLEARTEXT_PASS_NAME_SIZE - 1)) {
		SignetApplication::messageBoxWarn("Save password slot", "Name must be less than " +
						  QString::number((CLEARTEXT_PASS_NAME_SIZE - 1)) +
						  " characters", this);
		return;
	}

	if (passwordUTF8.size() > (CLEARTEXT_PASS_PASS_SIZE - 1)) {
		SignetApplication::messageBoxWarn("Save password slot", "Password must be less than " +
						  QString::number((CLEARTEXT_PASS_PASS_SIZE - 1)) +
						  " characters", this);
		return;
	}

	strncpy(m_passNext.name_utf8, nameUTF8.data(), CLEARTEXT_PASS_NAME_SIZE - 1);
	strncpy(m_passNext.password_utf8, passwordUTF8.data(), CLEARTEXT_PASS_PASS_SIZE - 1);

	int out_len = CLEARTEXT_PASS_SCANCODE_ENTRIES;

	int rc = signetdev_to_scancodes_w((const u16 *)password.data(), password.length(), (u16 *)m_passNext.scancodes, &out_len);

	switch (rc) {
	case 1:
		SignetApplication::messageBoxWarn("Save password slot",
		                                  "Password is too long",
		                                  this);
		return;
	case 2:
		SignetApplication::messageBoxWarn("Save password slot",
		                                  "Password contains characters not found in keyboard layout",
		                                  this);
		return;
	}

	m_passNext.scancode_entries = out_len;
	m_buttonWaitDialog = new ButtonWaitDialog("Save password slot", "save password slot", this, false);
	connect(m_buttonWaitDialog, SIGNAL(finished(int)), this, SLOT(buttonWaitFinished(int)));
	m_buttonWaitDialog->show();
	::signetdev_write_cleartext_password(nullptr, &m_signetdevCmdToken, m_index, &m_passNext);
}

void cleartextPasswordEditor::signetdevCmdResp(signetdevCmdRespInfo info)
{
	if (info.token != m_signetdevCmdToken) {
		return;
	}
	if (m_buttonWaitDialog) {
		m_buttonWaitDialog->done(0);
	}
	if (info.resp_code == OKAY) {
		*m_pass = m_passNext;
		m_changesMade = false;
		close();
	} else {
		SignetApplication::messageBoxError(QMessageBox::Critical,
		                                   "Save password slot",
		                                   "Failed to save password slot",
		                                   this);
	}
}

void cleartextPasswordEditor::buttonWaitFinished(int result)
{
	if (result != QMessageBox::Ok) {
		::signetdev_cancel_button_wait();
	}
	if (m_buttonWaitDialog)
		m_buttonWaitDialog->deleteLater();
	m_buttonWaitDialog = nullptr;
}

void cleartextPasswordEditor::closeEvent(QCloseEvent *event)
{
	if (m_changesMade) {
		QMessageBox *box = new QMessageBox(QMessageBox::Question, windowTitle(),
		                                   "You have made changes. Do you want to save them",
		                                   QMessageBox::Yes |
		                                   QMessageBox::No,
		                                   this);
		connect(box, SIGNAL(finished(int)), this, SLOT(saveOnCloseDialogFinished(int)));
		box->setWindowModality(Qt::WindowModal);
		box->setAttribute(Qt::WA_DeleteOnClose);
		box->show();
		event->ignore();
		return;
	}
	event->accept();
}

void cleartextPasswordEditor::saveOnCloseDialogFinished(int rc)
{
	if (rc == QMessageBox::Yes) {
		savePressed();
	} else {
		m_changesMade = false;
		done(0);
	}
}
