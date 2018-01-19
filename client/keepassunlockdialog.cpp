#include "keepassunlockdialog.h"
#include "signetapplication.h"

#include "core/Database.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QLineEdit>
#include <QFileDialog>
#include <QCheckBox>

#include <format/KeePass2Reader.h>
#include <keys/PasswordKey.h>
#include <keys/FileKey.h>
#include <core/Database.h>
#include <core/Group.h>
#include <core/Entry.h>

KeePassUnlockDialog::KeePassUnlockDialog(QFile *file, QWidget *parent) :
	QDialog(parent),
	m_keePassDatabase(NULL),
	m_databaseFile(file)
{
	QVBoxLayout *top = new QVBoxLayout();
	top->setAlignment(this, Qt::AlignTop);
	QPushButton *okay = new QPushButton("Ok");
	QPushButton *cancel = new QPushButton("Cancel");
	QHBoxLayout *password = new QHBoxLayout();
	QHBoxLayout *keyFile = new QHBoxLayout();
	QHBoxLayout *buttons = new QHBoxLayout();
	m_warnLabel = new QLabel("Database read failed");
	m_warnLabel->hide();
	m_warnLabel->setStyleSheet("QLabel { color : red; }");

	m_passwordCheckBox = new QCheckBox("Password");
	m_passwordCheckBox->setChecked(false);
	m_keyFileCheckBox = new QCheckBox("Key file");
	m_keyFileCheckBox->setChecked(false);
	m_passwordEdit = new QLineEdit();
	m_passwordEdit->setEchoMode(QLineEdit::Password);
	m_keyPathEdit = new QLineEdit();
	m_keyPathBrowse = new QPushButton("Browse");
	password->addWidget(m_passwordCheckBox);
	password->addWidget(m_passwordEdit);
	keyFile->addWidget(m_keyFileCheckBox);
	keyFile->addWidget(m_keyPathEdit);
	keyFile->addWidget(m_keyPathBrowse);
	buttons->addWidget(okay);
	buttons->addWidget(cancel);
	top->addWidget(new QLabel("Enter password and/or key file to read database"));
	top->addLayout(password);
	top->addLayout(keyFile);
	top->addWidget(m_warnLabel);
	top->addLayout(buttons);

	connect(m_passwordCheckBox, SIGNAL(toggled(bool)),
		this, SLOT(passwordCheckBoxToggled(bool)));
	connect(m_keyFileCheckBox, SIGNAL(toggled(bool)),
		this, SLOT(keyFileCheckBoxToggled(bool)));
	connect(m_passwordEdit, SIGNAL(textEdited(QString)),
		this, SLOT(passwordTextEdited()));
	connect(m_keyPathEdit, SIGNAL(textEdited(QString)),
		this, SLOT(keyFileTextEdited()));
	connect(okay, SIGNAL(pressed()),
		this, SLOT(okayPressed()));
	connect(cancel, SIGNAL(pressed()),
		this, SLOT(cancelPressed()));
	connect(m_keyPathBrowse, SIGNAL(pressed()),
		this, SLOT(keyPathBrowse()));
	setLayout(top);
}

void KeePassUnlockDialog::okayPressed()
{
	KeePass2Reader *r = new KeePass2Reader();
	CompositeKey k;
	QString password = m_passwordEdit->text();
	if (m_passwordCheckBox->isChecked()) {
		k.addKey(PasswordKey(password));
	}
	QString keyPath = m_keyPathEdit->text();
	if (m_keyFileCheckBox->isChecked()) {
		FileKey f;
		if (f.load(keyPath)) {
			k.addKey(f);
		} else {
			QMessageBox *mb = SignetApplication::messageBoxError(QMessageBox::Warning,
								windowTitle(),
								"Key file invalid",
								this);
			mb->exec();
			mb->deleteLater();
			m_keyFileCheckBox->setChecked(false);
			m_keyPathEdit->setText("");
			return;
		}
	}
	m_databaseFile->reset();
	m_keePassDatabase = r->readDatabase(m_databaseFile, k);
	if (!m_keePassDatabase) {
		m_warnLabel->show();
	} else {
		done(0);
	}
}

void KeePassUnlockDialog::cancelPressed()
{
	done(1);
}

void KeePassUnlockDialog::keyPathBrowse()
{
	QFileDialog fd(this);
	fd.setAcceptMode(QFileDialog::AcceptOpen);
	fd.setFileMode(QFileDialog::AnyFile);
	fd.exec();
	QStringList fl;
	fl = fd.selectedFiles();
	if (fl.size()) {
		m_keyPathEdit->setText(fl.at(0));
		m_keyFileCheckBox->setChecked(true);
	}
}

void KeePassUnlockDialog::passwordTextEdited()
{
	m_passwordCheckBox->setChecked(true);
}

void KeePassUnlockDialog::keyFileTextEdited()
{
	m_keyFileCheckBox->setChecked(true);
}

void KeePassUnlockDialog::passwordCheckBoxToggled(bool val)
{
	if (!val) {
		m_passwordEdit->setText("");
	}
}

void KeePassUnlockDialog::keyFileCheckBoxToggled(bool val)
{
	if (!val) {
		m_keyPathEdit->setText("");
	}
}
