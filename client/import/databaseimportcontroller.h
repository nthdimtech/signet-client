#ifndef DATABASEIMPORTCONTROLLER_H
#define DATABASEIMPORTCONTROLLER_H

#include <QObject>

#include "databaseimporter.h"
#include "signetapplication.h"

class LoggedInWidget;
class ButtonWaitDialog;

class DatabaseImportController : public QObject
{
	Q_OBJECT
	LoggedInWidget *m_loggedInWidget;
	DatabaseImporter *m_importer;
	DatabaseImporter::database *m_db;
	DatabaseImporter::databaseIter m_dbIter;
	DatabaseImporter::databaseTypeIter m_dbTypeIter;
	ButtonWaitDialog *m_buttonWaitDialog;
	esdbEntry *m_entry;
	bool m_overwriteAll;
	bool m_skipAll;
	bool m_entryNew;
	void advanceDbTypeIter();
	bool nextEntry();
	QString progressString();
	int m_signetdevCmdToken;
	bool m_updatePending;
	bool iteratorsAtEnd();
public:
	explicit DatabaseImportController(DatabaseImporter *importer, LoggedInWidget *parent);
	DatabaseImporter *importer() {
		return m_importer;
	}
signals:
	void entryCreated(QString,esdbEntry*);
	void done(bool success);
	void entryChanged(int);
	void abort();

public slots:
	void importDone(bool success);
	void start();
	void signetdevCmdResp(signetdevCmdRespInfo info);
};

#endif // DATABASEIMPORTCONTROLLER_H
