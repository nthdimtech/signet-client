#ifndef EDITACCOUNT_H
#define EDITACCOUNT_H

#include <QDialog>
#include "signetapplication.h"

class QLineEdit;
class QPushButton;
class ButtonWaitDialog;
class QString;
class CommThread;
class DatabaseField;
class PasswordEdit;
class QLabel;
struct account;
struct block;

struct signetdevCmdRespInfo;
class GenericFieldsEditor;

class EditAccount : public QDialog
{
	Q_OBJECT
	account *m_acct;
	ButtonWaitDialog *m_buttonDialog;
	QLineEdit *m_accountNameEdit;
	QLabel *m_accountNameWarning;
	DatabaseField *m_usernameField;
	PasswordEdit *m_passwordEdit;
	DatabaseField *m_urlField;
	DatabaseField *m_emailField;
	int m_signetdevCmdToken;
	QPushButton *m_saveButton;
	QPushButton *m_browseUrlButton;
	QPushButton *m_undoChangesButton;
	void setAccountValues();
	GenericFieldsEditor *m_genericFieldsEditor;
	bool m_settingFields;
	void closeEvent(QCloseEvent *);
	bool m_changesMade;
	bool m_closeOnSave;
public:
	EditAccount(account *acct, QWidget *parent = 0);
	virtual ~EditAccount();
signals:
	void abort();
	void accountChanged(int id);
public slots:
	void signetdevCmdResp(signetdevCmdRespInfo info);
	void editAccountFinished(int);
	void accountNameEdited();
	void browseUrl();
	void textEdited();
	void savePressed();
	void undoChangesUi();
};

#endif // EDITACCOUNT_H
