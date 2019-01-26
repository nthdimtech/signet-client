#ifndef RESET_DEVICE_H
#define RESET_DEVICE_H

#include <QWidget>
#include <QLineEdit>
#include <QDialog>
#include <QProgressBar>
#include <QMessageBox>
#include "signetapplication.h"

class QLabel;
class CommThread;
class KeyGeneratorThread;
class QSpinBox;

struct signetdevCmdRespInfo;

class ResetDevice : public QDialog
{
	Q_OBJECT
public:
	explicit ResetDevice(bool destructive, QWidget *parent = nullptr);
private:
	QDialog *m_buttonPrompt;
	QLabel *m_warningMessage;
	QLineEdit *m_passwordEdit_1;
	QLineEdit *m_passwordEdit_2;
	QLabel *m_passwordEdit_1Label;
	QLabel *m_passwordEdit_2Label;
	QLabel *m_generatingKeyLabel;
	QLabel *m_passwordWarningMessage;
	QLabel *m_writeProgressLabel;
	QProgressBar *m_writeProgressBar;
	QLabel *m_randomDataProgressLabel;
	QProgressBar *m_randomDataProgressBar;
	QString m_passwd;
	QPushButton *m_resetButton;
	KeyGeneratorThread *m_keyGenerator;
	QLabel *m_securityLevelComment;
	QSpinBox *m_authSecurityLevel;

	int m_signetdevCmdToken;
	bool m_destructive;
signals:
	void abort();
	void enterDeviceState(int);
public slots:
	void signetdevCmdResp(signetdevCmdRespInfo info);
	void signetdevGetProgressResp(signetdevCmdRespInfo info, signetdev_get_progress_resp_data data);
	void resetDeviceFinalize();
	void keyGenerated();
	void resetButtonPromptFinished(int);
	void passwordTextChanged(QString);
	void reset();
};

#endif // RESET_DEVICE_H
