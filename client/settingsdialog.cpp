#include "settingsdialog.h"
#include "mainwindow.h"
#include "keyboardlayouttester.h"

#include <QCheckBox>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>

#include <QPushButton>
#include <QLineEdit>
#include <QSpinBox>
#include <QFileDialog>
#include <QDir>

SettingsDialog::SettingsDialog(MainWindow *mainWindow, bool initial) :
	QDialog(mainWindow),
	m_initial(initial)
{
	m_settings = mainWindow->getSettings();
	setWindowTitle("System settings");

	QPushButton *okayButton = new QPushButton("&Save");
	connect(okayButton, SIGNAL(pressed()), this, SLOT(okayPressed()));

	QPushButton *cancelButton = new QPushButton("Cancel");
	connect(cancelButton, SIGNAL(pressed()), this, SLOT(cancelPressed()));

	m_activeKeyboardLayout = m_settings->activeKeyboardLayout;
	m_keyboardLayouts = m_settings->keyboardLayouts;

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

	bool keyboardLayoutConfigured = !m_settings->keyboardLayouts.isEmpty();
	QHBoxLayout *keyboardLayoutConfigurationLayout = new QHBoxLayout();
	if (keyboardLayoutConfigured) {
		keyboardLayoutConfigurationLayout->addWidget(new QLabel("Keyboard layout"));
		m_configureKeyboardLayout = new QPushButton("Reconfigure");
	} else {
		keyboardLayoutConfigurationLayout->addWidget(new QLabel("Keyboard layout"));
		m_configureKeyboardLayout = new QPushButton("Configure");
	}
	keyboardLayoutConfigurationLayout->addWidget(m_configureKeyboardLayout);
	connect(m_configureKeyboardLayout, SIGNAL(pressed()),
		this, SLOT(configureKeyboardLayout()));

	m_keyboardLayoutUnconfiguredWarning = new QLabel(
				"Note: Signet's keyboard layout is not configured. Signet will default to a English(US) keyboard layout which will cause incorrect keys to be generated if this system uses a different layout.");
	m_keyboardLayoutUnconfiguredWarning->setWordWrap(true);
	m_keyboardLayoutUnconfiguredWarning->setStyleSheet("font-weight: bold");
	if (keyboardLayoutConfigured) {
		m_keyboardLayoutUnconfiguredWarning->hide();
	}

	QVBoxLayout *topLayout = new QVBoxLayout();
	topLayout->setAlignment(Qt::AlignTop);
	topLayout->addWidget(m_localBackups);
	topLayout->addLayout(localBackupPathLayout);
	topLayout->addLayout(localBackupIntervalLayout);
	topLayout->addWidget(m_removableBackups);
	topLayout->addLayout(removableBackupVolumeLayout);
	topLayout->addLayout(removableBackupDirectoryLayout);
	topLayout->addLayout(removableBackupIntervalLayout);
	topLayout->addWidget(m_keyboardLayoutUnconfiguredWarning);
	topLayout->addLayout(keyboardLayoutConfigurationLayout);
	topLayout->addLayout(buttonLayout);
	setLayout(topLayout);
	setEnableDisable();
	connect(m_localBackups, SIGNAL(clicked(bool)), this, SLOT(setEnableDisable()));
	connect(m_removableBackups, SIGNAL(clicked(bool)), this, SLOT(setEnableDisable()));
}

void SettingsDialog::configureKeyboardLayout()
{
	QVector<struct signetdev_key> currentLayout;

	int n_keys;
	const signetdev_key *keymap = ::signetdev_get_keymap(&n_keys);
	for (int i = 0; i < n_keys; i++) {
		currentLayout.append(keymap[i]);
	}

	m_keyboardLayoutTester = new KeyboardLayoutTester(currentLayout, this);
	connect(m_keyboardLayoutTester, SIGNAL(closing(bool)),
		this, SLOT(keyboardLayoutTesterClosing(bool)));
	connect(m_keyboardLayoutTester, SIGNAL(applyChanges()),
		this, SLOT(applyKeyboardLayoutChanges()));
	m_keyboardLayoutTester->setWindowModality(Qt::WindowModal);
	m_keyboardLayoutTester->show();
}

void SettingsDialog::applyKeyboardLayoutChanges()
{
	auto keyboardLayout = m_keyboardLayoutTester->getLayout();
	::signetdev_set_keymap(keyboardLayout.data(), keyboardLayout.size());
	m_activeKeyboardLayout = QString("current");
	m_keyboardLayouts.insert("current", keyboardLayout);
	m_keyboardLayoutUnconfiguredWarning->hide();
	m_configureKeyboardLayout->setText("Reconfigure");
}

void SettingsDialog::keyboardLayoutTesterClosing(bool applyChanges)
{
	if (applyChanges) {
		applyKeyboardLayoutChanges();
	}
	m_keyboardLayoutTester->deleteLater();
}


void SettingsDialog::setEnableDisable()
{
	bool enableLocal = m_localBackups->isChecked();
	m_localBackupPath->setEnabled(enableLocal);
	m_localBackupInterval->setEnabled(enableLocal);

	bool enableRemovable = m_removableBackups->isChecked();
	m_removableBackupVolume->setEnabled(enableRemovable);
	m_removableBackupDirectory->setEnabled(enableRemovable);
	m_removableBackupInterval->setEnabled(enableRemovable);
}

void SettingsDialog::okayPressed()
{
	m_settings->localBackups = m_localBackups->isChecked();
	m_settings->localBackupPath = m_localBackupPath->text();
	m_settings->localBackupInterval = m_localBackupInterval->value();
	m_settings->removableBackups = m_removableBackups->isChecked();
	m_settings->removableBackupPath = m_removableBackupDirectory->text();
	m_settings->removableBackupVolume = m_removableBackupVolume->text();
	m_settings->removableBackupInterval = m_removableBackupInterval->value();
	m_settings->activeKeyboardLayout = m_activeKeyboardLayout;
	m_settings->keyboardLayouts = m_keyboardLayouts;
	done(0);
}

void SettingsDialog::cancelPressed()
{
	done(1);
}

void SettingsDialog::localBackupPathBrowse()
{
	QDir dir(m_localBackupPath->text());
	QFileDialog fd(this, "Local backup directory");
	fd.setDirectory(dir);
	fd.setFileMode(QFileDialog::Directory);
	fd.setWindowModality(Qt::WindowModal);
	fd.exec();
	QStringList sl = fd.selectedFiles();
	if (sl.empty())
		return;
	m_localBackupPath->setText(sl.first());
}
