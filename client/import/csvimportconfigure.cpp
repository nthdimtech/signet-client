#include "csvimportconfigure.h"

#include <QComboBox>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QLineEdit>
#include <QSet>

#include "csvimporter.h"
#include "esdbtypemodule.h"
#include "generictext.h"
#include "emphasistext.h"
#include "errortext.h"

CSVImportConfigure::CSVImportConfigure(CSVImporter *importer,
		const QSet<QString> &usedTypeNames,
		QString basename,
		QString filename,
		QWidget *parent) :
	QDialog(parent),
	m_usedTypeNames(usedTypeNames),
	m_importer(importer),
        m_dataTypeCombo(nullptr),
        m_typeModule(nullptr),
	m_basename(basename)
{
	QVBoxLayout *top = new QVBoxLayout();
	QHBoxLayout *buttons = new QHBoxLayout();
	QHBoxLayout *selection = new QHBoxLayout();
	QHBoxLayout *selectionNew = new QHBoxLayout();
	QHBoxLayout *prompt = new QHBoxLayout();

	m_dataTypeCombo = new QComboBox();
	m_dataTypeCombo->setMinimumWidth(200);
	m_dataTypeEdit = new QLineEdit();

	auto typeModules = m_importer->typeModules();
	m_dataTypeCombo->addItem("<New type>",QVariant(QString("")));
	for (auto typeModule : typeModules) {
		m_dataTypeCombo->addItem(typeModule->name(),typeModule->name());
	}

	m_selectionIndex = 0;
	for (int i = 1; i < m_dataTypeCombo->count(); i++) {
		if (!m_dataTypeCombo->itemData(i).toString().compare(basename, Qt::CaseInsensitive)) {
			m_selectionIndex = i;
			break;
		}
	}
	m_dataTypeCombo->setCurrentIndex(m_selectionIndex);

	m_errorLabel = new errorText("");

	QPushButton *okButton = new QPushButton("Ok");
	QPushButton *skipButton = new QPushButton("Skip");
	QPushButton *cancelButton = new QPushButton("Cancel");
	buttons->addWidget(okButton);
	buttons->addWidget(skipButton);
	buttons->addWidget(cancelButton);
	prompt->setAlignment(Qt::AlignLeft);
	prompt->addWidget(new genericText("Import file"));
	prompt->addWidget(new emphasisText("\"" + filename + "\""));
	selection->addWidget(new genericText("Destination"));
	selection->addWidget(m_dataTypeCombo);
	selection->addWidget(m_dataTypeEdit);
	top->addLayout(prompt);
	top->addLayout(selection);
	top->addWidget(m_errorLabel);
	top->addLayout(buttons);
	top->setAlignment(Qt::AlignTop);
	if (m_selectionIndex == 0) {
		m_dataTypeEdit->setText(basename);
		m_dataTypeEdit->setEnabled(true);
	} else {
		m_dataTypeEdit->setEnabled(false);
	}
	m_errorLabel->setVisible(false);
	setLayout(top);
	connect(okButton, SIGNAL(pressed()), this, SLOT(okPressed()));
	connect(skipButton, SIGNAL(pressed()), this, SLOT(skipPressed()));
	connect(cancelButton, SIGNAL(pressed()), this, SLOT(cancelPressed()));
	connect(m_dataTypeCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(currentSelectionIndexChanged(int)));
	connect(m_dataTypeEdit, SIGNAL(textEdited(QString)), this, SLOT(dataTypeTextEdited()));
}

void CSVImportConfigure::dataTypeTextEdited()
{
	m_errorLabel->setVisible(false);
}

void CSVImportConfigure::currentSelectionIndexChanged(int idx)
{
	m_selectionIndex = idx;
	if (m_selectionIndex == 0) {
		m_dataTypeEdit->setText(m_basename);
		m_dataTypeEdit->setEnabled(true);
	} else {
		m_dataTypeEdit->setEnabled(false);
		m_errorLabel->setVisible(false);
	}
}

void CSVImportConfigure::skipPressed()
{
	done(m_skippedResponseCode);
}

void CSVImportConfigure::okPressed()
{
	if (m_selectionIndex == 0) {
		QString typeName = m_dataTypeEdit->text();
		if (typeName.isEmpty()) {
			m_errorLabel->setText("Select a new data type name");
			m_errorLabel->setVisible(true);
		} else if (m_usedTypeNames.contains(typeName)) {
			m_errorLabel->setText("Type name \"" + typeName + "\" is already used");
			m_errorLabel->setVisible(true);
		} else {
			done(QDialog::Accepted);
		}
	} else {
		done(QDialog::Accepted);
	}
}

QString CSVImportConfigure::selectedType(bool &isNew)
{
	if (m_selectionIndex == 0) {
		isNew = true;
		return m_dataTypeEdit->text();
	} else {
		isNew = false;
		return m_dataTypeCombo->itemData(m_selectionIndex).toString();
	}
}

void CSVImportConfigure::cancelPressed()
{
	done(QDialog::Rejected);
}
