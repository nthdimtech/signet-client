#include "databaseimportcontroller.h"
#include "databaseimporter.h"
#include "esdb.h"
#include "signetapplication.h"
#include "account.h"
#include "entryrenamedialog.h"
#include "loggedinwidget.h"
#include "buttonwaitwidget.h"
#include "generictypedesc.h"
#include "generic.h"
#include "generictext.h"

#include <QPushButton>
#include <QDialog>
#include <QProgressBar>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QStackedWidget>

DatabaseImportController::DatabaseImportController(DatabaseImporter *importer, LoggedInWidget *parent, bool useUpdateUids) :
	QObject(parent),
	m_loggedInWidget(parent),
	m_importer(importer),
	m_overwriteAll(false),
	m_skipAll(false),
	m_updatePending(false),
	m_useUpdateUids(useUpdateUids),
	m_importProgressDialog(nullptr),
	m_importProgressStack(nullptr),
	m_conflictResponse(CONFLICT_RESPONSE_NONE),
	m_importCancel(false),
	m_importState(IMPORT_STATE_NO_SOURCE),
	m_typeIdMapBuilt(false)
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

void DatabaseImportController::conflictResponse()
{
	m_importProgressStack->setCurrentIndex(0);
	while (!nextEntry()) advanceDbTypeIter();
}

void DatabaseImportController::advanceDbTypeIter()
{
	m_conflictResponse = CONFLICT_RESPONSE_NONE;
	switch (m_importState) {
	case IMPORT_STATE_CONFLICT_RESOLUTION: {
		DatabaseImporter::databaseType *t = m_dbIter.value();
		m_dbTypeIter++;
		if (m_dbTypeIter == t->end()) {
			m_dbIter++;
			if (m_dbIter == m_db->end()) {
				m_dbIter = m_db->begin();
			}
			if (m_dbIter == m_dbIterStart) {
				m_dbIter = m_db->end();
			} else {
				t = m_dbIter.value();
				m_dbTypeIter = t->begin();
			}
		}
		break;
	}
	case IMPORT_STATE_WRITING: {
		m_importIndex++;
	} break;
	default:
		break;
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

void DatabaseImportController::importFinished(int code)
{
	Q_UNUSED(code);
	switch(m_importState) {
	case IMPORT_STATE_CONFLICT_RESOLUTION:
		m_importState = IMPORT_STATE_CONFLICT_RESOLUTION_CANCEL;
		done(true);
		break;
	case IMPORT_STATE_WRITING:
		m_importState = IMPORT_STATE_WRITE_CANCEL;
		done(true);
		break;
	default:
		break;
	}
	m_importProgressDialog->deleteLater();
	m_importProgressDialog = nullptr;
}

void DatabaseImportController::importCancel()
{
	m_importCancel = true;
}

bool DatabaseImportController::nextEntry()
{
	if (m_importState == IMPORT_STATE_WRITING) {
		m_importProgressStack->setCurrentIndex(0);
		if (m_importIndex == m_importEntries.size() || m_importCancel) {
			if (m_importCancel) {
				m_importState = IMPORT_STATE_WRITE_CANCEL;
			} else {
				m_importState = IMPORT_STATE_WRITE_COMPLETE;
				m_importProgressStack->setCurrentIndex(3);
				return true;
			}
			done(true);
			m_importProgressDialog->done(QDialog::Accepted);
			return true;
		}
		m_entry = m_importEntries[m_importIndex];
		m_typeName = m_importTypenames[m_importIndex];
		QString fullTitle = m_entry->getFullTitle();
		if (m_useUpdateUids) {
			if (m_importIndex == 0) {
				m_importProgressStack->setCurrentIndex(2);
			}
		} else {
			m_importProgressStack->setCurrentIndex(2);
		}

		m_importProgressBar->setMinimum(0);
		m_importProgressBar->setMaximum(m_importEntries.size());
		m_importProgressLabel->setText("Imported " + QString::number(m_importIndex) + " of " + QString::number(m_importEntries.size()) + " entries");
		m_importProgressBar->setValue(m_importIndex);

		block blk;
		m_entry->toBlock(&blk);
		if (m_useUpdateUids) {
			::signetdev_update_uids(nullptr, &m_signetdevCmdToken,
						m_entry->id,
						blk.data.size(),
						(const u8 *)blk.data.data(),
						(const u8 *)blk.mask.data(), m_importEntries.size() - m_importIndex - 1);
		} else {
			::signetdev_update_uid(nullptr, &m_signetdevCmdToken,
					       m_entry->id,
					       blk.data.size(),
					       (const u8 *)blk.data.data(),
					       (const u8 *)blk.mask.data());
		}
		m_updatePending = true;
		return true;
	} else if (m_importState == IMPORT_STATE_CONFLICT_RESOLUTION) {
		if (iteratorsAtEnd()) {
			m_importState = IMPORT_STATE_WRITING;
			m_importIndex = -1; //Will be incremented to 0 by advance function
			return false;
		}
		QString typeName = m_dbIter.key();
		if (typeName != "Data types" && m_typeIdMapBuilt == false) {
				m_typeIdMapBuilt = true;
				auto iter = m_db->find("Data types");
				if (iter != m_db->end()) {
					for (auto e : *iter.value()) {
						auto gt = static_cast<genericTypeDesc *>(e);
						m_typeIdMap.insert(e->getTitle(), gt->typeId);
					}
				}
				const std::vector<esdbGenericModule *> &gm = m_loggedInWidget->getGenericModules();
				for (auto m : gm) {
					if (m->name() == typeName) {
						m_typeIdMap.insert(m->name(), m->typeId());
						break;
					}
				}
		}
		m_importProgressStack->setCurrentIndex(1);
		esdbEntry *importEntry = *m_dbTypeIter;
		const esdbEntry *existingEntry = m_loggedInWidget->findEntry(typeName, importEntry->getFullTitle());

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
			if (m_conflictResponse == CONFLICT_RESPONSE_NONE) {
				QString conflictText =
						"Import \"" +
						m_dbIter.key() + "\" " + "entry \"" + fullTitle +
						"\" " +
						progressString() +
						" already exists";
				m_importConflictLabel->setText(conflictText);
				m_importProgressStack->setCurrentIndex(1);
				return true;
			} else {
				if (m_conflictResponse == CONFLICT_RESPONSE_OVERWRITE_ALL) {
					m_overwriteAll = true;
				}
				if (m_conflictResponse == CONFLICT_RESPONSE_CANCEL) {
					m_importState = IMPORT_STATE_CONFLICT_RESOLUTION_CANCEL;
					done(true);
					m_importProgressDialog->done(QDialog::Accepted);
					return true;
				}

				if (m_conflictResponse == CONFLICT_RESPONSE_OVERWRITE || m_conflictResponse == CONFLICT_RESPONSE_OVERWRITE_ALL) {
					overwrite = true;
				} else if (m_conflictResponse == CONFLICT_RESPONSE_RENAME) {
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
				} else {
					return false;
				}
			}
		}
		if (overwrite) {
			importEntry->id = existingEntry->id;
		} else {
			importEntry->id = m_loggedInWidget->getUnusedId(m_reservedIds);
			m_reservedIds.insert(importEntry->id);
			//TODO handle when no ID's available
		}

		QString fullTitle = importEntry->getFullTitle();
		if (fullTitle.at(0) == '/') {
			fullTitle.remove(0, 1);
		}

		if (importEntry->type == ESDB_TYPE_GENERIC) {
			generic *g = static_cast<generic *>(importEntry);
			if (g->typeId == generic::invalidTypeId) {
				auto iter = m_typeIdMap.find(typeName);
				if (iter == m_typeIdMap.end()) {
					int newId = m_loggedInWidget->getUnusedId(m_reservedIds);
					m_reservedIds.insert(newId);
					genericTypeDesc *typeDesc = new genericTypeDesc(newId);
					typeDesc->typeId = m_loggedInWidget->getUnusedTypeId(m_reservedTypeIds);
					m_reservedTypeIds.insert(typeDesc->typeId);
					typeDesc->name = typeName;

					//TODO: Ideally we shouldn't base the type description on a single entry
					QVector<genericField> fields;
					g->typeId = typeDesc->typeId;
					g->getFields(fields);
					for (auto f : fields) {
						fieldSpec fs;
						fs.name = f.name;
						fs.type = f.type;
						typeDesc->fields.push_back(fs);
					}
					m_importEntries.push_back(typeDesc);
					m_importTypenames.push_back("Data types");
					m_importOverwrite.push_back(false);
					m_typeIdMap.insert(typeName, typeDesc->typeId);
				} else {
					g->typeId = iter.value();
				}
			}
		} else if (importEntry->type == ESDB_TYPE_GENERIC_TYPE_DESC) {
			genericTypeDesc *gt = static_cast<genericTypeDesc *>(importEntry);
			if (overwrite) {
				const genericTypeDesc *gtExisting = static_cast<const genericTypeDesc *>(existingEntry);
				gt->typeId = gtExisting->typeId;
			} else {
				gt->typeId = m_loggedInWidget->getUnusedTypeId(m_reservedTypeIds);
				m_reservedTypeIds.insert(gt->typeId);
			}
		}
		m_importEntries.push_back(importEntry);
		m_importTypenames.push_back(m_dbIter.key());
		m_importOverwrite.push_back(overwrite);
		return false;
	} else {
		return false;
	}
}

void DatabaseImportController::buttonCanceled()
{
	m_importProgressStack->setCurrentIndex(0);
	::signetdev_cancel_button_wait();
	m_importState = IMPORT_STATE_WRITE_CANCEL;
	m_importProgressDialog->done(QDialog::Rejected);
}

void DatabaseImportController::buttonTimeout()
{
	m_importProgressStack->setCurrentIndex(0);
	m_importState = IMPORT_STATE_WRITE_CANCEL;
	m_importProgressDialog->done(QDialog::Rejected);
}

void DatabaseImportController::importDone(bool success)
{
	if (success) {
		m_importState = IMPORT_STATE_READING;
		m_db = m_importer->getDatabase();
		m_dbIter = m_db->begin();
		m_dbIterStart = m_db->begin();
		for (;m_dbIter != m_db->end(); m_dbIter++) {
			if (m_dbIter.key() == "Data types") {
				m_dbIterStart = m_dbIter;
				break;
			}
		}
		if (m_dbIter == m_db->end()) {
			m_dbIter = m_db->begin();
		}

		m_importProgressDialog = new QDialog(m_loggedInWidget);
		m_importProgressDialog->setWindowTitle(m_importer->databaseTypeName() + " import");
		m_importProgressDialog->setWindowModality(Qt::WindowModal);
		m_importProgressDialog->setLayout(new QVBoxLayout());
		m_importProgressStack = new QStackedWidget();
		m_importProgressDialog->layout()->addWidget(m_importProgressStack);
		m_importProgressBar = new QProgressBar();
		m_importConflictLabel = new genericText("");

		QPushButton *progressWidgetCancel = new QPushButton("Cancel");
		connect(progressWidgetCancel, SIGNAL(pressed()), this, SLOT(importCancel()));
		connect(m_importProgressDialog, SIGNAL(finished(int)), this, SLOT(importFinished(int)));

		QWidget *progressWidget = new QWidget();
		QVBoxLayout *vbox = new QVBoxLayout();
		vbox->setAlignment(Qt::AlignTop);
		m_importProgressLabel = new genericText("Importing entries...");
		m_importProgressLabel->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
		vbox->addWidget(m_importProgressLabel);
		vbox->addWidget(m_importProgressBar);
		vbox->addWidget(progressWidgetCancel);
		progressWidget->setLayout(vbox);
		m_importProgressStack->addWidget(progressWidget);

		QWidget *conflictWidget = new QWidget();
		vbox = new QVBoxLayout();
		vbox->setAlignment(Qt::AlignTop);
		conflictWidget->setLayout(vbox);
		m_importConflictLabel->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);
		vbox->addWidget(m_importConflictLabel);
		QHBoxLayout *hbox = new QHBoxLayout();
		hbox->setAlignment(Qt::AlignLeft);
		QPushButton *cancelButton = new QPushButton("Cancel");
		QPushButton *overwriteAllButton = new QPushButton("Overwrite All");
		QPushButton *overwriteButton = new QPushButton("Overwrite");
		QPushButton *skipAllButton = new QPushButton("Skip All");
		QPushButton *skipButton = new QPushButton("Skip");
		QPushButton *renameButton = new QPushButton("Rename");

		cancelButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
		hbox->addWidget(cancelButton);
		connect(cancelButton, SIGNAL(pressed()), this, SLOT(cancelConflictResponse()));

		overwriteAllButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
		hbox->addWidget(overwriteAllButton);
		connect(overwriteAllButton, SIGNAL(pressed()), this, SLOT(overwriteAllConflictResponse()));

		overwriteButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
		hbox->addWidget(overwriteButton);
		connect(overwriteButton, SIGNAL(pressed()), this, SLOT(overwriteConflictResponse()));

		skipAllButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
		hbox->addWidget(skipAllButton);
		connect(skipAllButton, SIGNAL(pressed()), this, SLOT(skipAllConflictResponse()));

		skipButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
		hbox->addWidget(skipButton);
		connect(skipButton, SIGNAL(pressed()), this, SLOT(skipConflictResponse()));

		renameButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
		hbox->addWidget(renameButton);
		connect(renameButton, SIGNAL(pressed()), this, SLOT(renameConflictResponse()));

		vbox->addLayout(hbox, Qt::AlignBottom);
		m_importProgressStack->addWidget(conflictWidget);

		m_buttonWaitWidget = new ButtonWaitWidget("begin importing " + m_importer->databaseTypeName(), true);
		m_importProgressStack->addWidget(m_buttonWaitWidget);
		connect(m_buttonWaitWidget, SIGNAL(timeout()), this, SLOT(buttonTimeout()));
		connect(m_buttonWaitWidget, SIGNAL(canceled()), this, SLOT(buttonCanceled()));

		QWidget *importCompleteWidget = new QWidget();
		vbox = new QVBoxLayout();
		vbox->setAlignment(Qt::AlignTop);
		vbox->addWidget(new genericText(m_importer->databaseTypeName() + " import complete"));
		QPushButton *ok = new QPushButton("Ok");
		connect(ok, SIGNAL(pressed()), m_importProgressDialog, SLOT(accept()));
		vbox->addWidget(ok);
		importCompleteWidget->setLayout(vbox);
		m_importProgressStack->addWidget(importCompleteWidget);

		m_importProgressStack->setCurrentIndex(0);

		m_importProgressDialog->show();

		DatabaseImporter::databaseType *t = m_dbIter.value();
		m_dbTypeIter = t->begin();
		m_importState = IMPORT_STATE_CONFLICT_RESOLUTION;
		m_overwriteAll = false;
		m_skipAll = false;
		while (!nextEntry()) advanceDbTypeIter();
	} else {
		delete m_importer;
		m_importer = NULL;
		m_importState = IMPORT_STATE_NO_SOURCE;
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

	m_importProgressStack->setCurrentIndex(0);

	bool overwrite = m_importOverwrite[m_importIndex];

	advanceDbTypeIter();

	switch (code) {
	case OKAY: {
		switch (info.cmd) {
		case SIGNETDEV_CMD_UPDATE_UIDS:
		case SIGNETDEV_CMD_UPDATE_UID:
			if (!overwrite) {
				entryCreated(m_typeName, m_entry);
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
