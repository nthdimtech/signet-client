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
#include "localcache.h"
#include "localsettings.h"
#include "esdbgenericmodule.h"

#include <QVector>

#include "zip.h"

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
class ButtonWaitWidget;
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
class QFileDialog;
struct fwSection {
	QString name;
	unsigned int lma;
	int size;
	QByteArray contents;
};

struct exportType {
	QList<QVector<QString> > m_data;
	QMap<QString, int> m_exportFieldMap;
	QVector<QString> m_exportField;
};

class MainWindow : public QMainWindow
{
	Q_OBJECT
	QString m_dbFilename;
	QString m_backupDirectory;
	localSettings m_settings;
	localCache m_cache;
	esdbTypeModule *m_genericTypeModule;
	esdbTypeModule *m_accountTypeModule;
	esdbTypeModule *m_bookmarkTypeModule;
	esdbTypeModule *m_genericModule;
	LoggedInWidget *m_loggedInWidget;

	bool uninitializedFirmwareUpdateSupported();
public:
	explicit MainWindow(QString dbFilename, QWidget *parent = 0);
	void closeEvent(QCloseEvent *event);
	~MainWindow();

	static QString csvQuote(const QString &s);

	localSettings *getSettings()
	{
		return &m_settings;
	}

	QString getDatabaseFileName()
	{
		return m_dbFilename;
	}

	QStackedWidget *loggedInStack()
	{
		return m_loggedInStack;
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
	ButtonWaitWidget *m_buttonWaitWidget;
	bool m_loggedIn;
	QTimer m_connectingTimer;
	bool m_wasConnected;
	bool m_autoBackupCheckPerformed;

	int m_fwUpgradeState;
	struct hc_firmware_file_header *m_NewFirmwareHeader;
	struct hc_firmware_file_body *m_NewFirmwareBody;

	keePassImportController *m_keePassImportController;

	QMap<QString, exportType> m_exportData;

	enum SignetApplication::device_state m_deviceState;

	QWidget *m_backupWidget;
	QProgressBar *m_backupProgress;
	DatabaseImportController *m_dbImportController;
	int m_backupBlock;
	QFile *m_backupFile;
	zipFile m_backupZipFile;
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
	QMessageBox *m_wipeDeviceDialog;
	enum signetdev_device_type m_deviceType;
	int m_signetdevCmdToken;
	bool m_startedExport;
	bool m_backupRestoreSupported;
	KeyboardLayoutTester *m_keyboardLayoutTester;
	bool m_firmwareUpdate;
	bool m_factoryFirmwareUpdate;
	bool m_factoryFirmwareUpdateWiped;
	bool m_factoryFirmwareUpdateComplete;

	QLabel *m_uninitTestButtonCountWidget;
	int m_uninitTestButtonCount;

	void firmwareUpdateCompleteMessageBox();

#ifdef Q_OS_UNIX
	QAction *m_importPassAction;
	bool m_passDatabaseFound;
#endif

	void sendFirmwareWriteCmd();
	void loadPersistentSettings();
	void loadCachedSettings();
	void loadSettings();
	void savePersistentSettings();
	void saveCachedSettings();
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
	void endButtonWait();
	ButtonWaitWidget *beginButtonWait(QString action, bool longPress);
private slots:
	void settingDialogFinished(int rc);
	void restoreError();
	void backupError();
	void importDone(bool success);
	void closeUi();
	void keyboardLayoutNotConfiguredDialogFinished(int rc);
	void backupDatabasePromptDialogFinished(int rc);
	void openFileDialogFinished(int rc);
public slots:
	void signetDevEvent(int);
	void deviceOpened(enum signetdev_device_type dev_type);
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
	void buttonWaitTimeout();
	void buttonWaitCancel();
private:
	QFileDialog *m_openFileDialog;
	void setCentralStack(QWidget *w);
	void updateFirmwareHC(QByteArray &datum);
	void updateFirmware(QByteArray &datum);
	void firmwareFileInvalidMsg();
	void sendFirmwareWriteCmdHC();
	int firmwareSizeHC();
	void createFirmwareUpdateWidget();
	void updateFirmwareHCIter(bool buttonWait);
	void firmwareUpgradeCompletionCheck();
	QString backupFilter();
	QString backupSuffix();
	QString firmwareFilter();
	QString firmwareSuffix();
	bool uninitializedWipeSupported();
public slots:
	void abort();
	void quit();
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
