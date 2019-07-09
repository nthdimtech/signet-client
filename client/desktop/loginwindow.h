#ifndef LOGINWINDOW_H
#define LOGINWINDOW_H

#include <QWidget>
#include <QThread>
#include "signetapplication.h"

class QLabel;
class QLineEdit;
class CommThread;
class QPushButton;
class KeyGeneratorThread;
class MainWindow;

struct signetdevCmdRespInfo;

class LoginWindow : public QWidget
{
	Q_OBJECT
        MainWindow *m_parent;
	QLineEdit *m_passwordInput;
	QLabel *m_incorrectPassword;
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
	void doLogin(void);
        void passwordTextEdited(QString);
private slots:
        void buttonWaitFinished();
};

#endif // LOGINWINDOW_H
