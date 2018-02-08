#include "csvimportconfigure.h"

#include <QComboBox>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>

#include "csvimporter.h"
#include "esdbtypemodule.h"

CSVImportConfigure::CSVImportConfigure(CSVImporter *importer, QWidget *parent) :
	QDialog(parent),
	m_importer(importer),
	m_dataTypeCombo(NULL),
	m_typeModule(NULL)
{
	QVBoxLayout *top = new QVBoxLayout();
	QHBoxLayout *dataType = new QHBoxLayout();
	QHBoxLayout *buttons = new QHBoxLayout();

	m_dataTypeCombo = new QComboBox();
	m_dataTypeCombo->setMinimumWidth(200);

	auto typeModules = m_importer->typeModules();
	for (auto typeModule : typeModules) {
		m_dataTypeCombo->addItem(typeModule->name());
	}

	QPushButton *okButton = new QPushButton("Ok");
	QPushButton *cancelButton = new QPushButton("Cancel");
	buttons->addWidget(cancelButton);
	buttons->addWidget(okButton);
	dataType->addWidget(new QLabel("Data Type"));
	dataType->addWidget(m_dataTypeCombo);
	top->addLayout(dataType);
	top->addLayout(buttons);
	setLayout(top);
	connect(okButton, SIGNAL(pressed()), this, SLOT(okPressed()));
	connect(cancelButton, SIGNAL(pressed()), this, SLOT(cancelPressed()));
}

void CSVImportConfigure::okPressed()
{
	auto typeModules = m_importer->typeModules();
	m_typeModule = typeModules.at(m_dataTypeCombo->currentIndex());
	done(0);
}

void CSVImportConfigure::cancelPressed()
{
	done(1);
}
