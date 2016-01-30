#ifndef LOGINWINDOW_H
#define LOGINWINDOW_H

#include <QWidget>
#include <QThread>
#include "signetapplication.h"

class QLabel;
class QLineEdit;
class ButtonWaitDialog;
class CommThread;
class QPushButton;
class KeyGeneratorThread;

struct signetdevCmdRespInfo;

class LoginWindow : public QWidget
{
	Q_OBJECT
	QLineEdit *m_passwordInput;
	QLabel *m_incorrectPassword;
	ButtonWaitDialog *m_buttonWait;
	QPushButton *m_loginButton;
	KeyGeneratorThread *m_keyGenerator;
	QLabel *m_preparingLabel;
	bool m_loggingIn;
	bool m_preparingLogin;
	void showEvent(QShowEvent *event);
	void updateWidgetState();
	int m_signetdevCmdToken;
public:
	explicit LoginWindow(QWidget *parent = 0);
	~LoginWindow();
signals:
	void abort();
	void login(QString password);
	void enterDeviceState(int state);
public slots:
	void signetdevCmdResp(signetdevCmdRespInfo info);
	void keyGenerated();
	void loginFinished(int);
	void doLogin(void);
	void passwordTextEdited(QString);
};

#endif // LOGINWINDOW_H
