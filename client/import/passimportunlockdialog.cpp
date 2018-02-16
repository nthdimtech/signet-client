#include "passimportunlockdialog.h"

#ifdef Q_OS_UNIX
#include "passimporter.h"

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QLineEdit>
#include <QProcess>

PassImportUnlockDialog::PassImportUnlockDialog(PassImporter *importer, QWidget *parent) :
	QDialog(parent),
	m_importer(importer)
{
	QVBoxLayout *top = new QVBoxLayout();
	top->setAlignment(this, Qt::AlignTop);
	QPushButton *okay = new QPushButton("Ok");
	QPushButton *cancel = new QPushButton("Cancel");
	QHBoxLayout *passphrase = new QHBoxLayout();
	QHBoxLayout *buttons = new QHBoxLayout();
	m_passphraseEdit = new QLineEdit();
	m_passphraseEdit->setEchoMode(QLineEdit::Password);
	m_warnLabel = new QLabel("GPG passphrase incorrect");
	m_warnLabel->setStyleSheet("QLabel { color : red; }");
	m_warnLabel->hide();

	m_passphraseEdit->setMinimumWidth(200);
	passphrase->addWidget(new QLabel("Passphrase"));
	passphrase->addWidget(m_passphraseEdit);
	buttons->addWidget(okay);
	buttons->addWidget(cancel);

	top->addLayout(passphrase);
	top->addWidget(m_warnLabel);
	top->addLayout(buttons);
	setLayout(top);

	connect(okay, SIGNAL(pressed()),
		this, SLOT(okayPressed()));
	connect(cancel, SIGNAL(pressed()),
		this, SLOT(cancelPressed()));
	connect(m_passphraseEdit, SIGNAL(textEdited(QString)),
		this, SLOT(passphraseTextEdited()));
}

QString PassImportUnlockDialog::passphrase()
{
	return m_passphraseEdit->text();
}

void PassImportUnlockDialog::okayPressed()
{
	if (m_importer->passphraseCheck(m_passphraseEdit->text())) {
		done(0);
	} else {
		m_warnLabel->show();
	}
}

void PassImportUnlockDialog::cancelPressed()
{
	done(1);
}

void PassImportUnlockDialog::passphraseTextEdited()
{
	m_warnLabel->hide();
}
#endif
