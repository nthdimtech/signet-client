#ifndef CONFIGUREMACHINE_H
#define CONFIGUREMACHINE_H

#include <QObject>
#include <QWidget>
#include <QDialog>

class MainWindow;
struct appSettings;
class QCheckBox;
class QLineEdit;
class QSpinBox;

class ConfigureMachine : public QDialog
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
	appSettings *m_settings;
public:
	ConfigureMachine(MainWindow *mainWindow, bool initial);
public slots:
	void okayPressed();
	void cancelPressed();
	void localBackupPathBrowse();
	void setEnableDisable();
};

#endif // CONFIGUREMACHINE_H
