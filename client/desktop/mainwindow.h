#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QComboBox>
#include <QList>
#include <QListView>
#include <QLineEdit>
#include <QMessageBox>
#include <QMainWindow>
#include <QApplication>

#include "esdbmodel.h"
#include "systemtray.h"
#include "signetapplication.h"
#include "localsettings.h"

#include <QVector>

struct signetdevCmdRespInfo;

namespace Ui
{
class MainWindow;
}

#include <map>
#include <QCloseEvent>
#include <QAction>
#include <QTimer>
#include <QDateTime>
#include <QString>

class ButtonWaitDialog;
class QFile;
class LoginWindow;
class QProgressBar;
class QMenu;
class QStackedWidget;
class QByteArray;
struct esdbTypeModule;
class KeyboardLayoutTester;
class Database;
class keePassImportController;
class DatabaseImportController;
class DatabaseImporter;
class LoggedInWidget;

struct fwSection {
	QString name;
	unsigned int lma;
	int size;
	QByteArray contents;
};

struct exportType {
	QList<QVector<QString> > m_data;
};

class MainWindow : public QMainWindow
{
	Q_OBJECT
	QString m_dbFilename;
	localSettings m_settings;
	esdbTypeModule *m_genericTypeModule;
	esdbTypeModule *m_accountTypeModule;
	esdbTypeModule *m_bookmarkTypeModule;
	LoggedInWidget *m_loggedInWidget;
public:
	explicit MainWindow(QString dbFilename, QWidget *parent = 0);
	void closeEvent(QCloseEvent *event);
	~MainWindow();

	static QString csvQuote(const QString &s);

	localSettings *getSettings() {
		return &m_settings;
	}

	QString getDatabaseFileName() {
		return m_dbFilename;
	}
private:
	Database *m_keePassDatabase;
	QProgressBar *m_wipeProgress;
	QWidget *m_wipingWidget;
	bool m_connected;
	bool m_quitting;
	QMenu *m_fileMenu;
	QMenu *m_deviceMenu;
	QStackedWidget *m_loggedInStack;
	QLabel *m_connectingLabel;
	bool m_loggedIn;
	QTimer m_connectingTimer;
	bool m_wasConnected;

	keePassImportController *m_keePassImportController;

	QMap<QString, int> m_exportFieldMap;
	QVector<QString> m_exportField;
	QMap<QString, exportType> m_exportData;

	enum SignetApplication::device_state m_deviceState;

	QWidget *m_backupWidget;
	QProgressBar *m_backupProgress;
	DatabaseImportController *m_dbImportController;
	int m_backupBlock;
	QFile *m_backupFile;
	enum SignetApplication::device_state m_backupPrevState;

	QWidget *m_restoreWidget;
	QProgressBar *m_restoreProgress;
	int m_restoreBlock;
	QFile *m_restoreFile;

	QWidget *m_firmwareUpdateWidget;
	QProgressBar *m_firmwareUpdateProgress;
	QList<fwSection>::iterator m_writingSectionIter;
	unsigned int m_writingAddr;
	unsigned int m_writingSize;
	unsigned int m_totalWritten;
	QList<fwSection> m_fwSections;
	QLabel *m_firmwareUpdateStage;
	QTimer m_resetTimer;

	QWidget *m_uninitPrompt;
	QMenu *m_exportMenu;
	QMenu *m_importMenu;

	QAction *m_openAction;
	QAction *m_closeAction;
	QAction *m_saveAction;
	QAction *m_settingsAction;
	QAction *m_exportCSVAction;
	QAction *m_importKeePassAction;
	QAction *m_importCSVAction;
	QAction *m_backupAction;
	QAction *m_restoreAction;
	QAction *m_passwordSlots;
	QAction *m_logoutAction;
	QAction *m_wipeDeviceAction;
	QAction *m_eraseDeviceAction;
	QAction *m_changePasswordAction;
	QAction *m_updateFirmwareAction;
	ButtonWaitDialog *m_buttonWaitDialog;
	QMessageBox *m_wipeDeviceDialog;
	int m_signetdevCmdToken;
	bool m_startedExport;
	KeyboardLayoutTester *m_keyboardLayoutTester;

#ifdef Q_OS_UNIX
	QAction *m_importPassAction;
	bool m_passDatabaseFound;
#endif

	void sendFirmwareWriteCmd();
	void loadSettings();
	void saveSettings();
	void settingsChanged();
	void autoBackupCheck();
	void showEvent(QShowEvent *event);
	void backupDevice(QString fileName);
	QString backupFileBaseName();
#ifdef _WIN32
	bool nativeEvent(const QByteArray &eventType, void *message, long *result);
#endif
	void startImport(DatabaseImporter *importer);
public:
	bool connected() const
	{
		return m_connected;
	}
private slots:
	void restoreError();
	void backupError();
	void importDone(bool success);
	void closeUi();
public slots:
	void signetDevEvent(int);
	void deviceOpened();
	void deviceClosed();
	void signetdevCmdResp(signetdevCmdRespInfo info);
	void signetdevGetProgressResp(signetdevCmdRespInfo info, signetdev_get_progress_resp_data data);
	void signetdevStartupResp(signetdevCmdRespInfo info, signetdev_startup_resp_data resp);
	void signetdevReadBlockResp(signetdevCmdRespInfo info, QByteArray block);
	void signetdevReadAllUIdsResp(signetdevCmdRespInfo info, int id, QByteArray data, QByteArray mask);
	void signetdevReadCleartextPasswordNames(signetdevCmdRespInfo info, QVector<int> formats, QStringList names);

	void wipeDeviceDialogFinished(int code);
	void connectionError();
	void messageReceived(QString);
	void open();
	void abort();
	void quit();
	void operationFinished(int);
	void enterDeviceState(int);

	void openUi();
	void logoutUi();
	void changePasswordUi();
	void eraseDeviceUi();
	void backupDeviceUi();
	void restoreDeviceUi();
	void updateFirmwareUi();
	void wipeDeviceUi();
	void resetTimer();
	void connectingTimer();
	void background();
	void openSettingsUi();
	void importKeePassUI();
	void passwordSlotsUi();
#ifdef Q_OS_UNIX
	void importPassUI();
#endif
	void importCSVUI();
	void exportCSVUi();
	void aboutUi();
	void startOnlineHelp();
	void applyKeyboardLayoutChanges();
	void keyboardLayoutTesterClosing(bool applyChanges);
};

#endif // MAINWINDOW_H
