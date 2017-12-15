#include "changemasterpassword.h"

extern "C" {
#include "signetdev/host/signetdev.h"
};

#include <QPushButton>
#include <QLineEdit>
#include <QBoxLayout>
#include <QLabel>
#include "buttonwaitdialog.h"
#include "signetapplication.h"

#include <algorithm>
#include <random>

ChangeMasterPassword::ChangeMasterPassword(QWidget *parent) :
	QDialog(parent),
	m_newPasswordWarningMessage(NULL),
	m_oldPasswordWarningMessage(NULL),
	m_oldPasswordEdit(NULL),
	m_newPasswordEdit(NULL),
	m_newPasswordRepeatEdit(NULL),
	m_buttonDialog(NULL),
	m_generatingOldKey(false),
	m_generatingNewKey(false),
	m_signetdevCmdToken(-1)
{
	std::random_device rd;
	m_newHashfn.resize(HASH_FN_SZ);
	m_newHashfn.data()[0] = 1;
	m_newHashfn.data()[1] = 12;
	m_newHashfn.data()[2] = 32;
	m_newHashfn.data()[3] = 0;
	m_newHashfn.data()[4] = 1;

	m_newSalt.resize(SALT_SZ_V2);
	for (int i = 0; i < (SALT_SZ_V2/4); i++) {
		*((uint32_t *)(m_newSalt.data() + (i*4))) = rd();
	}

	m_keyGenerator = new KeyGeneratorThread();
	QObject::connect(m_keyGenerator, SIGNAL(finished()), this, SLOT(keyGenerated()));

	SignetApplication *app = SignetApplication::get();

	connect(app, SIGNAL(signetdevCmdResp(signetdevCmdRespInfo)), this,
		SLOT(signetdevCmdResp(signetdevCmdRespInfo)));

	setWindowModality(Qt::WindowModal);

	setWindowTitle("Change Master Password");
	QBoxLayout *layout = new QBoxLayout(QBoxLayout::TopToBottom);

	QBoxLayout *old_password_layout = new QBoxLayout(QBoxLayout::LeftToRight);
	m_oldPasswordEdit = new QLineEdit();
	m_oldPasswordEdit->setEchoMode(QLineEdit::Password);
	old_password_layout->addWidget(new QLabel("Old password"));
	old_password_layout->addWidget(m_oldPasswordEdit);

	m_generatingKeys = new QLabel("Generating login keys...");
	m_generatingKeys->hide();

	m_oldPasswordWarningMessage = new QLabel();
	m_oldPasswordWarningMessage->hide();
	m_oldPasswordWarningMessage->setStyleSheet("QLabel { color : red; }");

	QBoxLayout *new_password_layout = new QBoxLayout(QBoxLayout::LeftToRight);
	m_newPasswordEdit = new QLineEdit();
	m_newPasswordEdit->setEchoMode(QLineEdit::Password);
	new_password_layout->addWidget(new QLabel("New password"));
	new_password_layout->addWidget(m_newPasswordEdit);

	QBoxLayout *new_password_repeat_layout = new QBoxLayout(QBoxLayout::LeftToRight);
	m_newPasswordRepeatEdit = new QLineEdit();
	m_newPasswordRepeatEdit->setEchoMode(QLineEdit::Password);
	new_password_repeat_layout->addWidget(new QLabel("New password (repeat) "));
	new_password_repeat_layout->addWidget(m_newPasswordRepeatEdit);

	m_newPasswordWarningMessage = new QLabel();
	m_newPasswordWarningMessage->hide();
	m_newPasswordWarningMessage->setStyleSheet("QLabel { color : red; }");

	m_changePasswordBtn = new QPushButton("Change password");
	m_changePasswordBtn->setAutoDefault(true);

	layout->addLayout(old_password_layout);
	layout->addWidget(m_oldPasswordWarningMessage);
	layout->addLayout(new_password_layout);
	layout->addLayout(new_password_repeat_layout);
	layout->addWidget(m_newPasswordWarningMessage);
	layout->addWidget(m_generatingKeys);
	layout->addWidget(m_changePasswordBtn);
	setLayout(layout);

	connect(m_changePasswordBtn, SIGNAL(pressed()), this, SLOT(changePasswordUi()));
	connect(m_newPasswordEdit, SIGNAL(textEdited(QString)), this, SLOT(newPasswordTextEdited(QString)));
	connect(m_newPasswordRepeatEdit, SIGNAL(textEdited(QString)), this, SLOT(newPasswordTextEdited(QString)));
	connect(m_oldPasswordEdit, SIGNAL(textEdited(QString)), this, SLOT(oldPasswordTextEdited(QString)));
}

void ChangeMasterPassword::keyGenerated()
{
	if (m_generatingOldKey) {
		m_generatingOldKey = false;
		m_generatingNewKey = true;
		m_oldKey = m_keyGenerator->getKey();
		int keyLength = SignetApplication::get()->getKeyLength();
		m_keyGenerator->setParams(this->m_newPasswordEdit->text(), m_newHashfn, m_newSalt, keyLength);
		m_keyGenerator->start();
	} else if (m_generatingNewKey) {
		m_generatingNewKey = false;
		m_newKey = m_keyGenerator->getKey();
		m_generatingKeys->hide();
		m_buttonDialog = new ButtonWaitDialog("Change Master Password", "change master password", this, true);
		connect(m_buttonDialog, SIGNAL(finished(int)), this, SLOT(changePasswordFinished(int)));
		m_buttonDialog->show();
		::signetdev_change_master_password(NULL, &m_signetdevCmdToken,
			(u8 *)m_oldKey.data(), m_oldKey.length(),
			(u8 *)m_newKey.data(), m_newKey.length(),
			(u8 *)m_newHashfn.data(), m_newHashfn.length(),
			(u8 *)m_newSalt.data(), m_newSalt.length());
	}
}

ChangeMasterPassword::~ChangeMasterPassword()
{
	m_generatingOldKey = false;
	m_generatingNewKey = false;
	m_keyGenerator->wait();
	m_keyGenerator->deleteLater();
	m_keyGenerator = NULL;
}

void ChangeMasterPassword::newPasswordTextEdited(QString s)
{
	Q_UNUSED(s);
	m_newPasswordWarningMessage->hide();
}

void ChangeMasterPassword::oldPasswordTextEdited(QString s)
{
	Q_UNUSED(s);
	m_oldPasswordWarningMessage->hide();
}

void ChangeMasterPassword::changePasswordUi()
{
	if (m_newPasswordEdit->text() != m_newPasswordRepeatEdit->text()) {
		m_newPasswordWarningMessage->setText("New passwords don't match, try again");
		m_newPasswordWarningMessage->show();
	} else {
		m_newPasswordEdit->setEnabled(false);
		m_oldPasswordEdit->setEnabled(false);
		m_changePasswordBtn->setEnabled(false);
		m_generatingKeys->show();
		m_generatingOldKey = true;
		m_generatingNewKey = false;
		SignetApplication *app = SignetApplication::get();
		QByteArray current_hashfn = app->getHashfn();
		QByteArray current_salt = app->getSalt();
		int keyLength = app->getKeyLength();
		m_keyGenerator->setParams(this->m_oldPasswordEdit->text(), current_hashfn, current_salt, keyLength);
		m_keyGenerator->start();
	}
}

void ChangeMasterPassword::changePasswordFinished(int code)
{
	if (code != QMessageBox::Ok) {
		::signetdev_cancel_button_wait();
	}
}

void ChangeMasterPassword::signetdevCmdResp(signetdevCmdRespInfo info)
{
	int code = info.resp_code;

	if (m_signetdevCmdToken != info.token) {
		return;
	}
	m_signetdevCmdToken = -1;
	if (m_buttonDialog) {
		m_buttonDialog->done(QMessageBox::Ok);
		m_buttonDialog->deleteLater();
		m_buttonDialog = NULL;
	}

	switch (code) {
	case OKAY: {
		QMessageBox *box = SignetApplication::messageBoxError(QMessageBox::Information, "Change Master Password", "Password change successful", this);;
		connect(box, SIGNAL(finished(int)), this, SLOT(close()));
		SignetApplication::get()->setHashfn(m_newHashfn);
		SignetApplication::get()->setSalt(m_newSalt);
	}
	break;
	case BAD_PASSWORD:
		m_oldPasswordWarningMessage->setText("Old passord incorrect, try again");
		m_oldPasswordWarningMessage->show();
		m_newPasswordEdit->setEnabled(true);
		m_oldPasswordEdit->setEnabled(true);
		m_changePasswordBtn->setEnabled(true);
		break;
	case BUTTON_PRESS_TIMEOUT:
	case BUTTON_PRESS_CANCELED:
		m_newPasswordEdit->setEnabled(true);
		m_oldPasswordEdit->setEnabled(true);
		m_changePasswordBtn->setEnabled(true);
		break;
	case SIGNET_ERROR_DISCONNECT:
	case SIGNET_ERROR_QUIT:
		close();
		break;
	default: {
		emit abort();
		close();
	}
	break;
	}
}
