#include "keepassimportcontroller.h"
#include "core/Database.h"
#include "core/Entry.h"
#include "core/Group.h"

#include "loggedinwidget.h"
#include "account.h"
#include "signetapplication.h"
#include "buttonwaitdialog.h"
#include "accountrenamedialog.h"

#include <QHBoxLayout>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QMessageBox>

keePassImportController::keePassImportController(LoggedInWidget *loggedInWidget, Database *keePassDatabase, QObject *parent) :
	QObject(parent),
	m_loggedInWidget(loggedInWidget),
	m_keePassDatabase(keePassDatabase),
	m_signetdevCmdToken(-1)
{
	SignetApplication *app = SignetApplication::get();
	connect(app, SIGNAL(signetdevCmdResp(signetdevCmdRespInfo)),
		this, SLOT(signetdevCmdResp(signetdevCmdRespInfo)));
	connect(this, SIGNAL(entryCreated(QString,esdbEntry*)),
		m_loggedInWidget, SLOT(entryCreated(QString,esdbEntry*)));
	connect(this, SIGNAL(entryChanged(int)),
		m_loggedInWidget, SLOT(entryChanged(int)));
}

void keePassImportController::traverse(QString path, Group *g)
{
	const QList<Entry *> &el = g->entries();
	QList<Entry *>::const_iterator eiter;
	for (eiter = el.constBegin(); eiter != el.constEnd(); eiter++) {
		Entry *e = (*eiter);
		account *a = new account(-1);
		a->acctName = e->title();
		a->userName = e->username();
		if (is_email(a->userName)) {
			a->email = a->userName;
		}
		a->url = e->url();
		a->password = e->password();
		if (e->notes().size()) {
			a->fields.addField(genericField("notes", "text block", e->notes()));
		}
		a->fields.addField(genericField("path", QString(), path));
		m_accounts.push_back(a);
	}
	const QList<Group*> &gl = g->children();
	for (auto giter = gl.constBegin(); giter != gl.constEnd(); giter++)
		traverse(path.append('/').append((*giter)->name().replace("/", "//")),  (*giter));
}

void keePassImportController::start()
{
	m_overwriteAll = false;
	m_skipAll = false;
	traverse(QString(), m_keePassDatabase->rootGroup());
	m_accountsIter = m_accounts.begin();
	while (!nextAccount())
		m_accountsIter++;
}

QString keePassImportController::progressString()
{
	return "(" + QString::number(m_accountsIter - m_accounts.begin() + 1) +
			"/" +
		QString::number(m_accounts.size()) + ")";
}

bool keePassImportController::nextAccount()
{
	if (m_accountsIter == m_accounts.end()) {
		done(true);
		return true;
	}
	account *a = *m_accountsIter;
	const esdbEntry *existing = m_loggedInWidget->findEntry("Accounts", a->acctName);

	bool overwrite = false;

	if (existing && m_overwriteAll) {
		overwrite = true;
	} else if (existing && m_skipAll) {
		return false;
	} else if (existing) {
		QMessageBox *resolution = new QMessageBox(QMessageBox::Warning,
							  "KeePass Database Import",
							  "Account \"" + a->acctName +
							  "\" " +
							  progressString() +
							  " already exists",
							  QMessageBox::NoButton,
							  m_loggedInWidget);
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
			AccountRenameDialog *d = new AccountRenameDialog(a->acctName, m_loggedInWidget);
			d->setWindowTitle("KeePass Database Import");
			d->exec();
			if (d->isOkayPressed()) {
				a->acctName = d->newName();
				return nextAccount();
			} else {
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
		a->id = existing->id;
		m_acctNew = false;
	} else {
		a->id = m_loggedInWidget->getUnusedId();
		m_acctNew = true;
		//TODO handle when no ID's available
	}

	m_buttonWaitDialog = new ButtonWaitDialog("KeePass Database Import",
		"import account \"" + a->acctName + "\" " +
			progressString(), m_loggedInWidget);
	m_buttonWaitDialog->show();
	connect(m_buttonWaitDialog, SIGNAL(finished(int)), this, SLOT(importAccountFinished(int)));

	block blk;
	a->toBlock(&blk);
	m_acct = a;
	::signetdev_update_uid(NULL, &m_signetdevCmdToken,
				   a->id,
				   blk.data.size(),
				   (const u8 *)blk.data.data(),
				   (const u8 *)blk.mask.data());
	return true;
}

void keePassImportController::importAccountFinished(int code)
{
	if (m_buttonWaitDialog)
		m_buttonWaitDialog->deleteLater();
	m_buttonWaitDialog = NULL;
	if (code != QMessageBox::Ok) {
		::signetdev_cancel_button_wait();
	}
}

void keePassImportController::signetdevCmdResp(signetdevCmdRespInfo info)
{
	int code = info.resp_code;

	if (m_signetdevCmdToken != info.token) {
		return;
	}
	m_signetdevCmdToken = -1;

	if (m_buttonWaitDialog) {
		m_buttonWaitDialog->done(QMessageBox::Ok);
	}

	m_accountsIter++;

	switch (code) {
	case OKAY: {
		switch (info.cmd) {
		case SIGNETDEV_CMD_UPDATE_UID:
			if (m_acctNew) {
				entryCreated("Accounts", m_acct);
			} else {
				entryChanged(m_acct->id);
			}
			while (!nextAccount()) m_accountsIter++;
			break;
		}
	}
	break;
	case BUTTON_PRESS_TIMEOUT:
	case BUTTON_PRESS_CANCELED:
	case SIGNET_ERROR_DISCONNECT:
	case SIGNET_ERROR_QUIT:
		while (!nextAccount()) m_accountsIter++;
		break;
	default: {
		abort();
	}
	break;
	}
}
