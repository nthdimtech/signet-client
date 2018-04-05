#include "resetdevice.h"

#include <QBoxLayout>
#include <QPushButton>
#include <QMessageBox>
#include <QLabel>
#include <QDebug>
#include <QDialog>
#include <QCoreApplication>
#include <QProgressBar>
#include <QStackedWidget>
#include <QByteArray>
#include <algorithm>
#include <random>

#include "buttonwaitdialog.h"

#include "mainwindow.h"

#include "signetapplication.h"
#include "keygeneratorthread.h"

extern "C" {
#include "signetdev/host/signetdev.h"
}

ResetDevice::ResetDevice(bool destructive, QWidget *parent) :
	QDialog(parent),
	m_buttonPrompt(NULL),
	m_warningMessage(NULL),
	m_passwordEdit_1(NULL),
	m_passwordEdit_2(NULL),
	m_passwordEdit_1Label(NULL),
	m_passwordEdit_2Label(NULL),
	m_passwordWarningMessage(NULL),
	m_writeProgressLabel(NULL),
	m_writeProgressBar(NULL),
	m_randomDataProgressLabel(NULL),
	m_randomDataProgressBar(NULL),
	m_resetButton(NULL),
	m_signetdevCmdToken(-1),
	m_destructive(destructive)
{
	m_keyGenerator = new KeyGeneratorThread();
	QObject::connect(m_keyGenerator, SIGNAL(finished()), this, SLOT(keyGenerated()));

	setWindowModality(Qt::WindowModal);

	setWindowTitle("Initialize device");

	SignetApplication *app = SignetApplication::get();
	connect(app, SIGNAL(signetdevCmdResp(signetdevCmdRespInfo)),
		this, SLOT(signetdevCmdResp(signetdevCmdRespInfo)));

	connect(app, SIGNAL(signetdevGetProgressResp(signetdevCmdRespInfo, signetdev_get_progress_resp_data)),
		this, SLOT(signetdevGetProgressResp(signetdevCmdRespInfo, signetdev_get_progress_resp_data)));


	QBoxLayout *layout = new QBoxLayout(QBoxLayout::TopToBottom);
	layout->setAlignment(Qt::AlignTop);
	m_passwordEdit_1Label = new QLabel("Master password");
	m_passwordEdit_2Label = new QLabel("Master password (repeat)");
	m_generatingKeyLabel =  new QLabel("Generating login key...");
	m_passwordEdit_1 = new QLineEdit();
	m_passwordEdit_2 = new QLineEdit();
	m_passwordEdit_1->setEchoMode(QLineEdit::Password);
	m_passwordEdit_2->setEchoMode(QLineEdit::Password);

	connect(m_passwordEdit_1, SIGNAL(textEdited(QString)), this, SLOT(passwordTextChanged(QString)));
	connect(m_passwordEdit_2, SIGNAL(textEdited(QString)), this, SLOT(passwordTextChanged(QString)));

	m_resetButton = new QPushButton(destructive ? "Reset" : "Initialize");
	QPushButton *cancel = new QPushButton("Cancel");
	QObject::connect(m_resetButton, SIGNAL(pressed()), this, SLOT(reset()));
	QObject::connect(cancel, SIGNAL(pressed()), this, SLOT(close()));

	m_writeProgressLabel = new QLabel("Clearing device...");
	m_writeProgressLabel->hide();
	m_writeProgressBar = new QProgressBar();
	m_writeProgressBar->setRange(0, 100);
	m_writeProgressBar->setTextVisible(true);
	m_writeProgressBar->hide();

	m_randomDataProgressLabel = new QLabel("Collecting random data...");
	m_randomDataProgressLabel->hide();
	m_randomDataProgressBar = new QProgressBar();
	m_randomDataProgressBar->setRange(0, 100);
	m_randomDataProgressBar->setTextVisible(true);
	m_randomDataProgressBar->hide();

	if (destructive) {
		m_warningMessage = new QLabel("Warning, this permenantly erase the contents of your device.");
		m_warningMessage->setStyleSheet("QLabel { color : red; }");
	} else {
		m_warningMessage = NULL;
	}
	m_passwordWarningMessage = new QLabel();
	m_passwordWarningMessage->hide();
	m_generatingKeyLabel->hide();
	if (m_destructive) {
		layout->addWidget(m_warningMessage);
	}
	layout->addWidget(m_passwordEdit_1Label);
	layout->addWidget(m_passwordEdit_1);
	layout->addWidget(m_passwordEdit_2Label);
	layout->addWidget(m_passwordEdit_2);
	layout->addWidget(m_passwordWarningMessage);
	layout->addWidget(m_writeProgressLabel);
	layout->addWidget(m_writeProgressBar);
	layout->addWidget(m_randomDataProgressLabel);
	layout->addWidget(m_randomDataProgressBar);
	layout->addWidget(m_generatingKeyLabel);
	layout->addWidget(m_resetButton);
	layout->addWidget(cancel);

	setLayout(layout);
}

void ResetDevice::passwordTextChanged(QString)
{
	m_passwordWarningMessage->hide();
}

void ResetDevice::keyGenerated()
{
	std::random_device rd;
	m_generatingKeyLabel->hide();
	m_buttonPrompt = new ButtonWaitDialog(m_destructive ? "Reset device" : "Initialize device", m_destructive ? "reset device" : "initialize device", this, true);
	connect(m_buttonPrompt, SIGNAL(finished(int)), this, SLOT(resetButtonPromptFinished(int)));
	m_buttonPrompt->show();

	u8 rand_data[INIT_RAND_DATA_SZ];
	int sz = sizeof(rand_data);
	for (int i = 0; i < sz; i += sizeof(unsigned int)) {
		unsigned int x = rd();
		memcpy(rand_data + i, &x,
		       std::min((int)sizeof(unsigned int), sz - i));
	}
	::signetdev_begin_initialize_device(NULL, &m_signetdevCmdToken,
		(const u8 *)m_keyGenerator->getKey().data(), m_keyGenerator->getKey().length(),
		(const u8 *)m_keyGenerator->getHashfn().data(), m_keyGenerator->getHashfn().length(),
		(const u8 *)m_keyGenerator->getSalt().data(), m_keyGenerator->getSalt().length(),
		rand_data, sizeof(rand_data));
}

void ResetDevice::reset()
{
	if (m_passwordEdit_1->text() != m_passwordEdit_2->text()) {
		m_passwordWarningMessage->setStyleSheet("QLabel { color : red; }");
		m_passwordWarningMessage->setText("Passwords don't match, try again");
		m_passwordWarningMessage->show();
		m_passwordEdit_1->setText("");
		m_passwordEdit_2->setText("");
		return;
	}
	m_passwd = m_passwordEdit_1->text();
	m_passwordEdit_1->setDisabled(true);
	m_passwordEdit_2->setDisabled(true);
	m_resetButton->setDisabled(true);
	m_generatingKeyLabel->show();
	std::random_device rd;

	QByteArray hashfn;
	QByteArray salt;

	hashfn.resize(HASH_FN_SZ);
	hashfn.data()[0] = 1;
	hashfn.data()[1] = 12;
	hashfn.data()[2] = 32;
	hashfn.data()[3] = 0;
	hashfn.data()[4] = 1;

	salt.resize(SALT_SZ_V2);
	for (int i = 0; i < (SALT_SZ_V2/4); i++) {
		*((uint32_t *)(salt.data() + (i*4))) = rd();
	}
	m_keyGenerator->setParams(m_passwd, hashfn, salt, AES_256_KEY_SIZE);
	m_keyGenerator->start();
}

void ResetDevice::signetdevCmdResp(signetdevCmdRespInfo info)
{
	int code = info.resp_code;

	if (m_signetdevCmdToken != info.token) {
		return;
	}
	m_signetdevCmdToken = -1;

	if (m_buttonPrompt)
		m_buttonPrompt->done(QMessageBox::Ok);

	switch (code) {
	case OKAY:
		m_writeProgressLabel->show();
		m_writeProgressBar->show();
		m_randomDataProgressLabel->show();
		m_randomDataProgressBar->show();
		if (m_warningMessage) {
			m_warningMessage->hide();
		}
		::signetdev_get_progress(NULL, &m_signetdevCmdToken, 0, INITIALIZING);
		break;
	case BUTTON_PRESS_TIMEOUT:
	case BUTTON_PRESS_CANCELED:
		m_passwordEdit_1->setDisabled(false);
		m_passwordEdit_2->setDisabled(false);
		m_resetButton->setDisabled(false);
		break;
	case SIGNET_ERROR_DISCONNECT:
	case SIGNET_ERROR_QUIT:
		break;
	default: {
		emit abort();
	}
	break;
	}
}

void ResetDevice::signetdevGetProgressResp(signetdevCmdRespInfo info, signetdev_get_progress_resp_data data)
{
	if (info.token != m_signetdevCmdToken) {
		return;
	}
	m_signetdevCmdToken = -1;
	int resp_code = info.resp_code;

	switch (resp_code) {
	case OKAY:
		if (data.n_components < 2) {
			m_writeProgressBar->setRange(0, data.total_progress_maximum);
			m_writeProgressBar->setValue(data.total_progress);
			m_writeProgressBar->update();
		} else {
			m_writeProgressBar->setRange(0, data.progress_maximum[0]);
			m_writeProgressBar->setValue(data.progress[0]);
			m_writeProgressBar->update();
			m_randomDataProgressBar->setRange(0, data.progress_maximum[1]);
			m_randomDataProgressBar->setValue(data.progress[1]);
			m_randomDataProgressBar->update();
		}
		::signetdev_get_progress(NULL, &m_signetdevCmdToken, data.total_progress, INITIALIZING);
		break;
	case INVALID_STATE: {
		SignetApplication *app = SignetApplication::get();
		app->setSalt(m_keyGenerator->getSalt());
		app->setHashfn(m_keyGenerator->getHashfn());
		QMessageBox * box = new QMessageBox(QMessageBox::Information,
						    m_destructive ? "Reset device" : "Initialize device", m_destructive ? "Device reset successfully" : "Device initialized successfully",
						    QMessageBox::Ok,
						    this);
		connect(box, SIGNAL(finished(int)), box, SLOT(deleteLater()));
		connect(box, SIGNAL(finished(int)), this, SLOT(resetDeviceFinalize()));
		box->show();
	}
	break;
	case SIGNET_ERROR_DISCONNECT:
	case SIGNET_ERROR_QUIT:
		close();
		return;
	default:
		abort();
		return;
	}
}

void ResetDevice::resetButtonPromptFinished(int code)
{
	if (code != QMessageBox::Ok) {
		::signetdev_cancel_button_wait();
	}
	m_buttonPrompt->deleteLater();
	m_buttonPrompt = NULL;
}

void ResetDevice::resetDeviceFinalize()
{
	emit enterDeviceState(SignetApplication::STATE_LOGGED_OUT);
	close();
}
