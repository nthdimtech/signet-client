#include "editaccount.h"
#include <QBoxLayout>
#include <QLineEdit>
#include <QLabel>
#include <QPushButton>
#include <QMessageBox>
#include <QString>
#include <QCheckBox>
#include <QUrl>
#include <QDesktopServices>
#include <QCloseEvent>
#include <passwordedit.h>

#include "databasefield.h"
#include "account.h"
#include "buttonwaitdialog.h"
#include "signetapplication.h"
#include "genericfieldseditor.h"

extern "C" {
#include "signetdev/host/signetdev.h"
};

EditAccount::EditAccount(int id, QString entryName, QStringList groupList, QWidget *parent) :
	EditEntryDialog("Account", id, parent),
    m_acct(nullptr),
    m_groupList(groupList)
{
	setup(entryName);
}

EditAccount::EditAccount(account *acct, QStringList groupList, QWidget *parent) :
	EditEntryDialog("Account", acct, parent),
    m_acct(acct),
    m_groupList(groupList)
{
	setup(acct->acctName);
	m_settingFields = true;
	setAccountValues();
	m_settingFields = false;
}

#include "groupdatabasefield.h"


void EditAccount::setup(QString name)
{
	m_accountNameEdit = new QLineEdit(name);
	m_accountNameEdit->setReadOnly(SignetApplication::get()->isDeviceEmulated());

	m_genericFieldsEditor = new GenericFieldsEditor(QList<fieldSpec>());

	QBoxLayout *account_name_layout = new QBoxLayout(QBoxLayout::LeftToRight);
	account_name_layout->addWidget(new QLabel("Account name"));
	account_name_layout->addWidget(m_accountNameEdit);
	connect(m_accountNameEdit, SIGNAL(textEdited(QString)), this, SLOT(accountNameEdited()));
	connect(m_accountNameEdit, SIGNAL(textEdited(QString)), this, SLOT(entryNameEdited()));

	m_accountNameWarning = new QLabel();
	m_accountNameWarning->setStyleSheet("QLabel { color : red; }");
	m_accountNameWarning->hide();

    m_usernameField = new DatabaseField("username", 120, nullptr);
    m_groupField = new GroupDatabaseField(120, m_groupList ,nullptr);
    m_emailField = new DatabaseField("email", 120, nullptr);
	m_passwordEdit = new PasswordEdit();
	m_browseUrlButton = new QPushButton(QIcon(":/images/browse.png"),"");
	m_browseUrlButton->setToolTip("Browse");
	connect(m_browseUrlButton, SIGNAL(pressed()), this, SLOT(browseUrl()));

    connect(m_usernameField, SIGNAL(editingFinished()), this, SLOT(usernameEditingFinished()));

	m_urlField = new DatabaseField("URL", 140, m_browseUrlButton);

	connect(m_accountNameEdit, SIGNAL(textEdited(QString)),
		this, SLOT(edited()));
	connect(m_passwordEdit, SIGNAL(textEdited(QString)),
		this, SLOT(edited()));
	connect(m_usernameField, SIGNAL(textEdited(QString)),
		this, SLOT(edited()));
	connect(m_emailField, SIGNAL(textEdited(QString)),
		this, SLOT(edited()));
	connect(m_urlField, SIGNAL(textEdited(QString)),
        this, SLOT(edited()));
	connect(m_genericFieldsEditor, SIGNAL(edited()),
		this, SLOT(edited()));
	connect(m_groupField, SIGNAL(textEdited(QString)),
		this, SLOT(edited()));

	QBoxLayout *mainLayout = new QBoxLayout(QBoxLayout::TopToBottom);
	mainLayout->setAlignment(Qt::AlignTop);
	mainLayout->addLayout(account_name_layout);
	mainLayout->addWidget(m_accountNameWarning);
	mainLayout->addWidget(m_groupField);
	mainLayout->addWidget(m_usernameField);
	mainLayout->addWidget(m_emailField);
	mainLayout->addWidget(m_passwordEdit);
	mainLayout->addWidget(m_urlField);
	mainLayout->addWidget(m_genericFieldsEditor);
	EditEntryDialog::setup(mainLayout);
}

EditAccount::~EditAccount()
{
}


void EditAccount::accountNameEdited()
{
	m_accountNameWarning->hide();
}

void EditAccount::browseUrl()
{
	QUrl url(m_urlField->text());
	QString scheme = url.scheme();
	if (!scheme.size()) {
		url.setScheme("HTTP");
	}
	QDesktopServices::openUrl(url);
}

void EditAccount::setAccountValues()
{
	m_usernameField->setText(m_acct->userName);
	m_accountNameEdit->setText(m_acct->acctName);
	m_emailField->setText(m_acct->email);
	m_passwordEdit->setPassword(m_acct->password);
	m_urlField->setText(m_acct->url);
	m_groupField->setText(m_acct->path);
	m_genericFieldsEditor->loadFields(m_acct->fields);
}

QString EditAccount::entryName()
{
	return m_accountNameEdit->text();
}

void EditAccount::applyChanges(esdbEntry *ent)
{
	account *acct = static_cast<account *>(ent);
	acct->acctName = m_accountNameEdit->text();
	acct->userName = m_usernameField->text();
	acct->password = m_passwordEdit->password();
	acct->url = m_urlField->text();
	acct->email = m_emailField->text();
	acct->path = m_groupField->text();
	acct->fields = acct->fields;
	m_genericFieldsEditor->saveFields(acct->fields);
}

void EditAccount::usernameEditingFinished()
{
    if (m_emailField->text().isEmpty() && isEmail(m_usernameField->text())) {
        m_emailField->setText(m_usernameField->text());
    }
}

esdbEntry *EditAccount::createEntry(int id)
{
	account *acct = new account(id);
	return acct;
}

void EditAccount::undoChanges()
{
	if (m_acct) {
		setAccountValues();
	}
}
