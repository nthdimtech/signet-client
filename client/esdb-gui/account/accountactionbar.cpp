#include "accountactionbar.h"
#include "loggedinwidget.h"
#include "esdb.h"
#include "editaccount.h"
#include "buttonwaitwidget.h"

#include <QHBoxLayout>
#include <QPushButton>
#include <QDesktopServices>
#include <QClipboard>
#include <QMenu>
#include <QApplication>

AccountActionBar::AccountActionBar(LoggedInWidget *parent, bool writeEnabled, bool typeEnabled) :
	EsdbActionBar(parent, "Account", writeEnabled, typeEnabled),
    m_loggedInWidget(parent),
    m_quickTypeState(QUICKTYPE_STATE_INITIAL)
{
	m_browseUrlButton = addBrowseButton();
	connect(m_browseUrlButton, SIGNAL(pressed()), this, SLOT(browseUrlUI()));

	m_loginButton = addButton(m_typeEnabled ? "Login" : "Copy username and password",
				  ":/images/login.png");
	connect(m_loginButton, SIGNAL(pressed()), this, SLOT(typeAccountUserPassUI()));

	QIcon user_icn = QIcon(":/images/user.png");
	m_typeUsernameButton = new QPushButton(user_icn, "");
	if (m_typeEnabled)
		m_typeUsernameButton->setToolTip("Type username");
	else
		m_typeUsernameButton->setToolTip("Copy username");
	addButton(m_typeUsernameButton);
	connect(m_typeUsernameButton, SIGNAL(pressed()), this, SLOT(typeAccountUserUI()));

	QIcon password_icn = QIcon(":/images/password.png");
	m_typePasswordButton = new QPushButton(password_icn, "");
	QSize icon_sz;
	password_icn.actualSize(icon_sz);
	QSize icon_display_sz = m_typePasswordButton->iconSize();
	icon_display_sz.setWidth((icon_display_sz.width()*180)/131);
	m_typePasswordButton->setIconSize(icon_display_sz);
	if (m_typeEnabled)
		m_typePasswordButton->setToolTip("Type password");
	else
		m_typePasswordButton->setToolTip("Copy password");
	addButton(m_typePasswordButton);
	connect(m_typePasswordButton, SIGNAL(pressed()), this, SLOT(typeAccountPassUI()));

	m_openButton = addOpenButton();
	connect(m_openButton, SIGNAL(pressed()), this, SLOT(openAccountUI()));

	m_DeleteButton = addDeleteButton();
	connect(m_DeleteButton, SIGNAL(pressed()), this, SLOT(deleteAccountUI()));
}

void AccountActionBar::newAccountFinished(int)
{
	m_newAccountDlg->deleteLater();
	m_newAccountDlg = nullptr;
	m_parent->finishTask();
}

void AccountActionBar::newInstanceUI(int id, const QString &name)
{
	QStringList groupList;
	m_loggedInWidget->getCurrentGroups("Accounts", groupList);
	m_newAccountDlg = new EditAccount(id, name, groupList, this);
	QObject::connect(m_newAccountDlg, SIGNAL(entryCreated(esdbEntry *)), this, SLOT(entryCreated(esdbEntry *)));
	QObject::connect(m_newAccountDlg, SIGNAL(finished(int)), this, SLOT(newAccountFinished(int)));
	m_newAccountDlg->show();
}

void AccountActionBar::defaultAction(esdbEntry *entry)
{
	Q_UNUSED(entry);
	QMenu *menu;
	menu = new QMenu(this);
	QAction *browseAction = menu->addAction("&Browse");
	menu->addSeparator();
	QAction *loginAction = nullptr;
	QAction *copyUsernamePasswordAction = nullptr;
	if (m_typeEnabled) {
		loginAction = menu->addAction("&Login");
	} else {
		copyUsernamePasswordAction = menu->addAction("Copy username and password");
	}
	QAction *usernameAction = nullptr;
	QAction *passwordAction = nullptr;
	if (m_typeEnabled) {
		usernameAction = menu->addAction("&Username");
		passwordAction = menu->addAction("&Password");
	}
	QAction *copyUsernameAction = menu->addAction("Copy user&name");
	QAction *copyPasswordAction = menu->addAction("&Copy password");
	menu->addSeparator();
	QAction *openAction = menu->addAction("&Open");
	QAction *deleteAction = menu->addAction("Delete");

	connect(browseAction, SIGNAL(triggered(bool)), this, SLOT(browseUrlUI()));
	if (m_typeEnabled) {
		connect(loginAction, SIGNAL(triggered(bool)), this, SLOT(typeAccountUserPassUI()));
		connect(usernameAction, SIGNAL(triggered(bool)), this, SLOT(typeAccountUserUI()));
		connect(passwordAction, SIGNAL(triggered(bool)), this, SLOT(typeAccountPassUI()));
	}
	connect(openAction, SIGNAL(triggered(bool)), this, SLOT(openAccountUI()));
	connect(deleteAction, SIGNAL(triggered(bool)), this, SLOT(deleteAccountUI()));
	connect(copyUsernameAction, SIGNAL(triggered(bool)), this, SLOT(copyUsername()));
	connect(copyPasswordAction, SIGNAL(triggered(bool)), this, SLOT(copyPassword()));
	if (!m_typeEnabled) {
		connect(copyUsernamePasswordAction, SIGNAL(triggered(bool)), this, SLOT(typeAccountUserPassUI()));
	}
	switch (m_quickTypeState) {
	case QUICKTYPE_STATE_INITIAL:
		menu->setActiveAction(browseAction);
		break;
	case QUICKTYPE_STATE_BROWSE:
		menu->setActiveAction(loginAction);
		break;
	case QUICKTYPE_STATE_USERNAME:
		menu->setActiveAction(passwordAction);
		break;
	default:
		break;
	}
	QRect r;
	m_parent->getSelectedAccountRect(r);
	QPoint p = r.bottomLeft();
	p.setX(p.x() + 15);
	p.setY(p.y() + 3);
	menu->exec(p);
}

int AccountActionBar::esdbType()
{
	return ESDB_TYPE_ACCOUNT;
}

void AccountActionBar::entrySelected(esdbEntry *entry)
{
	if (!entry)
		return;
	if (entry != m_selectedEntry) {
		m_quickTypeState = QUICKTYPE_STATE_INITIAL;
	}
	if (m_browseUrlButton) {
		QUrl url(entry->getUrl());
		m_browseUrlButton->setEnabled(entry && url.isValid() && !url.isEmpty());
	}
}

void AccountActionBar::browseUrlUI()
{
	esdbEntry *entry = selectedEntry();
	if (entry) {
		QUrl url(entry->getUrl());
		if (url.isValid() && !url.isEmpty()) {
			m_quickTypeState = QUICKTYPE_STATE_BROWSE;
			browseUrl(entry);
		}
	}
}

void AccountActionBar::accessAccountUI(bool typeData, bool username, bool password)
{
	account *acct = static_cast<account *>(selectedEntry());
	if (acct) {
		accessAccount(acct, typeData, username, password);
	}
}

void AccountActionBar::typeAccountUserUI()
{
	accessAccountUI(true, true, false);
}

void AccountActionBar::typeAccountPassUI()
{
	accessAccountUI(true, false, true);
}

void AccountActionBar::typeAccountUserPassUI()
{
	accessAccountUI(true, true, true);
}

void AccountActionBar::openAccountUI()
{
	openEntry(selectedEntry());
}

void AccountActionBar::deleteAccountUI()
{
	deleteEntry();
}

void AccountActionBar::copyUsername()
{
	esdbEntry *entry = selectedEntry();
	if (entry) {
		accessAccount(static_cast<account *>(entry), false, true, false);
	}
}

void AccountActionBar::copyPassword()
{
	esdbEntry *entry = selectedEntry();
	if (entry) {
		accessAccount(static_cast<account *>(entry), false, false, true);
	}
}

void AccountActionBar::accessAccount(account *acct, bool typeData, bool username, bool password)
{
	m_accessUsername = username;
	m_accessPassword = password;
	bool doTypeData = m_typeEnabled && typeData;
	QString message;
	if (username && password) {
		if (doTypeData) {
			message.append("to login to ");
		} else {
			message.append("to copy username and password for ");
		}
		message.append(acct->acctName);
	} else {
		if (username) {
			if (doTypeData)
				message.append("type username ");
			else
				message.append("copy username ");
			m_quickTypeState = QUICKTYPE_STATE_USERNAME;
		} else if (password) {
			if (m_typeEnabled && typeData)
				message.append("type password ");
			else
				message.append("copy password ");
			m_quickTypeState = QUICKTYPE_STATE_PASSWORD;
		}
		message.append("for ");
		message.append(acct->acctName);
	}
	accessEntry(acct, doTypeData ? INTENT_TYPE_ENTRY : INTENT_COPY_ENTRY,
            message, username && password && doTypeData);
}

bool AccountActionBar::typeAccountData(account *acct)
{
	if (QApplication::focusWindow()) {
		QMessageBox *box = SignetApplication::messageBoxError(
                       QMessageBox::Warning,
                       "Signet",
                       "A destination text area must be selected for typing to start\n\n"
                       "Click OK and try again.", this);
		connect(box, SIGNAL(finished(int)), this, SLOT(retryTypeData()));
		m_id = acct->id;
        return false;
	}
	QString keys;
	if (m_accessUsername) {
		keys.append(acct->userName);
	}
	if (m_accessPassword) {
		if (m_accessUsername) {
			keys.append("\t");
		}
		keys.append(acct->password);
		if (m_accessUsername) {
			keys.append("\n");
		}
	}
	QVector<u16> uKeys;
	for (auto key : keys) {
		uKeys.append(key.unicode());
	}
	if (!::signetdev_can_type_w((u16 *)uKeys.data(), uKeys.length())) {
		QMessageBox *msg = new QMessageBox(QMessageBox::Warning,
                           "Cannot type data",
                           "Signet cannot type this data. It contains characters not present in your keyboard layout.",
                           QMessageBox::NoButton, this);
		QPushButton *copyData = msg->addButton("Copy data", QMessageBox::AcceptRole);
		msg->addButton("Cancel", QMessageBox::RejectRole);
		msg->setWindowModality(Qt::WindowModal);
		activateWindow();
		raise();
		msg->exec();
		QAbstractButton *button = msg->clickedButton();
		if (button == copyData) {
			copyAccountData(acct, m_accessUsername, m_accessPassword);
		}
		msg->deleteLater();
        return true;
	} else {
		::signetdev_type_w(nullptr, &m_signetdevCmdToken,
                   (u16 *)uKeys.data(), uKeys.length());
        return true;
	}
}

bool AccountActionBar::copyAccountData(account *acct, bool username, bool password)
{
	QString s;
	if (username) {
		s = acct->userName;
	}
	if (password) {
		if (username) {
			s.append(" : ");
		}
		s.append(acct->password);
	}
	QClipboard *clipboard = QApplication::clipboard();
	clipboard->setText(s);
    return true;
}

bool AccountActionBar::accessEntryComplete(esdbEntry *entry, int intent)
{
	account *acct = static_cast<account *>(entry);
	switch (intent) {
	case INTENT_OPEN_ENTRY: {
		QStringList groupList;
		m_loggedInWidget->getCurrentGroups("Accounts", groupList);
		EditAccount *ea = new EditAccount(acct, groupList, m_parent);
		connect(ea, SIGNAL(abort()), this, SIGNAL(abort()));
		connect(ea, SIGNAL(entryChanged(int)), m_parent, SLOT(entryChanged(int)));
		connect(ea, SIGNAL(finished(int)), ea, SLOT(deleteLater()));
		ea->show();
        return true;
	}
	case INTENT_TYPE_ENTRY: {
        return typeAccountData(acct);
	}
	case INTENT_COPY_ENTRY: {
        return copyAccountData(acct, m_accessUsername, m_accessPassword);
	}
	}
    return true;
}

void AccountActionBar::entryCreated(esdbEntry *entry)
{
	m_parent->entryCreated("Accounts", entry);
}

void AccountActionBar::retryTypeData()
{
	if (m_parent->beginIDTask(selectedEntry()->id, LoggedInWidget::ID_TASK_READ, INTENT_TYPE_ENTRY, this)) {
		ButtonWaitWidget *buttonWaitWidget = m_parent->getButtonWaitWidget();
		buttonWaitWidget->resetTimeout();
	} else {
        m_parent->endButtonWait();
        m_parent->finishTask();
	}
}
