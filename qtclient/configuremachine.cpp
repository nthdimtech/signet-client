#include "configuremachine.h"

#include "mainwindow.h"

#include <QCheckBox>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>

#include <QPushButton>
#include <QLineEdit>

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

	m_removableBackups = new QCheckBox("Enable removable backups");
	m_removableBackups->setChecked(m_settings->removableBackups);

	m_removableBackupVolume = new QLineEdit(m_settings->removableBackupVolume);
	m_removableBackupDirectory = new QLineEdit(m_settings->removableBackupPath);

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

	QHBoxLayout *removableBackupVolumeLayout = new QHBoxLayout();
	removableBackupVolumeLayout->addWidget(new QLabel("Removeable backup volume label"));
	removableBackupVolumeLayout->addWidget(m_removableBackupVolume);

	QHBoxLayout *removableBackupDirectoryLayout = new QHBoxLayout();
	removableBackupDirectoryLayout->addWidget(new QLabel("Removeable backup directory name"));
	removableBackupDirectoryLayout->addWidget(m_removableBackupDirectory);

	QVBoxLayout *topLayout = new QVBoxLayout();
	topLayout->setAlignment(Qt::AlignTop);
	topLayout->addWidget(m_localBackups);
	topLayout->addLayout(localBackupPathLayout);
	topLayout->addWidget(m_removableBackups);
	topLayout->addLayout(removableBackupVolumeLayout);
	topLayout->addLayout(removableBackupDirectoryLayout);
	topLayout->addLayout(buttonLayout);
	setLayout(topLayout);
}

void ConfigureMachine::okayPressed()
{
	m_settings->localBackups = m_localBackups->isChecked();
	m_settings->localBackupPath = m_localBackupPath->text();
	m_settings->removableBackups = m_removableBackups->isChecked();
	m_settings->removableBackupPath = m_removableBackupDirectory->text();
	m_settings->removableBackupVolume = m_removableBackupVolume->text();
	done(0);
}

void ConfigureMachine::cancelPressed()
{
	done(1);
}

void ConfigureMachine::localBackupPathBrowse()
{

}
