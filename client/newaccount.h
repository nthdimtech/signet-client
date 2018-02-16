#ifndef NEWACCOUNT_H
#define NEWACCOUNT_H

#include <QMainWindow>
#include <QLineEdit>
#include <QMessageBox>
#include "signetapplication.h"

#include "account.h"

class QLineEdit;
class QCheckBox;
class QSpinBox;
class ButtonWaitDialog;
class DatabaseField;
class QLabel;
class PasswordEdit;
class GenericFieldsEditor;

struct signetdevCmdRespInfo;

class NewAccount : public QDialog
{
	Q_OBJECT
public:
	explicit NewAccount(int id, const QString &name, QWidget *parent = 0);
private:
	DatabaseField *m_username_field;
	DatabaseField *m_url_field;
	DatabaseField *m_groupField;
	DatabaseField *m_email_field;
	QLabel *m_account_name_warning;
	QLineEdit *m_account_name_field;
	ButtonWaitDialog *m_wait_dialog;
	PasswordEdit *m_password_edit;
	account *m_acct;
	int m_id;
	int m_signetdev_cmd_token;
	genericFields m_fields;
	GenericFieldsEditor *m_genericFieldsEditor;
signals:
	void abort();
	void accountCreated(account *acct);
public slots:
	void signetdev_cmd_resp(signetdevCmdRespInfo info);
	void username_editing_finished();
	void add_account_finished(int);
	void account_name_edited(QString str);
	void create_button_pressed();
};

#endif // NEWACCOUNT_H
