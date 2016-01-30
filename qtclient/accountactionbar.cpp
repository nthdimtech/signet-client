#include "accountactionbar.h"
#include "loggedinwidget.h"
#include "esdb.h"
#include "newaccount.h"
#include "editaccount.h"
#include "buttonwaitdialog.h"

#include <QHBoxLayout>
#include <QPushButton>
#include <QDesktopServices>
#include <QClipboard>

AccountActionBar::AccountActionBar(LoggedInWidget *parent) :
	m_parent(parent), m_buttonWaitDialog(NULL)
{
	QIcon delete_icn = QIcon(":/images/delete.png");
	QHBoxLayout *l = new QHBoxLayout();
	l->setAlignment(Qt::AlignLeft);
	l->setContentsMargins(0,0,0,0);
	m_DeleteButton = new QPushButton(delete_icn, "");
	m_DeleteButton->setToolTip("Delete");
	m_DeleteButton->setAutoDefault(true);
	connect(m_DeleteButton, SIGNAL(pressed()), this, SLOT(deleteAccountUI()));

	QIcon edit_icn = QIcon(":/images/open.png");
	m_openButton = new QPushButton(edit_icn, "");
	m_openButton->setToolTip("Open");
	m_openButton->setAutoDefault(true);
	connect(m_openButton, SIGNAL(pressed()), this, SLOT(openAccountUI()));

	QIcon login_icn = QIcon(":/images/login.png");
	m_loginButton = new QPushButton(login_icn, "");
	m_loginButton->setToolTip("Login");
	m_loginButton->setAutoDefault(true);
	connect(m_loginButton, SIGNAL(pressed()), this, SLOT(typeAccountUserPassUI()));

	QIcon browse_icn = QIcon(":/images/browse.png");
	m_browseUrlButton = new QPushButton(browse_icn, "");
	m_browseUrlButton->setToolTip("Browse");
	m_browseUrlButton->setAutoDefault(true);
	connect(m_browseUrlButton, SIGNAL(pressed()), this, SLOT(browseUrlUI()));

	QIcon password_icn = QIcon(":/images/password.png");
	m_typePasswordButton = new QPushButton(password_icn, "");
	QSize icon_sz;
	password_icn.actualSize(icon_sz);
	QSize icon_display_sz = m_typePasswordButton->iconSize();
	icon_display_sz.setWidth((icon_display_sz.width()*180)/131);
	m_typePasswordButton->setIconSize(icon_display_sz);
	m_typePasswordButton->setToolTip("Type password");
	m_typePasswordButton->setAutoDefault(true);
	connect(m_typePasswordButton, SIGNAL(pressed()), this, SLOT(typeAccountPassUI()));

	QIcon user_icn = QIcon(":/images/user.png");
	m_typeUsernameButton = new QPushButton(user_icn, "");
	m_typeUsernameButton->setToolTip("Type username");
	m_typeUsernameButton->setAutoDefault(true);
	connect(m_typeUsernameButton, SIGNAL(pressed()), this, SLOT(typeAccountUserUI()));

	l->addWidget(m_browseUrlButton);
	l->addWidget(m_loginButton);
	l->addWidget(m_typeUsernameButton);
	l->addWidget(m_typePasswordButton);
	l->addWidget(m_openButton);
	l->addWidget(m_DeleteButton);
	setLayout(l);
}

void AccountActionBar::newAccountFinished(int)
{
	m_newAccountDlg->deleteLater();
	m_newAccountDlg = NULL;
	m_parent->finishTask(false);
}

void AccountActionBar::accountCreated(account *acct)
{
	m_parent->entryCreated(this, acct);
}

void AccountActionBar::newInstanceUI(int id, const QString &name)
{
	m_newAccountDlg = new NewAccount(id, name, this);
	QObject::connect(m_newAccountDlg, SIGNAL(accountCreated(account*)), this, SLOT(accountCreated(account *)));
	QObject::connect(m_newAccountDlg, SIGNAL(finished(int)), this, SLOT(newAccountFinished(int)));
	m_newAccountDlg->show();
}

void AccountActionBar::defaultAction(esdbEntry *entry)
{
	Q_UNUSED(entry);
	typeAccountUserPassUI();
}

void AccountActionBar::selectEntry(esdbEntry *entry)
{
	m_selectedEntry = entry;
	bool enable = entry != NULL;
	bool browse_enable = enable;
	if (m_selectedEntry) {
		QUrl url(entry->getUrl());
		if (!url.isValid() || url.isEmpty()) {
			browse_enable = false;
		}
	}
	m_loginButton->setEnabled(enable);
	m_openButton->setEnabled(enable);
	m_DeleteButton->setEnabled(enable);
	m_browseUrlButton->setEnabled(browse_enable);
	m_typePasswordButton->setEnabled(enable);
	m_typeUsernameButton->setEnabled(enable);
}

void AccountActionBar::browseUrlUI()
{
	esdbEntry *entry = selectedEntry();
	if (entry) {
		QUrl url(entry->getUrl());
		if (!url.scheme().size()) {
			url.setScheme("HTTP");
		}
		QDesktopServices::openUrl(url);
	}
}

void AccountActionBar::accessAccountUI(bool copy_data, bool username, bool password)
{
	account *acct = (account *)selectedEntry();
	if (acct) {
		accessAccount(acct, copy_data, username, password);
	}
}

void AccountActionBar::typeAccountUserUI()
{
	accessAccountUI(false, true, false);
}

void AccountActionBar::typeAccountPassUI()
{
	accessAccountUI(false, false, true);
}

void AccountActionBar::typeAccountUserPassUI()
{
	accessAccountUI(false, true, true);
}

void AccountActionBar::openAccountUI()
{
	account *acct = (account *)selectedEntry();
	if (acct) {
		openAccount(acct);
	}
}

void AccountActionBar::deleteAccountUI()
{
	account *acct = (account *)selectedEntry();
	if (acct) {
		m_parent->selectEntry(NULL);
		int id = acct->id;
		m_buttonWaitDialog = new ButtonWaitDialog("Delete account",
		        QString("delete account \"") + acct->acctName + QString("\""),
		        m_parent);
		connect(m_buttonWaitDialog, SIGNAL(finished(int)), this, SLOT(deleteAccountFinished(int)));
		m_buttonWaitDialog->show();
		m_parent->beginIDTask(id, LoggedInWidget::ID_TASK_DELETE, NONE);
	}
}

void AccountActionBar::deleteAccountFinished(int code)
{
	if (code != QMessageBox::Ok) {
		::signetdev_cancel_button_wait();
	}
	m_buttonWaitDialog->deleteLater();
	m_buttonWaitDialog = NULL;
	m_parent->finishTask();
}

void AccountActionBar::openAccount(account *acct)
{
	int id = acct->id;
	m_buttonWaitDialog = new ButtonWaitDialog(
	    "Open account",
	    "Open account \"" + acct->acctName + "\"",
	    m_parent);
	connect(m_buttonWaitDialog, SIGNAL(finished(int)), this, SLOT(openAccountFinished(int)));
	m_buttonWaitDialog->show();
	m_parent->beginIDTask(id, LoggedInWidget::ID_TASK_READ, OPEN_ACCOUNT);
}

void AccountActionBar::openAccountFinished(int code)
{
	m_buttonWaitDialog->deleteLater();
	m_buttonWaitDialog = NULL;
	if (code != QMessageBox::Ok) {
		::signetdev_cancel_button_wait();
	}
	m_parent->finishTask(true);
}

void AccountActionBar::accessAccount(account *acct, bool copy_data, bool username, bool password)
{
	int id = acct->id;
	m_accessUsername = username;
	m_accessPassword = password;
	QString message;
	QString title = acct->acctName;
	if (username && password) {
		if (!copy_data) {
			message.append("to login to ");
			message.append(acct->acctName);
		} else {
			message = QString("copy username and password");
		}
	} else {
		if (copy_data) {
			if (username) {
				message.append("copy username ");
			} else if (password) {
				message.append("copy password ");
			}
		} else {
			if (username) {
				message.append("type username ");
			} else if (password) {
				message.append("type password ");
			}
		}
		if (username) {
			message.append("for ");
		} else if (password) {
			message.append("for ");
		}
		message.append(acct->acctName);
	}
	m_buttonWaitDialog = new ButtonWaitDialog(title, message, m_parent);
	connect(m_buttonWaitDialog, SIGNAL(finished(int)), this, SLOT(accessAccountFinished(int)));
	m_buttonWaitDialog->show();
	if (copy_data) {
		m_parent->beginIDTask(id, LoggedInWidget::ID_TASK_READ, COPY_DATA);
	} else {
		m_parent->beginIDTask(id, LoggedInWidget::ID_TASK_READ, TYPE_DATA);
	}
}

void AccountActionBar::accessAccountFinished(int code)
{
	m_buttonWaitDialog->deleteLater();
	m_buttonWaitDialog = NULL;
	if (code != QMessageBox::Ok) {
		::signetdev_cancel_button_wait();
	}
	m_parent->finishTask(m_accessUsername && m_accessPassword);
}

void AccountActionBar::idTaskComplete(int id, int intent)
{
	Q_UNUSED(id);
	Q_UNUSED(intent);
	if (m_buttonWaitDialog)
		m_buttonWaitDialog->done(QMessageBox::Ok);
}

void AccountActionBar::getEntryDone(esdbEntry *entry, int intent)
{
	switch (intent) {
	case TYPE_DATA: {
		account *acct = static_cast<account *>(entry);
		QApplication *app = static_cast<QApplication *>(QApplication::instance());
		if (app->focusWidget()) {
			m_buttonWaitDialog->stopTimer();
			QMessageBox *box = SignetApplication::messageBoxError(
			                       QMessageBox::Warning,
			                       "Signet",
			                       "A destination text area must be selected for typing to start\n\n"
			                       "Close this window and try again.", m_buttonWaitDialog);
			connect(box, SIGNAL(finished(int)), this, SLOT(retryTypeData()));
			m_id = entry->id;
			break;
		}
		if (m_buttonWaitDialog) {
			m_buttonWaitDialog->done(QMessageBox::Ok);
		}
		QByteArray keys;
		if (m_accessUsername) {
			keys.append(acct->userName.toLatin1());
		}
		if (m_accessPassword) {
			if (m_accessUsername) {
				keys.append("\t");
			}
			keys.append(acct->password.toLatin1());
			if (m_accessUsername) {
				keys.append("\n");

			}
		}
		::signetdev_type_async(NULL, &m_signetdevCmdToken,
		                       (u8 *)keys.data(), keys.length());
	}
	break;
	case COPY_DATA: {
		account *acct = static_cast<account *>(entry);
		if (m_buttonWaitDialog) {
			m_buttonWaitDialog->done(QMessageBox::Ok);
		}
		QByteArray keys;
		if (m_accessUsername) {
			keys.append(acct->userName.toLatin1());
		}
		if (m_accessPassword) {
			if (m_accessUsername) {
				keys.append(":");
			}
			keys.append(acct->password.toLatin1());
		}
		QClipboard *clipboard = QApplication::clipboard();
		clipboard->setText(QString(keys));
	}
	break;
	case OPEN_ACCOUNT: {
		account *acct = static_cast<account *>(entry);
		if (m_buttonWaitDialog) {
			m_buttonWaitDialog->done(QMessageBox::Ok);
		}
		EditAccount *ea = new EditAccount(acct, m_parent);
		connect(ea, SIGNAL(abort()), this, SLOT(abort_proxy()));
		connect(ea, SIGNAL(accountChanged(int)), m_parent, SLOT(entryChanged(int)));
		connect(ea, SIGNAL(finished(int)), ea, SLOT(deleteLater()));
		ea->show();
	}
	break;
	default:
		if (m_buttonWaitDialog) {
			m_buttonWaitDialog->done(QMessageBox::Ok);
		}
		break;
	}
}

void AccountActionBar::retryTypeData()
{
	m_parent->beginIDTask(m_id, LoggedInWidget::ID_TASK_READ, TYPE_DATA);
}


