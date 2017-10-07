#include "configuremachine.h"

#include "mainwindow.h"

#include <QCheckBox>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>

#include <QPushButton>
#include <QLineEdit>
#include <QSpinBox>

ConfigureMachine::ConfigureMachine(MainWindow *mainWindow, bool initial) :
	QDialog(mainWindow),
	m_initial(initial)
{
	m_settings = mainWindow->getSettings();
	setWindowTitle("System settings");

	QPushButton *okayButton = new QPushButton("Ok");
	connect(okayButton, SIGNAL(pressed()), this, SLOT(okayPressed()));

	QPushButton *cancelButton = new QPushButton("Cancel");
	connect(cancelButton, SIGNAL(pressed()), this, SLOT(cancelPressed()));

	m_localBackups = new QCheckBox("Enable local backups");
	m_localBackups->setChecked(m_settings->localBackups);

	m_localBackupPath = new QLineEdit(m_settings->localBackupPath);
	m_localBackupPath->setMinimumWidth(300);

	m_localBackupInterval = new QSpinBox();
	m_localBackupInterval->setSuffix(" days");
	m_localBackupInterval->setMinimum(1);
	m_localBackupInterval->setMaximum(60);
	m_localBackupInterval->setValue(m_settings->localBackupInterval);

	m_removableBackups = new QCheckBox("Enable removable backups");
	m_removableBackups->setChecked(m_settings->removableBackups);

	m_removableBackupVolume = new QLineEdit(m_settings->removableBackupVolume);
	m_removableBackupDirectory = new QLineEdit(m_settings->removableBackupPath);

	m_removableBackupInterval = new QSpinBox();
	m_removableBackupInterval->setSuffix(" days");
	m_removableBackupInterval->setMinimum(1);
	m_removableBackupInterval->setMaximum(60);
	m_removableBackupInterval->setValue(m_settings->removableBackupInterval);

	QPushButton *localBackupPathBrowseButton = new QPushButton("Browse");
	connect(localBackupPathBrowseButton, SIGNAL(pressed()), this, SLOT(localBackupPathBrowse()));

	QLayout *buttonLayout = new QHBoxLayout();
	buttonLayout->setAlignment(Qt::AlignRight);
	buttonLayout->addWidget(okayButton);
	buttonLayout->addWidget(cancelButton);

	QHBoxLayout *localBackupPathLayout = new QHBoxLayout();
	localBackupPathLayout->addWidget(new QLabel("Local backup path"));
	localBackupPathLayout->addWidget(m_localBackupPath);
	localBackupPathLayout->addWidget(localBackupPathBrowseButton);

	QHBoxLayout *localBackupIntervalLayout = new QHBoxLayout();
	localBackupIntervalLayout->addWidget(new QLabel("Local backup every"));
	localBackupIntervalLayout->addWidget(m_localBackupInterval);

	QHBoxLayout *removableBackupVolumeLayout = new QHBoxLayout();
	removableBackupVolumeLayout->addWidget(new QLabel("Removeable backup volume label"));
	removableBackupVolumeLayout->addWidget(m_removableBackupVolume);

	QHBoxLayout *removableBackupDirectoryLayout = new QHBoxLayout();
	removableBackupDirectoryLayout->addWidget(new QLabel("Removeable backup directory name"));
	removableBackupDirectoryLayout->addWidget(m_removableBackupDirectory);

	QHBoxLayout *removableBackupIntervalLayout = new QHBoxLayout();
	removableBackupIntervalLayout->addWidget(new QLabel("Removable backup every"));
	removableBackupIntervalLayout->addWidget(m_removableBackupInterval);

	QVBoxLayout *topLayout = new QVBoxLayout();
	topLayout->setAlignment(Qt::AlignTop);
	topLayout->addWidget(m_localBackups);
	topLayout->addLayout(localBackupPathLayout);
	topLayout->addLayout(localBackupIntervalLayout);
	topLayout->addWidget(m_removableBackups);
	topLayout->addLayout(removableBackupVolumeLayout);
	topLayout->addLayout(removableBackupDirectoryLayout);
	topLayout->addLayout(removableBackupIntervalLayout);
	topLayout->addLayout(buttonLayout);
	setLayout(topLayout);
	setEnableDisable();
	connect(m_localBackups, SIGNAL(clicked(bool)), this, SLOT(setEnableDisable()));
	connect(m_removableBackups, SIGNAL(clicked(bool)), this, SLOT(setEnableDisable()));
}

void ConfigureMachine::setEnableDisable()
{
	bool enableLocal = m_localBackups->isChecked();
	m_localBackupPath->setEnabled(enableLocal);
	m_localBackupInterval->setEnabled(enableLocal);

	bool enableRemovable = m_removableBackups->isChecked();
	m_removableBackupVolume->setEnabled(enableRemovable);
	m_removableBackupDirectory->setEnabled(enableRemovable);
	m_removableBackupInterval->setEnabled(enableRemovable);
}

void ConfigureMachine::okayPressed()
{
	m_settings->localBackups = m_localBackups->isChecked();
	m_settings->localBackupPath = m_localBackupPath->text();
	m_settings->localBackupInterval = m_localBackupInterval->value();
	m_settings->removableBackups = m_removableBackups->isChecked();
	m_settings->removableBackupPath = m_removableBackupDirectory->text();
	m_settings->removableBackupVolume = m_removableBackupVolume->text();
	m_settings->removableBackupInterval = m_removableBackupInterval->value();
	done(0);
}

void ConfigureMachine::cancelPressed()
{
	done(1);
}

void ConfigureMachine::localBackupPathBrowse()
{

}
