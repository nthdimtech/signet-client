#include "entryrenamedialog.h"

#include <QLineEdit>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QString>
#include <QDialog>

EntryRenameDialog::EntryRenameDialog(QString initialName, QWidget *parent) :
	QDialog(parent),
	m_okayPressed(false)
{
	setWindowModality(Qt::WindowModal);
	QHBoxLayout *main = new QHBoxLayout();
	QVBoxLayout *top = new QVBoxLayout();

	m_warningLabel = new QLabel("New name must not be empty");
	m_warningLabel->setStyleSheet("QLabel { color : red; }");
	m_warningLabel->hide();

	top->addLayout(main);
	top->addWidget(m_warningLabel);
	setLayout(top);

	setLayout(new QVBoxLayout());
	QPushButton *okay = new QPushButton("Ok");
	QPushButton *cancel = new QPushButton("Cancel");
	m_newNameEdit = new QLineEdit();
	connect(okay, SIGNAL(pressed()), this, SLOT(okayPressed()));
	connect(cancel, SIGNAL(pressed()), this, SLOT(cancelPressed()));
	connect(m_newNameEdit, SIGNAL(textEdited(QString)), this, SLOT(textEdited(QString)));
	main->addWidget(new QLabel("New entry name:"));
	main->addWidget(m_newNameEdit);
	main->addWidget(okay);
	main->addWidget(cancel);
	m_newNameEdit->setText(initialName);
}

QString EntryRenameDialog::newName()
{
	return m_newNameEdit->text();
}

void EntryRenameDialog::okayPressed()
{
	if (!m_newNameEdit->text().size()) {
		m_warningLabel->show();
		return;
	} else {
		m_okayPressed = true;
		close();
	}
}

void EntryRenameDialog::cancelPressed()
{
	close();
}

void EntryRenameDialog::textEdited(QString s)
{
	if (s.size())
		m_warningLabel->hide();
}
