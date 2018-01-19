#ifndef KEEPASSIMPORTWIDGET_H
#define KEEPASSIMPORTWIDGET_H

#include <QObject>
#include <QWidget>
#include <QVector>

#include "signetapplication.h"

class Database;
class Group;
struct account;
class LoggedInWidget;
class ButtonWaitDialog;
struct esdbEntry;

class keePassImportController : public QObject
{
	Q_OBJECT
	LoggedInWidget *m_loggedInWidget;
	Database *m_keePassDatabase;
	void traverse(QString path, Group *group);
	QVector<account *> m_accounts;
	QVector<account *>::iterator m_accountsIter;
	int m_signetdevCmdToken;
	bool nextAccount();
	bool m_overwriteAll;
	bool m_skipAll;
	ButtonWaitDialog *m_buttonWaitDialog;
	account *m_acct;
	bool m_acctNew;
	QString progressString();
public:
	explicit keePassImportController(LoggedInWidget *loggedInWidget, Database *keePassDatabase, QObject *parent = 0);
signals:
	void done(bool);
	void entryCreated(QString typeName, esdbEntry *entry);
	void entryChanged(int id);
	void abort();
public slots:
	void signetdevCmdResp(signetdevCmdRespInfo info);
	void start();
	void importAccountFinished(int code);
};

#endif // KEEPASSIMPORTWIDGET_H
