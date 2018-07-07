#include "databaseimportcontroller.h"
#include "databaseimporter.h"
#include "esdb.h"
#include "signetapplication.h"
#include "account.h"
#include "entryrenamedialog.h"
#include "loggedinwidget.h"
#include "buttonwaitdialog.h"

#include <QPushButton>

DatabaseImportController::DatabaseImportController(DatabaseImporter *importer, LoggedInWidget *parent) :
	QObject(parent),
	m_loggedInWidget(parent),
	m_importer(importer),
	m_overwriteAll(false),
	m_skipAll(false),
	m_updatePending(false)
{
	importer->setParent(this);
	connect(m_importer, SIGNAL(done(bool)), this, SLOT(importDone(bool)));
	SignetApplication *app = SignetApplication::get();
	connect(app, SIGNAL(signetdevCmdResp(signetdevCmdRespInfo)),
		this, SLOT(signetdevCmdResp(signetdevCmdRespInfo)));
	connect(this, SIGNAL(entryCreated(QString,esdbEntry*)),
		parent, SLOT(entryCreated(QString,esdbEntry*)));
	connect(this, SIGNAL(entryChanged(int)),
		parent, SLOT(entryChanged(int)));
}

void DatabaseImportController::advanceDbTypeIter()
{
	DatabaseImporter::databaseType *t = m_dbIter.value();
	m_dbTypeIter++;
	if (m_dbTypeIter == t->end()) {
		m_dbIter++;
		if (m_dbIter != m_db->end()) {
			t = m_dbIter.value();
			m_dbTypeIter = t->begin();
		}
	}
}

QString DatabaseImportController::progressString()
{
	return "(" + QString::number(m_dbTypeIter - m_dbIter.value()->begin() + 1) +
			"/" +
		QString::number(m_dbIter.value()->size()) + ")";
}

bool DatabaseImportController::iteratorsAtEnd()
{
	if (m_dbIter == m_db->end()) {
		return true;
	}
	DatabaseImporter::databaseType *t = m_dbIter.value();
	if (m_dbTypeIter == t->end()) {
		return true;
	}
	return false;
}

bool DatabaseImportController::nextEntry()
{
	if (iteratorsAtEnd()) {
		done(true);
		return true;
	}
	esdbEntry *importEntry = *m_dbTypeIter;
	const esdbEntry *existingEntry = m_loggedInWidget->findEntry(m_dbIter.key(), importEntry->getFullTitle());

	bool overwrite = false;

	if (existingEntry && m_overwriteAll) {
		overwrite = true;
	} else if (existingEntry && m_skipAll) {
		return false;
	} else if (existingEntry) {
		QString fullTitle = importEntry->getFullTitle();
		if (fullTitle.at(0) == '/') {
			fullTitle.remove(0, 1);
		}
		QMessageBox *resolution = new QMessageBox(QMessageBox::Warning,
							  m_importer->databaseTypeName() + " Import",
							  "Entry \"" + fullTitle +
							  "\" " +
							  progressString() +
							  " already exists",
							  QMessageBox::NoButton,
							  (QWidget *)parent());
		QPushButton *cancelButton = resolution->addButton("Cancel", QMessageBox::AcceptRole);
		QPushButton *ovewriteAllButton = resolution->addButton("Overwrite All", QMessageBox::AcceptRole);
		QPushButton *ovewriteButton = resolution->addButton("Overwrite", QMessageBox::AcceptRole);
		QPushButton *skipAllButton = resolution->addButton("Skip All", QMessageBox::AcceptRole);
		resolution->addButton("Skip", QMessageBox::AcceptRole);
		QPushButton *renameButton = resolution->addButton("Rename", QMessageBox::AcceptRole);

		resolution->setWindowModality(Qt::WindowModal);
		resolution->exec();
		auto clickedButton = resolution->clickedButton();

		if (clickedButton == ovewriteAllButton) {
			m_overwriteAll = true;
		}
		if (clickedButton == skipAllButton) {
			m_skipAll = true;
			return false;
		}

		if (clickedButton == ovewriteButton || clickedButton == ovewriteAllButton) {
			overwrite = true;
		} else if (clickedButton == renameButton){
			EntryRenameDialog *d = new EntryRenameDialog(importEntry->getTitle(), m_loggedInWidget);
			d->setWindowTitle(m_importer->databaseTypeName() + " Import");
			d->exec();
			if (d->isOkayPressed()) {
				importEntry->setTitle(d->newName());
				d->deleteLater();
				return nextEntry();
			} else {
				d->deleteLater();
				return false;
			}
		} else if (clickedButton == cancelButton) {
			done(false);
			return true;
		} else {
			return false;
		}
	}
	if (overwrite) {
		importEntry->id = existingEntry->id;
		m_entryNew = false;
	} else {
		importEntry->id = m_loggedInWidget->getUnusedId();
		m_entryNew = true;
		//TODO handle when no ID's available
	}

	QString fullTitle = importEntry->getFullTitle();
	if (fullTitle.at(0) == '/') {
		fullTitle.remove(0, 1);
	}
	m_buttonWaitDialog = new ButtonWaitDialog(m_importer->databaseTypeName() + " Import",
		"import \"" + fullTitle + "\" " +
		progressString(), (QWidget *)parent());
	m_buttonWaitDialog->show();
	connect(m_buttonWaitDialog, SIGNAL(finished(int)), this, SLOT(importAccountFinished(int)));

	block blk;
	m_entry = importEntry;
	m_entry->toBlock(&blk);
	::signetdev_update_uid(NULL, &m_signetdevCmdToken,
				   m_entry->id,
				   blk.data.size(),
				   (const u8 *)blk.data.data(),
				   (const u8 *)blk.mask.data());
	m_updatePending = true;
	return true;
}

void DatabaseImportController::importAccountFinished(int code)
{
	if (code != QMessageBox::Ok) {
		::signetdev_cancel_button_wait();
	}
	m_buttonWaitDialog->deleteLater();
	m_buttonWaitDialog = NULL;
}

void DatabaseImportController::importDone(bool success)
{
	if (success) {
		m_db = m_importer->getDatabase();
		m_dbIter = m_db->begin();
		DatabaseImporter::databaseType *t = m_dbIter.value();
		m_dbTypeIter = t->begin();

		m_overwriteAll = false;
		m_skipAll = false;
		while (!nextEntry())
			advanceDbTypeIter();
	} else {
		delete m_importer;
		m_importer = NULL;
		done(false);
	}
}

void DatabaseImportController::start()
{
	m_importer->start();
}

void DatabaseImportController::signetdevCmdResp(signetdevCmdRespInfo info)
{
	int code = info.resp_code;

	if (m_signetdevCmdToken != info.token) {
		return;
	}
	m_signetdevCmdToken = -1;
	m_updatePending = false;

	if (m_buttonWaitDialog) {
		m_buttonWaitDialog->done(QMessageBox::Ok);
	}

	QString typeName = m_dbIter.key();

	advanceDbTypeIter();

	switch (code) {
	case OKAY: {
		switch (info.cmd) {
		case SIGNETDEV_CMD_UPDATE_UID:
			if (m_entryNew) {
				entryCreated(typeName, m_entry);
			} else {
				entryChanged(m_entry->id);
			}
			while (!nextEntry()) advanceDbTypeIter();
			break;
		}
	}
	break;
	case BUTTON_PRESS_TIMEOUT:
	case BUTTON_PRESS_CANCELED:
	case SIGNET_ERROR_DISCONNECT:
	case SIGNET_ERROR_QUIT:
		while (!nextEntry()) advanceDbTypeIter();
		break;
	default: {
		abort();
	}
	break;
	}
}
