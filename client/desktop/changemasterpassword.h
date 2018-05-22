#ifndef CHANGEMASTERPASSWORD_H
#define CHANGEMASTERPASSWORD_H

#include <QDialog>
#include <QString>

#include "keygeneratorthread.h"
#include "signetapplication.h"

class CommThread;
class QLabel;
class QLineEdit;
class ButtonWaitDialog;
class QPushButton;
struct signetdevCmdRespInfo;

class ChangeMasterPassword : public QDialog
{
	Q_OBJECT
	QLabel *m_newPasswordWarningMessage;
	QLabel *m_oldPasswordWarningMessage;
	QLabel *m_generatingKeys;
	QLineEdit *m_oldPasswordEdit;
	QLineEdit *m_newPasswordEdit;
	QLineEdit *m_newPasswordRepeatEdit;
	ButtonWaitDialog *m_buttonDialog;
	QPushButton *m_changePasswordBtn;
	KeyGeneratorThread *m_keyGenerator;
	bool m_generatingOldKey;
	bool m_generatingNewKey;
	QByteArray m_oldKey;
	QByteArray m_newKey;
	QByteArray m_newHashfn;
	QByteArray m_newSalt;
	int m_signetdevCmdToken;
public:
	ChangeMasterPassword(QWidget *parent = 0);
	virtual ~ChangeMasterPassword();
signals:
	void abort();
public slots:
	void signetdevCmdResp(signetdevCmdRespInfo info);
	void keyGenerated();
	void changePasswordFinished(int);
	void changePasswordUi();
	void newPasswordTextEdited(QString);
	void oldPasswordTextEdited(QString);
};

#endif // CHANGEMASTERPASSWORD_H
