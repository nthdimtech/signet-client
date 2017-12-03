#include "loginwindow.h"
#include <QtGui>
#include <QDebug>
#include <QBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QMessageBox>
#include <QLineEdit>
#include <QThread>

#include "buttonwaitdialog.h"
#include "mainwindow.h"
#include "signetapplication.h"
#include "keygeneratorthread.h"

extern "C" {
#include "signetdev/host/signetdev.h"
};

LoginWindow::LoginWindow(QWidget *parent) : QWidget(parent),
	m_buttonWait(NULL),
	m_loggingIn(false),
	m_preparingLogin(false),
	m_signetdevCmdToken(-1)
{
	m_keyGenerator = new KeyGeneratorThread();

	SignetApplication *app = SignetApplication::get();

	QObject::connect(m_keyGenerator, SIGNAL(finished()), this, SLOT(keyGenerated()));
	QObject::connect(app, SIGNAL(signetdevCmdResp(signetdevCmdRespInfo)),
			 this, SLOT(signetdevCmdResp(signetdevCmdRespInfo)));

	QLayout *top_layout = new QBoxLayout(QBoxLayout::TopToBottom);
	QLayout *layout = new QBoxLayout(QBoxLayout::LeftToRight);
	QLabel *password_label = new QLabel("Password");

	m_incorrectPassword = new QLabel("Incorrect password");
	m_incorrectPassword->setStyleSheet("QLabel { color : red; }");
	m_incorrectPassword->hide();
	m_passwordInput = new QLineEdit();
	m_passwordInput->setEchoMode(QLineEdit::Password);
	m_loginButton = new QPushButton("Unlock");
	m_loginButton->setAutoDefault(true);

	m_preparingLabel = new QLabel("Generating authorization key...");
	m_preparingLabel->hide();

	layout->addWidget(password_label);
	layout->addWidget(m_passwordInput);
	top_layout->addWidget(new QLabel("Enter your master password"));
	top_layout->addItem(layout);
	top_layout->addWidget(m_incorrectPassword);
	top_layout->addWidget(m_preparingLabel);
	top_layout->addWidget(m_loginButton);
	connect(m_loginButton, SIGNAL(pressed(void)),
		this, SLOT(doLogin(void)));
	connect(m_passwordInput, SIGNAL(returnPressed(void)),
		this, SLOT(doLogin(void)));
	connect(m_passwordInput, SIGNAL(textEdited(QString)), this, SLOT(passwordTextEdited(QString)));
	top_layout->setAlignment(Qt::AlignTop);
	setLayout(top_layout);
}

LoginWindow::~LoginWindow()
{
	m_preparingLogin = false;
	m_loggingIn = false;
	m_keyGenerator->wait();
	m_keyGenerator->deleteLater();
	m_keyGenerator = NULL;
}

void LoginWindow::keyGenerated()
{
	if (m_loggingIn) {
		if (m_preparingLogin) {
			m_preparingLogin = false;
			updateWidgetState();
			m_buttonWait = new ButtonWaitDialog("Unlock","unlock", this);
			connect(m_buttonWait, SIGNAL(finished(int)), this, SLOT(loginFinished(int)));
			m_buttonWait->show();
			::signetdev_login_async(NULL, &m_signetdevCmdToken,
						(u8 *)m_keyGenerator->getKey().data(),
						m_keyGenerator->getKey().length());
		}
	}
}

void LoginWindow::updateWidgetState()
{
	m_loginButton->setEnabled(!m_preparingLogin);
	m_passwordInput->setEnabled(!m_loggingIn);

	if (m_preparingLogin) {
		m_preparingLabel->show();
	} else {
		m_preparingLabel->hide();
	}
}

void LoginWindow::showEvent(QShowEvent *event)
{
	Q_UNUSED(event);
	this->m_passwordInput->setFocus();
}

void LoginWindow::passwordTextEdited(QString)
{
	m_incorrectPassword->hide();
}

void LoginWindow::doLogin()
{
	m_loggingIn = true;
	m_preparingLogin = true;
	updateWidgetState();
	const QByteArray &current_hashfn = SignetApplication::get()->getHashfn();
	const QByteArray &current_salt = SignetApplication::get()->getSalt();
	int keyLength = SignetApplication::get()->getKeyLength();
	m_keyGenerator->setParams(m_passwordInput->text(), current_hashfn, current_salt, keyLength);
	m_keyGenerator->start();
}

void LoginWindow::loginFinished(int code)
{
	if (code != QMessageBox::Ok) {
		::signetdev_cancel_button_wait();
		m_loggingIn = false;
		m_preparingLogin = false;
		updateWidgetState();
	}
	m_buttonWait->deleteLater();
	m_buttonWait = NULL;
}

void LoginWindow::signetdevCmdResp(signetdevCmdRespInfo info)
{
	if (info.token != m_signetdevCmdToken) {
		return;
	}
	m_signetdevCmdToken = -1;
	int resp_code = info.resp_code;

	m_loggingIn = false;
	m_preparingLogin = false;
	updateWidgetState();
	if (m_buttonWait)
		m_buttonWait->done(QMessageBox::Ok);

	switch (resp_code) {
	case OKAY:
		emit enterDeviceState(MainWindow::STATE_LOGGED_IN_LOADING_ACCOUNTS);
		break;
	case BAD_PASSWORD:
		m_incorrectPassword->show();
		m_passwordInput->setText("");
		m_passwordInput->setFocus();
		break;
	case BUTTON_PRESS_CANCELED:
	case BUTTON_PRESS_TIMEOUT:
	case SIGNET_ERROR_DISCONNECT:
	case SIGNET_ERROR_QUIT:
		break;
	default:
		abort();
		return;
	}
}
