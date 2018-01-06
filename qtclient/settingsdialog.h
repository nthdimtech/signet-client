#ifndef SETTINGSDIALOG_H
#define SETTINGSDIALOG_H

#include <QObject>
#include <QWidget>
#include <QDialog>

class MainWindow;
class QCheckBox;
class QLineEdit;
class QSpinBox;

#include "keyboardlayouttester.h"
#include "localsettings.h"

class SettingsDialog : public QDialog
{
Q_OBJECT
	bool m_initial;
	QCheckBox *m_localBackups;
	QLineEdit *m_localBackupPath;
	QSpinBox  *m_localBackupInterval;
	QCheckBox *m_removableBackups;
	QLineEdit *m_removableBackupVolume;
	QLineEdit *m_removableBackupDirectory;
	QSpinBox  *m_removableBackupInterval;
	localSettings *m_settings;
	QPushButton *m_configureKeyboardLayout;
	KeyboardLayoutTester *m_keyboardLayoutTester;
	QMap<QString, keyboardLayout> m_keyboardLayouts;
	QString m_activeKeyboardLayout;
public:
	SettingsDialog(MainWindow *mainWindow, bool initial);
public slots:
	void okayPressed();
	void cancelPressed();
	void localBackupPathBrowse();
	void setEnableDisable();
	void configureKeyboardLayout();
	void applyKeyboardLayoutChanges();
	void keyboardLayoutTesterClosing(bool applyChanges);
};

#endif // SETTINGSDIALOG_H
