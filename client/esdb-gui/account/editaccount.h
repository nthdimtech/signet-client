#ifndef EDITACCOUNT_H
#define EDITACCOUNT_H

#include <QDialog>
#include "signetapplication.h"
#include "editentrydialog.h"

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

class EditAccount : public EditEntryDialog
{
	Q_OBJECT
	account *m_acct;
	QLineEdit *m_accountNameEdit;
	QLabel *m_accountNameWarning;
	DatabaseField *m_usernameField;
	DatabaseField *m_groupField;
	PasswordEdit *m_passwordEdit;
	DatabaseField *m_urlField;
	DatabaseField *m_emailField;
	QPushButton *m_browseUrlButton;
	GenericFieldsEditor *m_genericFieldsEditor;
	void setAccountValues();
	bool toBlock(block &blk);
	void setup(QString name);
	virtual QString entryName();
	virtual void applyChanges(esdbEntry *);
	virtual esdbEntry *createEntry(int id);
	virtual void undoChanges();
    QStringList m_groupList;
public:
    EditAccount(int id, QString entryName, QStringList groupList, QWidget *parent = 0);
    EditAccount(account *generic, QStringList groupList, QWidget *parent = 0);
	virtual ~EditAccount();
public slots:
	void browseUrl();
    void accountNameEdited();
private slots:
    void usernameEditingFinished();
};

#endif // EDITACCOUNT_H
