#include "mainwindow.h"
#include "localsettings.h"

#include <QVector>
#include <QMessageBox>
#include <QBoxLayout>
#include <QPushButton>
#include <QComboBox>
#include <QThread>
#include <QListView>
#include <QLineEdit>
#include <QAbstractButton>
#include <QPushButton>
#include <QLabel>
#include <QClipboard>
#include <QDir>
#include <QMenuBar>
#include <QFile>
#include <QFileDialog>
#include <QMenu>
#include <QRadioButton>
#include <QProgressBar>
#include <QStackedWidget>
#include <QJsonObject>
#include <QJsonDocument>
#include <QStandardPaths>
#include <QDateTime>
#include <QDesktopServices>
#include <QJsonArray>
#include <QString>
#include <QDesktopWidget>
#include <QTextStream>
#include <zlib.h>

#include "cleartextpasswordeditor.h"

#if (QT_VERSION >= QT_VERSION_CHECK(5, 4, 0))
#include <QStorageInfo>
#endif

#include <format/KeePass2Reader.h>
#include <keys/PasswordKey.h>
#include <core/Database.h>
#include <core/Group.h>
#include <core/Entry.h>

#include "about.h"
#include "loggedinwidget.h"
#include "account.h"
#include "esdbmodel.h"
#include "resetdevice.h"
#include "editaccount.h"
#include "buttonwaitdialog.h"
#include "buttonwaitwidget.h"
#include "loginwindow.h"
#include "keyboardlayouttester.h"

#include "signetapplication.h"
#include "settingsdialog.h"
#include "import/keepassunlockdialog.h"
#include "import/databaseimportcontroller.h"
#include "import/keepassimporter.h"

#ifdef Q_OS_UNIX
#include "import/passimporter.h"
#endif

#include "import/csvimporter.h"

extern "C" {
#include "signetdev/host/signetdev.h"
};

#include "changemasterpassword.h"
#include "esdbgenericmodule.h"
#include "esdbbookmarkmodule.h"
#include "esdbaccountmodule.h"
#include "generictypedesc.h"
#include "esdb/generictype/esdbgenerictypemodule.h"
#include "style.h"

#define SIGNET_BACKUP_EXTENSION "sdb"
#define SIGNET_HC_BACKUP_EXTENSION "sdbhc"
#define SIGNET_FIRMWARE_SUFFIX "sfw"
#define SIGNET_HC_FIRMWARE_SUFFIX "sfwhc"

bool MainWindow::uninitializedFirmwareUpdateSupported()
{
	int major, minor, step;
	SignetApplication::get()->getConnectedFirmwareVersion(major, minor, step);
	return (m_deviceType == SIGNETDEV_DEVICE_HC) && ((major > 0) || (minor > 1) || (step >=3));
}

bool MainWindow::uninitializedWipeSupported()
{
	int major, minor, step;
	SignetApplication::get()->getConnectedFirmwareVersion(major, minor, step);
	return (m_deviceType == SIGNETDEV_DEVICE_HC) && ((major > 0) || (minor > 1) || (step >=3));
}

MainWindow::MainWindow(QString dbFilename, QWidget *parent) :
	QMainWindow(parent),
	m_dbFilename(dbFilename),
	m_wipeProgress(nullptr),
	m_wipingWidget(nullptr),
	m_connected(false),
	m_quitting(false),
	m_fileMenu(nullptr),
	m_deviceMenu(nullptr),
	m_loggedInStack(nullptr),
	m_connectingLabel(nullptr),
	m_buttonWaitWidget(nullptr),
	m_loggedIn(false),
	m_wasConnected(false),
	m_autoBackupCheckPerformed(false),
	m_fwUpgradeState(0),
	m_NewFirmwareHeader(nullptr),
	m_NewFirmwareBody(nullptr),
	m_deviceState(SignetApplication::STATE_INVALID),
	m_backupWidget(nullptr),
	m_backupProgress(nullptr),
	m_backupFile(nullptr),
	m_backupPrevState(SignetApplication::STATE_INVALID),
	m_restoreWidget(nullptr),
	m_restoreBlock(0),
	m_restoreFile(nullptr),
	m_uninitPrompt(nullptr),
	m_backupAction(nullptr),
	m_restoreAction(nullptr),
	m_logoutAction(nullptr),
	m_wipeDeviceAction(nullptr),
	m_eraseDeviceAction(nullptr),
	m_changePasswordAction(nullptr),
	m_deviceType(SIGNETDEV_DEVICE_NONE),
	m_signetdevCmdToken(-1),
	m_startedExport(false),
	m_backupRestoreSupported(false)
{
	SignetApplication *app = SignetApplication::get();
	genericTypeDesc *g = new genericTypeDesc(-1);
	g->name = "generic";
	m_genericModule = new esdbGenericModule(g);
	m_genericTypeModule = new esdbGenericTypeModule();
	m_accountTypeModule = new esdbAccountModule();
	m_bookmarkTypeModule = new esdbBookmarkModule();

	if (dbFilename.size()) {
		int rc = signetdev_emulate_init(dbFilename.toLatin1().data());
		if (rc) {
			signetdev_emulate_begin();
			m_dbFilename = dbFilename;
		}
	}

	connect(app, SIGNAL(deviceOpened(enum signetdev_device_type)), this, SLOT(deviceOpened(enum signetdev_device_type)));
	connect(app, SIGNAL(deviceClosed()), this, SLOT(deviceClosed()));
	connect(app, SIGNAL(signetdevCmdResp(signetdevCmdRespInfo)),
		this, SLOT(signetdevCmdResp(signetdevCmdRespInfo)));
	connect(app, SIGNAL(signetdevGetProgressResp(signetdevCmdRespInfo, signetdev_get_progress_resp_data)),
		this, SLOT(signetdevGetProgressResp(signetdevCmdRespInfo, signetdev_get_progress_resp_data)));
	connect(app, SIGNAL(signetdevStartupResp(signetdevCmdRespInfo, signetdev_startup_resp_data)),
		this, SLOT(signetdevStartupResp(signetdevCmdRespInfo, signetdev_startup_resp_data)));
	connect(app, SIGNAL(signetdevReadBlockResp(signetdevCmdRespInfo, QByteArray)),
		this, SLOT(signetdevReadBlockResp(signetdevCmdRespInfo, QByteArray)));
	connect(app, SIGNAL(signetdevReadAllUIdsResp(signetdevCmdRespInfo, int, QByteArray, QByteArray)),
		this, SLOT(signetdevReadAllUIdsResp(signetdevCmdRespInfo, int, QByteArray, QByteArray)));
	connect(app, SIGNAL(signetdevReadCleartextPasswordNames(signetdevCmdRespInfo, QVector<int>, QStringList)),
		this, SLOT(signetdevReadCleartextPasswordNames(signetdevCmdRespInfo, QVector<int>, QStringList)));
	connect(app, SIGNAL(connectionError()),
		this, SLOT(connectionError()));

	QObject::connect(&m_resetTimer, SIGNAL(timeout()), this, SLOT(resetTimer()));

	QObject::connect(&m_connectingTimer, SIGNAL(timeout()), this, SLOT(connectingTimer()));
	QMenuBar *bar = new QMenuBar();
	setMenuBar(bar);
	m_fileMenu = bar->addMenu("&File");

	m_openAction = m_fileMenu->addAction("&Open");
	connect(m_openAction, SIGNAL(triggered(bool)), this, SLOT(openUi()));

	m_closeAction = m_fileMenu->addAction("&Close");
	connect(m_closeAction, SIGNAL(triggered(bool)), this, SLOT(closeUi()));
	m_closeAction->setDisabled(true);

	m_settingsAction = m_fileMenu->addAction("&Settings");

	m_exportMenu = m_fileMenu->addMenu("&Export");
	m_exportMenu->setVisible(false);
	m_exportCSVAction = m_exportMenu->addAction("&CSV");

	m_importMenu = m_fileMenu->addMenu("&Import");
	m_importMenu->setVisible(false);

	m_importKeePassAction = m_importMenu->addAction("&KeePass 2.x Database");
	connect(m_importKeePassAction, SIGNAL(triggered(bool)),
		this, SLOT(importKeePassUI()));

#ifdef Q_OS_UNIX
	m_importPassAction = m_importMenu->addAction("&Pass Database");
	connect(m_importPassAction, SIGNAL(triggered(bool)),
		this, SLOT(importPassUI()));
	QString gpgId = PassImporter::getGPGId();
	m_passDatabaseFound = (gpgId.size() != 0);
#endif
	m_importCSVAction = m_importMenu->addAction("&CSV");
	connect(m_importCSVAction, SIGNAL(triggered(bool)),
		this, SLOT(importCSVUI()));

	QAction *minimize_action = m_fileMenu->addAction("Minimize &window");
	minimize_action->setShortcut(Qt::CTRL | Qt::Key_W);
	QObject::connect(minimize_action, SIGNAL(triggered(bool)), this, SLOT(background()));

#ifdef Q_OS_UNIX
	QAction *quit_action = m_fileMenu->addAction("&Quit");
	quit_action->setShortcut(Qt::CTRL | Qt::Key_Q);
#else
	QAction *quit_action = m_fileMenu->addAction("Exit");
#endif
	QObject::connect(quit_action, SIGNAL(triggered(bool)), this, SLOT(quit()));

	connect(m_settingsAction, SIGNAL(triggered(bool)), this,
		SLOT(openSettingsUi()));

	connect(m_exportCSVAction, SIGNAL(triggered(bool)), this,
		SLOT(exportCSVUi()));

	if (!m_dbFilename.size()) {
		m_deviceMenu = bar->addMenu("&Device");
	} else {
		m_deviceMenu = bar->addMenu("&Database");
	}

	// Some of these don't get keyboard accelerators by design to avoid accidental activation

	m_logoutAction = m_deviceMenu->addAction("&Lock");
	QObject::connect(m_logoutAction, SIGNAL(triggered(bool)),
			 this, SLOT(logoutUi()));

	m_backupAction = m_deviceMenu->addAction("&Backup to file");
	QObject::connect(m_backupAction, SIGNAL(triggered(bool)),
			 this, SLOT(backupDeviceUi()));

	m_restoreAction = m_deviceMenu->addAction("Restore from file");
	QObject::connect(m_restoreAction, SIGNAL(triggered(bool)),
			 this, SLOT(restoreDeviceUi()));

	m_passwordSlots = m_deviceMenu->addAction("Password slots");
	QObject::connect(m_passwordSlots, SIGNAL(triggered(bool)),
			 this, SLOT(passwordSlotsUi()));

	m_changePasswordAction = m_deviceMenu->addAction("Change master password");
	QObject::connect(m_changePasswordAction, SIGNAL(triggered(bool)),
			 this, SLOT(changePasswordUi()));

	m_eraseDeviceAction = m_deviceMenu->addAction("Reset");
	QObject::connect(m_eraseDeviceAction, SIGNAL(triggered(bool)),
			 this, SLOT(eraseDeviceUi()));

	m_wipeDeviceAction = m_deviceMenu->addAction("Wipe");
	QObject::connect(m_wipeDeviceAction, SIGNAL(triggered(bool)),
			 this, SLOT(wipeDeviceUi()));

	m_updateFirmwareAction = m_deviceMenu->addAction("Update &firmware");
	QObject::connect(m_updateFirmwareAction, SIGNAL(triggered(bool)),
			 this, SLOT(updateFirmwareUi()));

	QMenu *helpMenu = bar->addMenu("&Help");

	QAction *onlineHelpAction = helpMenu->addAction("Online &help");
	connect(onlineHelpAction, SIGNAL(triggered(bool)), this, SLOT(startOnlineHelp()));

	QAction *about_action = helpMenu->addAction("&About");
	connect(about_action, SIGNAL(triggered(bool)), this, SLOT(aboutUi()));

	m_logoutAction->setVisible(true);
	m_logoutAction->setDisabled(true);
	m_eraseDeviceAction->setVisible(false);
	m_wipeDeviceAction->setVisible(false);
	m_changePasswordAction->setVisible(false);
	m_backupAction->setVisible(false);
	m_restoreAction->setVisible(false);
	m_passwordSlots->setVisible(false);
	if (m_dbFilename.size()) {
		enterDeviceState(SignetApplication::STATE_NEVER_SHOWN);
		enterDeviceState(SignetApplication::STATE_CONNECTING);
		::signetdev_startup(nullptr, &m_signetdevCmdToken);
	} else {
		enterDeviceState(SignetApplication::STATE_NEVER_SHOWN);
#ifdef Q_OS_UNIX
		enterDeviceState(SignetApplication::STATE_CONNECTING);
		m_deviceType = signetdev_open_connection();
		SignetApplication::get()->setDeviceType(m_deviceType);
		if (m_deviceType != SIGNETDEV_DEVICE_NONE) {
			::signetdev_startup(nullptr, &m_signetdevCmdToken);
			enterDeviceState(SignetApplication::STATE_STARTING_DEVICE);
		}
#endif
	}

	loadSettings();
	if (!m_settings.windowGeometry.size()) {
		QDesktopWidget* d = QApplication::desktop();
		QRect deskRect = d->screenGeometry(d->screenNumber(QCursor::pos()));
		adjustSize();
		move(deskRect.width() / 2 - width() / 2 + deskRect.left(),
		     deskRect.height() / 2 - height() / 2 + deskRect.top());
	} else {
		restoreGeometry(m_settings.windowGeometry);
	}

	if (m_settings.browserPluginSupport) {
		SignetApplication::get()->startWebsocketServer();
	}
}

QString MainWindow::csvQuote(const QString &s)
{
	if (s.contains(QChar('"'))) {
		QString sEsc;
		for (QChar c : s) {
			sEsc.append(c);
			if(c == '"') {
				sEsc.append('"');
			}
		}
		return '"' + sEsc + '"';
	} else {
		return '"' + s + '"';
	}
}

void MainWindow::startOnlineHelp()
{
	QUrl url;
	switch (m_deviceType) {
	case SIGNETDEV_DEVICE_HC:
		url.setUrl("https://nthdimtech.com/signet-hc");
		break;
	case SIGNETDEV_DEVICE_ORIGINAL:
	case SIGNETDEV_DEVICE_NONE:
		url.setUrl("https://nthdimtech.com/signet");
		break;
	}
	QDesktopServices::openUrl(url);
}

void MainWindow::connectionError()
{
	if (!m_resetTimer.isActive()) {
		m_wasConnected = true;
		enterDeviceState(SignetApplication::STATE_CONNECTING);
		m_deviceType = signetdev_open_connection();
		SignetApplication::get()->setDeviceType(m_deviceType);
		if (m_deviceType != SIGNETDEV_DEVICE_NONE) {
			::signetdev_startup(nullptr, &m_signetdevCmdToken);
			enterDeviceState(SignetApplication::STATE_STARTING_DEVICE);
		}
	}
}

#ifdef _WIN32
bool MainWindow::nativeEvent(const QByteArray & eventType, void *message, long *result)
{
	Q_UNUSED(result);
	if (eventType == QByteArray("windows_generic_MSG")) {
		MSG *msg = (MSG *) message;
		signetdev_filter_window_messasage(msg->message, msg->wParam, msg->lParam);
	}
	return false;
}
#endif

void MainWindow::deviceOpened(enum signetdev_device_type dev_type)
{
	m_deviceType = dev_type;
	SignetApplication::get()->setDeviceType(m_deviceType);
	::signetdev_startup(nullptr, &m_signetdevCmdToken);
	enterDeviceState(SignetApplication::STATE_STARTING_DEVICE);
}

void MainWindow::deviceClosed()
{
	m_deviceType = SIGNETDEV_DEVICE_NONE;
	SignetApplication::get()->setDeviceType(m_deviceType);
	emit connectionError();
}

void MainWindow::signetdevGetProgressResp(signetdevCmdRespInfo info, signetdev_get_progress_resp_data data)
{
	if (info.token != m_signetdevCmdToken) {
		return;
	}
	m_signetdevCmdToken = -1;
	int resp_code = info.resp_code;

	if (m_deviceState == SignetApplication::STATE_WIPING) {
		switch (resp_code) {
		case OKAY:
			m_wipeProgress->setRange(0, data.total_progress_maximum);
			m_wipeProgress->setValue(data.total_progress);
			m_wipeProgress->update();
			::signetdev_get_progress(nullptr, &m_signetdevCmdToken, data.total_progress, DS_WIPING);
			break;
		case INVALID_STATE:
			enterDeviceState(SignetApplication::STATE_UNINITIALIZED);
			break;
		case SIGNET_ERROR_DISCONNECT:
		case SIGNET_ERROR_QUIT:
			return;
		default:
			abort();
			return;
		}
	} else if (m_deviceState == SignetApplication::STATE_UPDATING_FIRMWARE) {
		switch (resp_code) {
		case OKAY:
			m_firmwareUpdateProgress->setRange(0, data.total_progress_maximum);
			m_firmwareUpdateProgress->setValue(data.total_progress);
			m_firmwareUpdateProgress->update();
			::signetdev_get_progress(nullptr, &m_signetdevCmdToken, data.total_progress, DS_ERASING_PAGES);
			break;
		case INVALID_STATE: {
			m_totalWritten = 0;
			QString updatingString;

			if (m_deviceType == SIGNETDEV_DEVICE_HC) {
				SignetApplication *app = SignetApplication::get();
				auto bootMode = app->getBootMode();

				switch (bootMode) {
				case HC_BOOT_BOOTLOADER_MODE:
					updatingString = "Writing second stage firmware...";
					break;
				case HC_BOOT_APPLICATION_MODE:
					updatingString = "Writing first stage firmware...";
					break;
				default:
					abort();
					break;
				}
				m_writingAddr = 0;
				int firmwareSize = firmwareSizeHC();
				m_firmwareUpdateProgress->setRange(0, firmwareSize);
				m_firmwareUpdateProgress->setValue(0);
				sendFirmwareWriteCmdHC();
			} else {
				int total_bytes = 0;

				for (auto a = m_fwSections.begin(); a != m_fwSections.end(); a++) {
					total_bytes += a->size;
				}
				m_firmwareUpdateProgress->setRange(0, total_bytes);
				m_firmwareUpdateProgress->setValue(0);

				m_writingSectionIter = m_fwSections.begin();
				m_writingAddr = m_writingSectionIter->lma;
				sendFirmwareWriteCmd();
				updatingString = "Writing firmware...";
			}
			m_firmwareUpdateStage->setText(updatingString);
		}
		break;
		case SIGNET_ERROR_DISCONNECT:
		case SIGNET_ERROR_QUIT:
			return;
		default:
			abort();
			return;
		}
	}
}

void MainWindow::signetdevReadBlockResp(signetdevCmdRespInfo info, QByteArray block)
{
	if (info.token != m_signetdevCmdToken) {
		return;
	}
	m_signetdevCmdToken = -1;

	int code = info.resp_code;
	switch (code) {
	case OKAY: {
		int rc;
		rc = m_backupFile->write(block);
		if (rc == -1) {
			QMessageBox * box = SignetApplication::messageBoxError(QMessageBox::Critical, "Backup database to file", "Failed to write to backup file", this);
			connect(box, SIGNAL(finished(int)), this, SLOT(backupError()));
			m_backupFile->close();
			m_backupFile->remove();
			delete m_backupFile;
			m_backupFile = nullptr;
			::signetdev_end_device_backup(nullptr, &m_signetdevCmdToken);
			return;
		}
		m_backupBlock++;
		m_backupProgress->setMinimum(0);
		m_backupProgress->setMaximum(::signetdev_device_num_storage_blocks()-1);
		m_backupProgress->setValue(m_backupBlock);
		if (m_backupBlock > (::signetdev_device_num_storage_blocks()-1)) {
			::signetdev_end_device_backup(nullptr, &m_signetdevCmdToken);
		} else {
			::signetdev_read_block(nullptr, &m_signetdevCmdToken, m_backupBlock);
		}
	}
	break;
	case BUTTON_PRESS_CANCELED:
	case BUTTON_PRESS_TIMEOUT:
	case SIGNET_ERROR_DISCONNECT:
	case SIGNET_ERROR_QUIT:
		m_backupFile->close();
		m_backupFile->remove();
		delete m_backupFile;
		m_backupFile = nullptr;
		break;
	default:
		m_backupFile->close();
		m_backupFile->remove();
		delete m_backupFile;
		m_backupFile = nullptr;
		abort();
		return;
	}
}

void MainWindow::signetdevCmdResp(signetdevCmdRespInfo info)
{
	if (info.token != m_signetdevCmdToken) {
		return;
	}
	m_signetdevCmdToken = -1;

	bool do_abort = false;
	int code = info.resp_code;

	switch (code) {
	case OKAY:
	case BUTTON_PRESS_CANCELED:
	case BUTTON_PRESS_TIMEOUT:
	case SIGNET_ERROR_DISCONNECT:
	case SIGNET_ERROR_QUIT:
	case DEVICE_NOT_WIPED:
		break;
	default:
		do_abort = true;
		return;
	}

	switch (info.cmd) {
	case SIGNETDEV_CMD_DISCONNECT:
		enterDeviceState(SignetApplication::STATE_DISCONNECTED);
		emit close();
		break;
	case SIGNETDEV_CMD_ERASE_PAGES:
		if (code == OKAY) {
			::signetdev_get_progress(nullptr, &m_signetdevCmdToken, 0, DS_ERASING_PAGES);
		}
		break;
	case SIGNETDEV_CMD_WRITE_FLASH:
		if (code == OKAY) {
			m_totalWritten += m_writingSize;
			m_firmwareUpdateProgress->setValue(m_totalWritten);
			m_firmwareUpdateProgress->update();
			if (m_deviceType == SIGNETDEV_DEVICE_HC) {
				if (m_totalWritten >= firmwareSizeHC()) {
					SignetApplication *app = SignetApplication::get();
					auto bootMode = app->getBootMode();
					bool needReset = false;
					switch (bootMode) {
					case HC_BOOT_BOOTLOADER_MODE:
						m_fwUpgradeState &= ~HC_UPGRADING_APPLICATION_MASK;
						m_fwUpgradeState |= HC_UPGRADED_APPLICATION_MASK;
						needReset = true;
						break;
					case HC_BOOT_APPLICATION_MODE:
						m_fwUpgradeState &= ~HC_UPGRADING_BOOTLOADER_MASK;
						m_fwUpgradeState |= HC_UPGRADED_BOOTLOADER_MASK;
						needReset = (m_fwUpgradeState & HC_UPGRADED_APPLICATION_MASK) == 0;
						break;
					default:
						//TODO
						break;
					}
					if (needReset) {
						::signetdev_switch_boot_mode(nullptr, &m_signetdevCmdToken);
					}
				} else {
					sendFirmwareWriteCmdHC();
				}
			} else {
				if (m_writingSectionIter == m_fwSections.end()) {
					::signetdev_reset_device(nullptr, &m_signetdevCmdToken);
				} else {
					sendFirmwareWriteCmd();
				}
			}
		} else {
			m_totalWritten++;
		}
		break;
	case SIGNETDEV_CMD_WRITE_BLOCK:
		if (code == OKAY) {
			m_restoreBlock++;
			m_restoreProgress->setMinimum(0);
			m_restoreProgress->setMaximum(::signetdev_device_num_storage_blocks());

			m_restoreProgress->setValue(m_restoreBlock);
			if (m_restoreBlock < ::signetdev_device_num_storage_blocks()) {
				QByteArray block(::signetdev_device_block_size(), 0);
				unsigned int sz = m_restoreFile->read(block.data(), block.length());
				if (sz == ::signetdev_device_block_size()) {
					::signetdev_write_block(nullptr, &m_signetdevCmdToken, m_restoreBlock, block.data());
				} else {
					QMessageBox *box = SignetApplication::messageBoxError(QMessageBox::Critical, "Restore device", "Failed to read from source file", this);
					connect(box, SIGNAL(finished(int)), this, SLOT(restoreError()));
				}
			} else {
				::signetdev_end_device_restore(nullptr, &m_signetdevCmdToken);
			}
		}
		break;
	case SIGNETDEV_CMD_BEGIN_DEVICE_BACKUP:
		endButtonWait();
		if (code == OKAY) {
			m_backupPrevState = m_deviceState;
			enterDeviceState(SignetApplication::STATE_BACKING_UP);
			m_backupBlock = 0;
			::signetdev_read_block(nullptr, &m_signetdevCmdToken, m_backupBlock);
		} else {
			do_abort = do_abort && (code != BUTTON_PRESS_CANCELED && code != BUTTON_PRESS_TIMEOUT);
			if (m_backupFile) {
				m_backupFile->close();
				m_backupFile->remove();
				delete m_backupFile;
				m_backupFile = nullptr;
			}
		}
		break;
	case SIGNETDEV_CMD_END_DEVICE_BACKUP:
		if (m_backupFile) {
			m_backupFile->close();
			delete m_backupFile;
			m_backupFile = nullptr;
		}
		if (code == OKAY) {
			enterDeviceState(m_backupPrevState);
		}
		break;
	case SIGNETDEV_CMD_BEGIN_DEVICE_RESTORE:
		endButtonWait();
		if (code == OKAY) {
			enterDeviceState(SignetApplication::STATE_RESTORING);
			m_restoreBlock = 0;
			m_restoreProgress->setMinimum(0);
			m_restoreProgress->setMaximum(signetdev_device_num_data_blocks());
			m_restoreProgress->setValue(m_restoreBlock);
			QByteArray block(::signetdev_device_block_size(), 0);
			qint64 sz = m_restoreFile->read(block.data(), block.length());
			if (sz == ::signetdev_device_block_size()) {
				::signetdev_write_block(nullptr, &m_signetdevCmdToken, m_restoreBlock, block.data());
			} else {
				QMessageBox *box = SignetApplication::messageBoxError(QMessageBox::Critical, "Restore device", "Failed to read from source file", this);
				connect(box, SIGNAL(finished(int)), this, SLOT(restoreError()));
			}
		} else {
			do_abort = do_abort && (code != BUTTON_PRESS_CANCELED && code != BUTTON_PRESS_TIMEOUT);
		}
		break;
	case SIGNETDEV_CMD_END_DEVICE_RESTORE:
		if (code == OKAY) {
			::signetdev_startup(nullptr, &m_signetdevCmdToken);
		}
		break;
	case SIGNETDEV_CMD_LOGOUT:
		if (code == OKAY) {
			enterDeviceState(SignetApplication::STATE_LOGGED_OUT);
		}
		break;
	case SIGNETDEV_CMD_WIPE:
		endButtonWait();
		if (code == OKAY) {
			enterDeviceState(SignetApplication::STATE_WIPING);
		}
		break;
	case SIGNETDEV_CMD_BEGIN_UPDATE_FIRMWARE: {
		endButtonWait();
		if (code == OKAY) {
			enterDeviceState(SignetApplication::STATE_UPDATING_FIRMWARE);
		} else if (code == DEVICE_NOT_WIPED && uninitializedFirmwareUpdateSupported()) {
			QMessageBox *box = SignetApplication::messageBoxError(QMessageBox::Critical, "Update firmware", "Device not wiped yet. Wipe device and try again", this);
			connect(box, SIGNAL(finished(int)), this, SLOT(restoreError()));
		}
	}
	break;
	case SIGNETDEV_CMD_RESET_DEVICE: {
		if (code == OKAY) {
			m_firmwareUpdateStage->setText("Resetting device...");
			m_firmwareUpdateProgress->hide();
			::signetdev_close_connection();
			m_resetTimer.setSingleShot(true);
			m_resetTimer.setInterval(500);
			m_resetTimer.start();
		}
	} break;
	case SIGNETDEV_CMD_SWITCH_BOOT_MODE: {
		if (code == OKAY) {
			m_firmwareUpdateStage->setText("Resetting device...");
			m_firmwareUpdateProgress->hide();
			::signetdev_close_connection();
			m_resetTimer.setSingleShot(true);
			m_resetTimer.setInterval(500);
			m_resetTimer.start();
		} else if (code == INVALID_STATE) {
			m_firmwareUpdateStage->setText("CRC validation failed");
			m_firmwareUpdateProgress->hide();
		} else if (code > 0){
			m_firmwareUpdateStage->setText("Firmare upgrade failed");
			m_firmwareUpdateProgress->hide();
		}
	}
	break;
	}
	if (do_abort) {
		abort();
	}
}

void MainWindow::restoreError()
{
	::signetdev_end_device_restore(nullptr, &m_signetdevCmdToken);
}

void MainWindow::backupError()
{
	::signetdev_end_device_backup(nullptr, &m_signetdevCmdToken);
}

#include "genericfields.h"
#include "generic.h"
#include <QVector>

void MainWindow::signetdevReadAllUIdsResp(signetdevCmdRespInfo info, int id, QByteArray data, QByteArray mask)
{
	if (info.token != m_signetdevCmdToken) {
		return;
	}

	if (info.resp_code != OKAY) {
		endButtonWait();
		zipClose(m_backupZipFile, NULL);
		m_backupZipFile = nullptr;
		enterDeviceState(SignetApplication::STATE_LOGGED_IN);
		return;
	}

	if (m_startedExport) {
		endButtonWait();
		enterDeviceState(SignetApplication::STATE_EXPORTING);
		m_backupProgress->setMaximum(info.messages_remaining);
		m_backupProgress->setMinimum(0);
		m_backupProgress->setValue(0);
		m_startedExport = false;
		m_exportData.clear();
	} else {
		m_backupProgress->setValue(m_backupProgress->maximum() - info.messages_remaining);
	}

	if (id >= MIN_UID && id <= MAX_UID) {
		block *blk = new block();
		blk->data = data;
		blk->mask = mask;
		esdbEntry_1 tmp(id);
		tmp.fromBlock(blk);

		esdbTypeModule *typeModule = nullptr;
		QString typeName;

		switch (tmp.type) {
		case ESDB_TYPE_ACCOUNT:
			typeModule = m_accountTypeModule;
			break;
		case ESDB_TYPE_BOOKMARK:
			typeModule = m_bookmarkTypeModule;
			break;
		case ESDB_TYPE_GENERIC:
			typeModule = m_genericModule;
			break;
		case ESDB_TYPE_GENERIC_TYPE_DESC:
			typeModule = m_genericTypeModule;
			break;
		}
		if (typeModule != nullptr) {
			esdbEntry *entry = typeModule->decodeEntry(id, tmp.revision, nullptr, blk);
			if (entry) {
				esdbTypeModule *entryTypeModule = m_loggedInWidget->esdbEntryToModule(entry);
				QString moduleName = entryTypeModule->name();
				exportType &exportType = m_exportData[moduleName];
				QVector<genericField> fields;
				entry->getFields(fields);
				for (genericField field : fields) {
					auto iter = exportType.m_exportFieldMap.find(field.name);
					if (iter == exportType.m_exportFieldMap.end()) {
						exportType.m_exportField.push_back(field.name);
						exportType.m_exportFieldMap[field.name] = exportType.m_exportField.size() - 1;
					}
				}
				exportType.m_data.push_back(QVector<QString>());
				QVector<QString> &csvEntry = exportType.m_data.back();
				csvEntry.resize(exportType.m_exportField.size() + 1);
				for (genericField field : fields) {
					int index = exportType.m_exportFieldMap[field.name];
					csvEntry[index] = field.value;
				}
			}
		}
	}
	if (!info.messages_remaining) {
		QMap<QString, exportType>::iterator exportType;
		for (auto x = m_exportData.begin(); x != m_exportData.end(); x++) {
			QDateTime current = QDateTime::currentDateTime();
			zip_fileinfo zfi = {};
			zfi.tmz_date.tm_year = current.date().year();
			zfi.tmz_date.tm_mon = current.date().month() - 1;
			zfi.tmz_date.tm_mday = current.date().day();
			zfi.tmz_date.tm_hour = current.time().hour();
			zfi.tmz_date.tm_min = current.time().minute();
			zfi.tmz_date.tm_sec = current.time().second();
			zipOpenNewFileInZip(m_backupZipFile,
				(x.key() + ".csv").toLatin1().data(),
				&zfi,
				NULL, 0, NULL, 0, NULL,
				Z_DEFLATED,
				Z_DEFAULT_COMPRESSION);

			auto exportType = x.value();
			QString outStr;
			QTextStream out(&outStr);
			for (auto x : exportType.m_exportField) {
				out << csvQuote(x) << ",";
			}
			out << endl;
			for (auto entry : exportType.m_data) {
				for (auto value : entry) {
					out << csvQuote(value) << ",";
				}
				out << endl;
			}
			out.flush();
			QByteArray outUTF8 = outStr.toUtf8();
			zipWriteInFileInZip(m_backupZipFile, outUTF8.data() , outUTF8.size());
			zipCloseFileInZip(m_backupZipFile);

		}
		zipClose(m_backupZipFile, NULL);
		m_backupZipFile = nullptr;
		QMessageBox *box = new QMessageBox(QMessageBox::Information,
						   "Export database to CSV",
						   "Export successful",
						   QMessageBox::Ok,
						   this);
		box->setWindowModality(Qt::WindowModal);
		box->show();
		enterDeviceState(SignetApplication::STATE_LOGGED_IN);
	}
}

void MainWindow::firmwareUpgradeCompletionCheck()
{
	SignetApplication *app = SignetApplication::get();
	if (m_fwUpgradeState & HC_UPGRADED_APPLICATION_MASK) {
		auto box = app->messageBoxError(QMessageBox::Information, "Firmware upgraded",
										"The firmware has been upgraded to version " + QString::number(m_NewFirmwareHeader->fw_version.major)
										+ "." + QString::number(m_NewFirmwareHeader->fw_version.minor)
										+ "." + QString::number(m_NewFirmwareHeader->fw_version.step),
										this);
		m_fwUpgradeState = 0;
		delete m_NewFirmwareBody;
		delete m_NewFirmwareHeader;
		m_NewFirmwareBody = nullptr;
		m_NewFirmwareHeader = nullptr;
		box->exec();
	}
}

void MainWindow::signetdevStartupResp(signetdevCmdRespInfo info, signetdev_startup_resp_data resp)
{
	if (info.token != m_signetdevCmdToken) {
		return;
	}
	m_signetdevCmdToken = -1;
	int code = info.resp_code;
	SignetApplication *app = SignetApplication::get();
	QByteArray salt;
	int keyLength;
	int saltLength;
	int root_block_format = resp.root_block_format;
	int device_state = resp.device_state;
	int db_format = resp.db_format;
	if (root_block_format == 1) {
		saltLength = AES_128_KEY_SIZE;
		keyLength = AES_128_KEY_SIZE;
	} else {
		saltLength = AES_256_KEY_SIZE;
		keyLength = AES_256_KEY_SIZE;
	}
	salt = QByteArray((const char *)resp.salt, saltLength);
	app->setBootMode(resp.boot_mode);
	app->setSaltLength(saltLength);
	app->setSalt(salt);
	app->setHashfn(QByteArray((const char *)resp.hashfn, HASH_FN_SZ));
	app->setKeyLength(keyLength);
	app->setDBFormat(db_format);
	app->setConnectedFirmwareVersion(resp.fw_major_version, resp.fw_minor_version, resp.fw_step_version);


	switch (m_deviceType) {
	case SIGNETDEV_DEVICE_HC:
		if (resp.fw_major_version > 0) {
			m_backupRestoreSupported = true;
		} else if (resp.fw_major_version == 0 && resp.fw_minor_version >= 2) {
			m_backupRestoreSupported = true;
		} else {
			m_backupRestoreSupported = false;
		}
		break;
	case SIGNETDEV_DEVICE_ORIGINAL:
		m_backupRestoreSupported = true;
		break;
	default:
		break;
	}

	if (m_restoreFile) {
		m_restoreFile->close();
		delete m_restoreFile;
		m_restoreFile = nullptr;
	}

	switch (code) {
	case UNKNOWN_DB_FORMAT:
		enterDeviceState(SignetApplication::STATE_UNINITIALIZED);
		firmwareUpgradeCompletionCheck();
		break;
	case OKAY:
		switch (device_state) {
		case DS_LOGGED_OUT:
			enterDeviceState(SignetApplication::STATE_LOGGED_OUT);
			firmwareUpgradeCompletionCheck();
			break;
		case DS_UNINITIALIZED:
			enterDeviceState(SignetApplication::STATE_UNINITIALIZED);
			firmwareUpgradeCompletionCheck();
			break;
		case DS_BOOTLOADER:
			if (m_NewFirmwareBody && m_NewFirmwareHeader) {
				updateFirmwareHCIter(false);
			} else {
				enterDeviceState(SignetApplication::STATE_BOOTLOADER);
			}
			break;
		}
		break;
	case BUTTON_PRESS_CANCELED:
	case BUTTON_PRESS_TIMEOUT:
	case SIGNET_ERROR_DISCONNECT:
	case SIGNET_ERROR_QUIT:
		return;
	default:
		abort();
		return;
	}
}

void MainWindow::showEvent(QShowEvent *event)
{
#ifdef _WIN32
	if (!event->spontaneous()) {
		if (m_deviceState == SignetApplication::STATE_NEVER_SHOWN) {
			enterDeviceState(SignetApplication::STATE_CONNECTING);
			signetdev_win32_set_window_handle((HANDLE)winId());
			m_deviceType = signetdev_open_connection();
			SignetApplication::get()->setDeviceType(m_deviceType);
			if (m_deviceType != SIGNETDEV_DEVICE_NONE) {
				deviceOpened(m_deviceType);
			}
		}
	}
#else
	Q_UNUSED(event);
#endif
}

void MainWindow::messageReceived(QString message)
{
	if (message == QString("open")) {
		open();
	} else if (message == QString("close")) {
		quit();
	}
}

void MainWindow::quit()
{
	m_quitting = true;
	emit close();
}

void MainWindow::changePasswordUi()
{
	ChangeMasterPassword *cmp = new ChangeMasterPassword(this);
	connect(cmp, SIGNAL(abort()), this, SLOT(abort()));
	connect(cmp, SIGNAL(finished(int)), cmp, SLOT(deleteLater()));
	cmp->show();
}

void MainWindow::closeEvent(QCloseEvent *event)
{

#ifndef Q_OS_MACOS
	if (m_quitting) {
#endif
		switch (m_deviceState) {
		case SignetApplication::STATE_CONNECTING:
			enterDeviceState(SignetApplication::STATE_DISCONNECTED);
			break;
		case SignetApplication::STATE_DISCONNECTED:
			break;
		default:
			event->ignore();
			::signetdev_disconnect(nullptr, &m_signetdevCmdToken);
			return;
		}
		m_settings.windowGeometry = saveGeometry();
		saveSettings();
		signetdev_close_connection();
		SignetApplication::get()->quit();
		event->accept();
#ifndef Q_OS_MACOS
	} else {
		emit hide();
		event->ignore();
	}
#endif
}

MainWindow::~MainWindow()
{
	delete m_genericTypeModule;
	delete m_accountTypeModule;
	delete m_bookmarkTypeModule;
}

void MainWindow::logoutUi()
{
	if (m_deviceState == SignetApplication::STATE_LOGGED_IN) {
		::signetdev_logout(nullptr, &m_signetdevCmdToken);
	}
}

void MainWindow::open()
{
	show();
	setWindowState((windowState() & ~Qt::WindowMinimized) | Qt::WindowActive);
	raise();
	activateWindow();
}

void MainWindow::buttonWaitTimeout()
{
	endButtonWait();
}

void MainWindow::buttonWaitCancel()
{
	::signetdev_cancel_button_wait();
	endButtonWait();
}

void MainWindow::signetDevEvent(int code)
{
	switch (code) {
	case 1:
		open();
		break;
	}
}

void MainWindow::background()
{
	showMinimized();
	if (getSettings()->minimizeToTray) {
		hide();
	}
}

extern "C" {
#include "signetdev/host/signetdev.h"
}

void MainWindow::saveSettings()
{
	QString configFileName = QStandardPaths::writableLocation(QStandardPaths::GenericConfigLocation) +
				 "/signet/config.json";
	QFile configFile(configFileName);
	if (!configFile.open(QFile::WriteOnly)) {
		auto box = SignetApplication::get()->messageBoxError(QMessageBox::Warning, "Couldn't write settings",
									"Failed to write settings file at " + configFileName,
									this);
		box->exec();
		return;
	}
	QJsonDocument doc;
	QJsonObject obj;
	obj.insert("browserPluginSupport", QJsonValue(m_settings.browserPluginSupport));
	obj.insert("localBackups", QJsonValue(m_settings.localBackups));
	obj.insert("localBackupPath", QJsonValue(m_settings.localBackupPath));
	obj.insert("localBackupInterval", QJsonValue(m_settings.localBackupInterval));
	obj.insert("removableBackups", QJsonValue(m_settings.removableBackups));
	obj.insert("removableBackupPath", QJsonValue(m_settings.removableBackupPath));
	obj.insert("removableBackupVolume", QJsonValue(m_settings.removableBackupVolume));
	obj.insert("removableBackupInterval", QJsonValue(m_settings.removableBackupInterval));
	obj.insert("lastRemoveableBackup", QJsonValue(m_settings.lastRemoveableBackup.toString()));
	obj.insert("lastUpdatePrompt", QJsonValue(m_settings.lastUpdatePrompt.toString()));
	obj.insert("activeKeyboardLayout", QJsonValue(m_settings.activeKeyboardLayout));
#ifndef Q_OS_MACOS
	obj.insert("minimizeToTray", QJsonValue(m_settings.minimizeToTray));
#endif
	obj.insert("windowGeometry", QJsonValue(QLatin1String(m_settings.windowGeometry.toBase64())));

	QJsonObject keyboardLayouts;
	for (QMap<QString, keyboardLayout>::iterator v = m_settings.keyboardLayouts.begin();
	     v != m_settings.keyboardLayouts.end();
	     v++) {
		QString name = v.key();
		QJsonArray layout;
		for (signetdev_key key : (*v)) {
			QJsonArray jKey;
			jKey.append(QJsonValue(QString(QChar(key.key))));
			jKey.append(QJsonValue(key.phy_key[0].scancode));
			jKey.append(QJsonValue(key.phy_key[0].modifier));
			jKey.append(QJsonValue(key.phy_key[1].scancode));
			jKey.append(QJsonValue(key.phy_key[1].modifier));
			layout.append(jKey);
		}
		keyboardLayouts.insert(v.key(), layout);
	}
	obj.insert("keyboardLayouts", keyboardLayouts);

	doc.setObject(obj);
	QByteArray datum = doc.toJson();
	configFile.write(datum);
	configFile.close();
}

QString MainWindow::backupFileBaseName()
{
	QDateTime currentTime = QDateTime::currentDateTime();
	return "signet-hc-" +
	       QString::number(currentTime.date().year()) + "-" +
	       QString("%1").arg(QString::number(currentTime.date().month()), 2, '0') + "-" +
	       QString("%1").arg(QString::number(currentTime.date().day()), 2, '0');
}

QString MainWindow::backupSuffix()
{
	if (m_deviceType == SIGNETDEV_DEVICE_HC) {
		return QString(SIGNET_HC_BACKUP_EXTENSION);
	} else {
		return QString(SIGNET_BACKUP_EXTENSION);
	}
}

QString MainWindow::backupFilter()
{
	return "*." + backupSuffix();
}

void MainWindow::autoBackupCheck()
{
	QDateTime currentTime = QDateTime::currentDateTime();
	if (m_settings.localBackups) {
		QString backupFileName = m_settings.localBackupPath + "/" + backupFileBaseName() + "." + backupSuffix();
		QDir backupPath(m_settings.localBackupPath);
		if (!backupPath.exists()) {
			QString dirName = backupPath.dirName();
			if (backupPath.cdUp()) {
				backupPath.mkdir(dirName);
				backupPath.setPath(m_settings.localBackupPath);
			}
		}
		if (backupPath.exists()) {
			QStringList nameFilters;
			nameFilters.push_back(backupFilter());
			QFileInfoList files = backupPath.entryInfoList(nameFilters, QDir::Files, QDir::Time);
			bool needToCreate = false;
			if (files.size()) {
				needToCreate = true;
				for (auto f : files) {
					QDateTime lastModified = f.lastModified();
					qint64 delta = lastModified.daysTo(currentTime);
					if (delta < m_settings.localBackupInterval) {
						needToCreate = false;
					}
				}
			} else {
				needToCreate = true;
			}
			if (needToCreate) {
				QMessageBox *box = new QMessageBox(QMessageBox::Warning,
								   "Backup database",
								   "No local database backups have been made in the last " +
								   QString::number(m_settings.localBackupInterval) +
								   " days. Create a new backup?",
								   QMessageBox::Yes | QMessageBox::No,
								   this);
				int rc = box->exec();
				box->deleteLater();
				if (rc == QMessageBox::Yes) {
					backupDeviceUi();
					return;
				}
			}
		}
	}
#if (QT_VERSION >= QT_VERSION_CHECK(5, 4, 0))
	if (m_settings.removableBackups) {
		if (!m_settings.lastRemoveableBackup.isValid() ||
		    m_settings.lastRemoveableBackup.daysTo(currentTime) > m_settings.removableBackupInterval) {
			QMessageBox *box = new QMessageBox(QMessageBox::Warning,
							   "Backup database",
							   "No database backups have been made to removable volume " +
							   m_settings.removableBackupVolume +
							   " in the last " +
							   QString::number(m_settings.removableBackupInterval) +
							   " days. Create a new backup?",
							   QMessageBox::Yes | QMessageBox::No,
							   this);
			connect(box, SIGNAL(finished(int)), this, SLOT(backupDatabasePromptDialogFinished(int)));
			box->setWindowModality(Qt::WindowModal);
			box->setAttribute(Qt::WA_DeleteOnClose);
			box->show();
		}
	}
#endif
}

void MainWindow::backupDatabasePromptDialogFinished(int rc)
{
#if (QT_VERSION >= QT_VERSION_CHECK(5, 4, 0))
	QDateTime currentTime = QDateTime::currentDateTime();
	if (rc == QMessageBox::Yes) {
		QMessageBox *box = new QMessageBox(QMessageBox::Information,
						   "Load volume",
						   "Insert and open volume " + m_settings.removableBackupVolume +
						   " then click 'Ok' to start backup",
						   QMessageBox::Ok | QMessageBox::Cancel,
						   this);
		int rc = box->exec();
		box->deleteLater();
		QStorageInfo storageInfo;
		bool volumeFound = false;
		if (rc == QMessageBox::Ok) {
			QList<QStorageInfo> volumes = QStorageInfo::mountedVolumes();
			for (auto x : volumes) {
				if (x.name() != m_settings.removableBackupVolume)
					continue;
				storageInfo = x;
				volumeFound = true;
				break;
			}
			if (volumeFound) {
				QString backupPath = storageInfo.rootPath() + "/" +
							 m_settings.removableBackupPath;
				QString backupFileName = backupPath + "/" + backupFileBaseName() + "." + backupSuffix();
				QDir backupPathDir(backupPath);
				if (!backupPathDir.exists()) {
					QString dirName = backupPathDir.dirName();
					if (backupPathDir.cdUp()) {
						backupPathDir.mkdir(dirName);
						backupPathDir.setPath(m_settings.localBackupPath);
					}
				}
				if (backupPathDir.exists()) {
					backupDevice(backupFileName);
					m_settings.lastRemoveableBackup = currentTime;
					saveSettings();
				} else {
					QMessageBox *box = new QMessageBox(QMessageBox::Warning,
									   "Backup database",
									   "Failed to create backup path",
									   QMessageBox::Ok,
									   this);
					box->setWindowModality(Qt::WindowModal);
					box->setAttribute(Qt::WA_DeleteOnClose);
					box->show();
				}
			} else {
				QMessageBox *box = new QMessageBox(QMessageBox::Warning,
								   "Backup database",
								   "No open volume named " + m_settings.removableBackupVolume +
								   " found",
								   QMessageBox::Ok,
								   this);
				box->setWindowModality(Qt::WindowModal);
				box->setAttribute(Qt::WA_DeleteOnClose);
				box->show();
			}
		}
	}
#else
	Q_UNUSED(rc);
#endif
}

void MainWindow::settingsChanged()
{
	if (m_settings.browserPluginSupport) {
		SignetApplication::get()->startWebsocketServer();
	} else {
		SignetApplication::get()->stopWebsocketServer();
	}
}

void MainWindow::loadSettings()
{
	QString genericConfigPath = QStandardPaths::writableLocation(QStandardPaths::GenericConfigLocation);
	QString appConfigPath = genericConfigPath + "/signet";

	QDir configDir(genericConfigPath);
	QDir appConfigDir(appConfigPath);
	if (!appConfigDir.exists()) {
		configDir.mkdir("signet");
	}

	QString configFileName = appConfigPath + "/config.json";

	QFile configFile(configFileName);
	QJsonDocument doc;
	if (configFile.exists()) {
		if (!configFile.open(QFile::ReadWrite)) {
			//TODO
		} else {
			QByteArray datum = configFile.readAll();
			doc = QJsonDocument::fromJson(datum);
			configFile.close();
		}
	}
	QJsonObject obj = doc.object();

	QJsonValue browserPluginSupport = obj.value("browserPluginSupport");
	if (browserPluginSupport.isBool()) {
		m_settings.browserPluginSupport = browserPluginSupport.toBool();
	} else {
		m_settings.browserPluginSupport = true;
	}

	QJsonValue localBackups = obj.value("localBackups");
	if (localBackups.isBool()) {
		m_settings.localBackups = localBackups.toBool();
	} else {
		m_settings.localBackups = false;
	}
	QJsonValue localBackupPath = obj.value("localBackupPath");
	if (localBackupPath.isString()) {
		m_settings.localBackupPath = localBackupPath.toString();
	} else {
		m_settings.localBackupPath = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation) + "/SignetBackups";
	}
	QJsonValue localBackupInterval = obj.value("localBackupInterval");
	if (localBackupInterval.isDouble()) {
		m_settings.localBackupInterval = localBackupInterval.toInt();
	} else {
		m_settings.localBackupInterval = 7;
	}

	QJsonValue removableBackups = obj.value("removableBackups");
	if (removableBackups.isBool()) {
		m_settings.removableBackups = removableBackups.toBool();
	} else {
		m_settings.removableBackups = false;
	}
	QJsonValue removableBackupPath = obj.value("removableBackupPath");
	if (removableBackupPath.isString()) {
		m_settings.removableBackupPath = removableBackupPath.toString();
	} else {
		m_settings.removableBackupPath = "SignetBackups";
	}
	QJsonValue removableBackupVolume = obj.value("removableBackupVolume");
	if (removableBackupVolume.isString()) {
		m_settings.removableBackupVolume = removableBackupVolume.toString();
	} else {
		m_settings.removableBackupVolume = "SIGNET";
	}
	QJsonValue removableBackupInterval = obj.value("removableBackupInterval");
	if (removableBackupInterval.isDouble()) {
		m_settings.removableBackupInterval = removableBackupInterval.toInt();
	} else {
		m_settings.removableBackupInterval = 30;
	}
	QJsonValue lastRemoveableBackup = obj.value("lastRemoveableBackup");
	if (lastRemoveableBackup.isString()) {
		m_settings.lastRemoveableBackup = QDateTime::fromString(lastRemoveableBackup.toString());
	} else {
		m_settings.lastRemoveableBackup = QDateTime();
	}

	QJsonValue lastUpdatePrompt = obj.value("lastUpdatePrompt");
	if (lastUpdatePrompt.isString()) {
		m_settings.lastUpdatePrompt = QDateTime::fromString(lastUpdatePrompt.toString());
	} else {
		m_settings.lastUpdatePrompt = QDateTime();
	}

#ifdef Q_OS_MACOS
	m_settings.minimizeToTray = false;
#else
	QJsonValue minimizeToTray = obj.value("minimizeToTray");
	if (minimizeToTray.isBool()) {
		m_settings.minimizeToTray = minimizeToTray.toBool();
	} else {
		m_settings.minimizeToTray = false;
	}
#endif

	QJsonValue activeKeyboardLayout = obj.value("activeKeyboardLayout");
	if (activeKeyboardLayout.isString()) {
		m_settings.activeKeyboardLayout = activeKeyboardLayout.toString();
	} else {
		m_settings.activeKeyboardLayout = "";
	}

	QJsonValue windowGeometry = obj.value("windowGeometry");
	if (windowGeometry.isString()) {
		m_settings.windowGeometry = QByteArray::fromBase64(windowGeometry.toString().toLatin1());
	}

	QJsonValue keyboardLayoutsV = obj.value("keyboardLayouts");
	if (keyboardLayoutsV.isObject()) {
		QJsonObject keyboardLayouts = keyboardLayoutsV.toObject();
		for (QJsonObject::iterator layoutIter = keyboardLayouts.begin();
			 layoutIter != keyboardLayouts.end();
			 layoutIter++) {

			if (!(*layoutIter).isArray())
				continue;
			keyboardLayout layout;

			QJsonArray layoutA = (*layoutIter).toArray();
			QString name = layoutIter.key();
			for (auto entryV : layoutA) {
				if (!entryV.isArray())
					continue;

				QJsonArray entry = entryV.toArray();
				if (entry.size() != 3 && entry.size() != 5)
					continue;
				if (!entry.first().isString())
					continue;

				QString keyS = entry.first().toString();
				if (keyS.size() != 1)
					continue;
				signetdev_key key;
				key.key = keyS.at(0).unicode();

				int codes[4];
				bool numeric = true;
				for (int i = 1; i < entry.size(); i++) {
					if (!entry.at(i).isDouble()) {
						numeric = false;
					}
					codes[i - 1] = entry.at(i).toInt();
				}
				if (!numeric)
					continue;
				if (entry.size() == 3) {
					key.phy_key[0].scancode = codes[0];
					key.phy_key[0].modifier = codes[1];
				} else if (entry.size() == 5) {
					key.phy_key[0].scancode = codes[0];
					key.phy_key[0].modifier = codes[1];
					key.phy_key[1].scancode = codes[2];
					key.phy_key[1].modifier = codes[3];
				}
				layout.append(key);
				m_settings.keyboardLayouts.insert(name, layout);
			}
		}
	} else {
		m_settings.keyboardLayouts.clear();
	}

	auto iter = m_settings.keyboardLayouts.find(m_settings.activeKeyboardLayout);
	if (iter != m_settings.keyboardLayouts.end()) {
		auto keyboardLayout = *iter;
		::signetdev_set_keymap(keyboardLayout.data(), keyboardLayout.size());
	}

	SignetApplication *app = SignetApplication::get();

	QDateTime current = QDateTime::currentDateTime();
	QDateTime release = QDateTime(app->getReleaseDate());
	QDateTime prompt = m_settings.lastUpdatePrompt;

	if ((release.daysTo(current) > app->releasePeriod()) &&
		(!prompt.isValid() ||
		 (prompt.daysTo(current) > 30))) {
		QMessageBox *box = new QMessageBox(QMessageBox::Information, "Client update check",
						   "This client is more than " + QString::number(app->releasePeriod()) + " days old. Check for a new version?",
						   QMessageBox::No | QMessageBox::Yes,
						   this);
		int rc = box->exec();
		m_settings.lastUpdatePrompt = current;
		saveSettings();
		box->deleteLater();
		if (rc == QMessageBox::Yes) {
			QUrl url("https://nthdimtech.com/signet/downloads");
			QDesktopServices::openUrl(url);
		}
	}

	if (!configFile.exists()) {
		QMessageBox *box = new QMessageBox(QMessageBox::Information, "Machine not configured",
						   "Would you like to configure Signet for this system now?\n\n"
						   "If you don't configure your system backups will not be enabled and Signet "
						   "will use a standard English(US) keyboard which may prevent Signet from typing data "
						   "correctly if you use a different keyboard layout.\n\n"
						   "You can configure Sigent later from "
#ifdef Q_OS_MACOS
						   "'Preferences'",
#else
						   "the 'File->Settings' menu option",
#endif
						   QMessageBox::No | QMessageBox::Yes,
						   this);
		int rc = box->exec();
		box->deleteLater();
		if (rc == QMessageBox::Yes) {
			SettingsDialog *config = new SettingsDialog(this, true);
			config->exec();
			config->deleteLater();
		}
		saveSettings();
	}

	settingsChanged();
	QLocale inputLocale = QApplication::inputMethod()->locale();
	if ((inputLocale.language() != QLocale::English ||
		 inputLocale.country() != QLocale::UnitedStates)
		&& inputLocale != QLocale::c() &&
		!m_settings.keyboardLayouts.size()) {
		QMessageBox *box = new QMessageBox(QMessageBox::Information, "Keyboard layout not configured",
						   "Configure your keyboard layout?\n\n"
						   "By default Signet uses a standard English(US) keyboard layout but your "
						   "input locale is not English(US). If you don't conifigure your keyboard layout Signet "
						   "may not function correctly.",
						   QMessageBox::No | QMessageBox::Yes,
						   this);
		connect(box, SIGNAL(finished(int)), this, SLOT(keyboardLayoutNotConfiguredDialogFinished(int)));
		box->setWindowModality(Qt::WindowModal);
		box->setAttribute(Qt::WA_DeleteOnClose);
		box->show();
	}
}

void MainWindow::keyboardLayoutNotConfiguredDialogFinished(int rc)
{
	if (rc == QMessageBox::Yes) {
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
}

void MainWindow::applyKeyboardLayoutChanges()
{
	auto keyboardLayout = m_keyboardLayoutTester->getLayout();
	::signetdev_set_keymap(keyboardLayout.data(), keyboardLayout.size());
	m_settings.activeKeyboardLayout = QString("current");
	m_settings.keyboardLayouts.insert("current", keyboardLayout);
	saveSettings();
	settingsChanged();
}

void MainWindow::keyboardLayoutTesterClosing(bool applyChanges)
{
	if (applyChanges) {
		applyKeyboardLayoutChanges();
	}
	m_keyboardLayoutTester->deleteLater();
}

void MainWindow::setCentralStack(QWidget *w)
{
	QStackedWidget *stack = new QStackedWidget();
	stack->addWidget(w);
	setCentralWidget(stack);
}

void MainWindow::createFirmwareUpdateWidget()
{
	m_firmwareUpdateWidget = new QWidget();
	QBoxLayout *layout = new QBoxLayout(QBoxLayout::TopToBottom);
	layout->setAlignment(Qt::AlignTop);
	m_firmwareUpdateProgress = new QProgressBar();
	if (m_deviceType == SIGNETDEV_DEVICE_HC) {
		SignetApplication *app =  SignetApplication::get();
		QString progressTitle;
		auto btMode = app->getBootMode();
		switch (btMode) {
		case HC_BOOT_BOOTLOADER_MODE:
			progressTitle = QString("Preparing to write second stage firmware...");
			break;
		case HC_BOOT_APPLICATION_MODE:
			progressTitle = QString("Preparing to write first stage firmware...");
			break;
		default:
			//TODO
			break;
		}
		m_firmwareUpdateStage = new genericText(progressTitle);
	} else {
		m_firmwareUpdateStage = new processingText("Erasing firmware pages...");
	}
	layout->addWidget(m_firmwareUpdateStage);
	layout->addWidget(m_firmwareUpdateProgress);
	m_firmwareUpdateWidget->setLayout(layout);
}

void MainWindow::enterDeviceState(int state)
{
	bool databaseFile = m_dbFilename.size();
	if (databaseFile) {
		m_changePasswordAction->setVisible(false);
		m_backupAction->setVisible(false);
		m_restoreAction->setVisible(false);
		m_updateFirmwareAction->setVisible(false);
		m_eraseDeviceAction->setVisible(false);
		m_wipeDeviceAction->setVisible(false);
		m_passwordSlots->setVisible(false);
		m_importMenu->setDisabled(true);
		m_deviceMenu->setTitle("&Database");
	} else {
		m_deviceMenu->setTitle("&Device");
	}
	m_closeAction->setDisabled(!databaseFile);
	if (state == m_deviceState && (state != SignetApplication::STATE_LOGGED_OUT))
		return;
	switch (m_deviceState) {
	case SignetApplication::STATE_LOGGED_IN:
		break;
	case SignetApplication::STATE_BACKING_UP:
	case SignetApplication::STATE_EXPORTING:
		m_loggedInStack->setCurrentIndex(0);
		m_loggedInStack->removeWidget(m_backupWidget);
		m_backupWidget->deleteLater();
		m_backupWidget = nullptr;
		break;
	case SignetApplication::STATE_LOGGED_IN_LOADING_ACCOUNTS: {
		QWidget *w = m_loggedInStack->currentWidget();
		m_loggedInStack->setCurrentIndex(0);
		m_loggedInStack->removeWidget(w);
		w->deleteLater();
	}
	break;
	case SignetApplication::STATE_CONNECTING:
		m_connectingTimer.stop();
		m_connectingLabel = nullptr;
		break;
	default:
		break;
	}

	m_deviceState = static_cast<enum SignetApplication::device_state>(state);

	switch (m_deviceState) {
	case SignetApplication::STATE_DISCONNECTING: {
		m_loggedIn = false;
		QWidget *disconnecting_widget = new QWidget();
		QBoxLayout *layout = new QBoxLayout(QBoxLayout::TopToBottom);
		layout->setAlignment(Qt::AlignTop);
		disconnecting_widget->setLayout(layout);
		QLabel *label = new genericText("Disconnecting from device");
		layout->addWidget(label);
		disconnecting_widget->setLayout(layout);
		setCentralStack(disconnecting_widget);
	}
	break;
	case SignetApplication::STATE_DISCONNECTED: {
		m_loggedIn = false;
		QWidget *disconnected_widget = new QWidget();
		setCentralStack(disconnected_widget);
	}
	break;
	case SignetApplication::STATE_CONNECTING: {
		m_loggedIn = false;
		QWidget *connecting_widget = new QWidget();
		QBoxLayout *layout = new QBoxLayout(QBoxLayout::TopToBottom);
		layout->setAlignment(Qt::AlignTop);
		if (m_wasConnected && !m_fwUpgradeState) {
			m_connectingLabel = new genericText("No Signet device detected.\n\nPlease insert device.");
			m_wasConnected = false;
		} else {
			if (m_fwUpgradeState) {
				m_connectingLabel = new processingText("Waiting for device...");
			} else {
				m_connectingLabel = new processingText("Searching for device...");
				m_connectingTimer.setSingleShot(true);
				m_connectingTimer.setInterval(2000);
				m_connectingTimer.start();
			}
		}

		layout->addWidget(m_connectingLabel);

		m_deviceMenu->setDisabled(true);
		m_fileMenu->setDisabled(false);
		connecting_widget->setLayout(layout);
		setCentralStack(connecting_widget);
	}
	break;
	case SignetApplication::STATE_STARTING_DEVICE: {
		m_loggedIn = false;
		QWidget *starting_widget = new QWidget();
		QBoxLayout *layout = new QBoxLayout(QBoxLayout::TopToBottom);
		layout->setAlignment(Qt::AlignTop);
		layout->addWidget(new processingText("Starting up device...."));
		m_deviceMenu->setDisabled(true);
		m_fileMenu->setDisabled(false);
		starting_widget->setLayout(layout);
		setCentralStack(starting_widget);
	}
	break;
	case SignetApplication::STATE_RESET:
		m_loggedIn = false;
		break;
	case SignetApplication::STATE_WIPING: {
		m_loggedIn = false;
		m_wipingWidget = new QWidget(this);
		QBoxLayout *layout = new QBoxLayout(QBoxLayout::TopToBottom);
		layout->setAlignment(Qt::AlignTop);
		m_wipeProgress = new QProgressBar();
		layout->addWidget(new processingText("Wiping device..."));
		layout->addWidget(m_wipeProgress);
		m_wipingWidget->setLayout(layout);
		m_deviceMenu->setDisabled(true);
		m_fileMenu->setDisabled(true);
		setCentralStack(m_wipingWidget);
		::signetdev_get_progress(nullptr, &m_signetdevCmdToken, 0, DS_WIPING);
	}
	break;
	case SignetApplication::STATE_LOGGED_IN_LOADING_ACCOUNTS: {
		m_loggedIn = true;
		m_deviceMenu->setDisabled(true);
		m_fileMenu->setDisabled(true);
		QLabel *loading_label = new processingText("Loading...");

		QProgressBar *loading_progress = new QProgressBar();

		QVBoxLayout *layout = new QVBoxLayout();
		layout->setAlignment(Qt::AlignTop);
		layout->addWidget(loading_label);
		layout->addWidget(loading_progress);

		QWidget *loadingWidget = new QWidget();
		loadingWidget->setLayout(layout);

		m_loggedInWidget = new LoggedInWidget(loading_progress, this);
		connect(m_loggedInWidget, SIGNAL(abort()), this, SLOT(abort()));
		connect(m_loggedInWidget, SIGNAL(enterDeviceState(int)),
			this, SLOT(enterDeviceState(int)));
		connect(SignetApplication::get(), SIGNAL(signetdevEvent(int)), m_loggedInWidget, SLOT(signetDevEvent(int)));
		connect(m_loggedInWidget, SIGNAL(background()), this, SLOT(background()));

		m_loggedInStack = new QStackedWidget();
		m_loggedInStack->addWidget(m_loggedInWidget);
		m_loggedInStack->addWidget(loadingWidget);
		m_loggedInStack->setCurrentIndex(1);
		setCentralWidget(m_loggedInStack);
	}
	break;
	case SignetApplication::STATE_BACKING_UP: {
		m_loggedIn = true;
		m_backupWidget = new QWidget();
		QBoxLayout *layout = new QBoxLayout(QBoxLayout::TopToBottom);
		layout->setAlignment(Qt::AlignTop);
		m_backupProgress = new QProgressBar();
		QFileInfo fi(m_backupFile->fileName());
		layout->addWidget(new processingText("Backing up device to " + fi.fileName() + "..."));
		layout->addWidget(m_backupProgress);
		m_backupWidget->setLayout(layout);
		m_deviceMenu->setDisabled(true);
		m_fileMenu->setDisabled(true);
		m_loggedInStack->addWidget(m_backupWidget);
		m_loggedInStack->setCurrentWidget(m_backupWidget);
	}
	break;
	case SignetApplication::STATE_EXPORTING: {
		m_loggedIn = true;
		m_backupWidget = new QWidget();
		QBoxLayout *layout = new QBoxLayout(QBoxLayout::TopToBottom);
		layout->setAlignment(Qt::AlignTop);
		m_backupProgress = new QProgressBar();
		layout->addWidget(new genericText("Exporting to CSV"));
		layout->addWidget(m_backupProgress);
		m_backupWidget->setLayout(layout);
		m_deviceMenu->setDisabled(true);
		m_fileMenu->setDisabled(true);
		m_loggedInStack->addWidget(m_backupWidget);
		m_loggedInStack->setCurrentWidget(m_backupWidget);
	}
	break;
	case SignetApplication::STATE_RESTORING: {
		m_loggedIn = false;
		m_restoreWidget = new QWidget();
		QBoxLayout *layout = new QBoxLayout(QBoxLayout::TopToBottom);
		layout->setAlignment(Qt::AlignTop);
		m_restoreProgress = new QProgressBar();
		layout->addWidget(new processingText("Restoring device..."));
		layout->addWidget(m_restoreProgress);
		m_restoreWidget->setLayout(layout);
		m_deviceMenu->setDisabled(true);
		m_fileMenu->setDisabled(true);
		setCentralStack(m_restoreWidget);
	}
	break;
	case SignetApplication::STATE_UPDATING_FIRMWARE: {
		SignetApplication *app = SignetApplication::get();
		auto bootMode = app->getBootMode();
		switch (bootMode) {
		case HC_BOOT_BOOTLOADER_MODE:
			m_fwUpgradeState |= HC_UPGRADING_APPLICATION_MASK;
			break;
		case HC_BOOT_APPLICATION_MODE:
			m_fwUpgradeState |= HC_UPGRADING_BOOTLOADER_MASK;
			break;
		default:
			//TODO
			break;
		}
		createFirmwareUpdateWidget();

		m_deviceMenu->setDisabled(true);
		m_fileMenu->setDisabled(true);
		if (m_deviceType == SIGNETDEV_DEVICE_HC) {
			::signetdev_erase_pages_hc(nullptr, &m_signetdevCmdToken);
		} else {
			QByteArray erase_pages_;
			QByteArray page_mask(512, 0);

			for (auto iter = m_fwSections.begin(); iter != m_fwSections.end(); iter++) {
				const fwSection &section = (*iter);
				unsigned int lma = section.lma;
				unsigned int lma_end = lma + section.size;
				int page_begin = (lma - 0x8000000)/2048;
				int page_end = (lma_end - 1 - 0x8000000)/2048;
				for (int i  = page_begin; i <= page_end; i++) {
					if (i < 0)
						continue;
					if (i >= 511)
						continue;
					page_mask[i] = 1;
				}
			}

			for (int i = 0; i < 512; i++) {
				if (page_mask[i]) {
					erase_pages_.push_back(i);
				}
			}
			::signetdev_erase_pages(nullptr, &m_signetdevCmdToken,
						erase_pages_.size(),
						(u8 *)erase_pages_.data());
		}
		setCentralStack(m_firmwareUpdateWidget);
	}
	break;
	case SignetApplication::STATE_BOOTLOADER: {
		SignetApplication *app = SignetApplication::get();
		QIcon warn = app->style()->standardIcon(QStyle::SP_MessageBoxWarning);
		QWidget *bootloaderWidget = new QWidget();
		m_loggedIn = false;
		m_deviceMenu->setDisabled(true);
		m_fileMenu->setDisabled(false);
		QVBoxLayout *layout = new QVBoxLayout();
		layout->setAlignment(Qt::AlignTop);
		QPushButton *upgradeButton = new QPushButton("Upgrade firmware");
		QLabel *warningImage = new QLabel();
		warningImage->setPixmap(warn.pixmap(64, 64));
		warningImage->setAlignment(Qt::AlignHCenter);
		QLabel *failureMessage = new emphasisLargeText("Firmware upgrade incomplete");
		failureMessage->setAlignment(Qt::AlignHCenter);
		QLabel *instructionMessage = new genericText("Click below to complete the upgrade and access your device.");
		instructionMessage->setAlignment(Qt::AlignHCenter);
		layout->addWidget(warningImage);
		layout->addWidget(failureMessage);
		layout->addWidget(instructionMessage);
		layout->addWidget(upgradeButton);
		connect(upgradeButton, SIGNAL(pressed()), this, SLOT(updateFirmwareUi()));
		bootloaderWidget->setLayout(layout);
		setCentralStack(bootloaderWidget);
	} break;
	case SignetApplication::STATE_UNINITIALIZED: {
		m_loggedIn = false;
		m_deviceMenu->setDisabled(false);
		m_fileMenu->setDisabled(false);

		if (!databaseFile) {
			if (m_backupRestoreSupported) {
				m_restoreAction->setVisible(true);
			}
			m_eraseDeviceAction->setVisible(true);
			m_eraseDeviceAction->setText("Initialize");
		}

		m_logoutAction->setDisabled(true);
		m_backupAction->setVisible(false);
		if (uninitializedWipeSupported()) {
			m_wipeDeviceAction->setVisible(true);
		} else {
			m_wipeDeviceAction->setVisible(false);
		}

		if (uninitializedFirmwareUpdateSupported()) {
			m_updateFirmwareAction->setVisible(true);
		} else {
			m_updateFirmwareAction->setVisible(false);
		}
		m_changePasswordAction->setVisible(false);
		m_passwordSlots->setVisible(false);

		m_uninitPrompt = new QWidget();
		QVBoxLayout *layout = new QVBoxLayout();
		layout->setAlignment(Qt::AlignTop);
		QPushButton *init_button = new QPushButton("Initialize for the first time");
		QPushButton *restore_button = new QPushButton("Initialize from a backup file");
		layout->addWidget(new genericText("This device is uninitialized.\n\nSelect an option below to initialize the device.\n"));
		layout->addWidget(init_button);
		QLabel *orText = new genericText("OR");
		orText->setAlignment(Qt::AlignCenter);
		layout->addWidget(orText);
		layout->addWidget(restore_button);
		m_uninitPrompt->setLayout(layout);
		connect(init_button, SIGNAL(pressed()), this, SLOT(eraseDeviceUi()));
		connect(restore_button, SIGNAL(pressed()), this, SLOT(restoreDeviceUi()));
		setCentralStack(m_uninitPrompt);
	}
	break;
	case SignetApplication::STATE_LOGGED_OUT: {
		m_loggedIn = false;
		m_deviceMenu->setDisabled(false);
		m_fileMenu->setDisabled(false);
		int fwMaj, fwMin, fwStep;
		SignetApplication::get()->getConnectedFirmwareVersion(fwMaj, fwMin, fwStep);

		if (!databaseFile) {
			if ((m_deviceType == SIGNETDEV_DEVICE_ORIGINAL) && fwMaj == 1 && fwMin == 2 && fwStep == 1) {
				//Version 1.2.1 has a glitch that causes a lockup when starting
				// a restore from the logged out state
				m_restoreAction->setVisible(false);
			} else if (m_backupRestoreSupported) {
				m_restoreAction->setVisible(true);
			}
			m_wipeDeviceAction->setVisible(true);
			m_changePasswordAction->setVisible(true);
			m_eraseDeviceAction->setVisible(true);
			m_eraseDeviceAction->setText("Reinitialize");
		}
		m_passwordSlots->setVisible(false);
		m_backupAction->setVisible(false);
		m_logoutAction->setDisabled(true);
		m_updateFirmwareAction->setVisible(false);

		LoginWindow *login_window = new LoginWindow(this);
		connect(login_window, SIGNAL(enterDeviceState(int)),
			this, SLOT(enterDeviceState(int)));
		connect(login_window, SIGNAL(abort()), this, SLOT(abort()));
		setCentralStack(login_window);
	}
	break;
	case SignetApplication::STATE_LOGGED_IN: {
		m_loggedIn = true;
		m_logoutAction->setDisabled(false);
		m_deviceMenu->setDisabled(false);
		m_fileMenu->setDisabled(false);
		m_restoreAction->setVisible(false);
		m_wipeDeviceAction->setVisible(false);
		m_eraseDeviceAction->setVisible(false);
		m_loggedInStack->setCurrentIndex(0);

		if (!databaseFile) {
			if (m_backupRestoreSupported && !m_autoBackupCheckPerformed) {
				m_autoBackupCheckPerformed = true;
				autoBackupCheck();
				m_backupAction->setVisible(true);
			}
			m_changePasswordAction->setVisible(true);
			m_updateFirmwareAction->setVisible(true);
			SignetApplication *app = SignetApplication::get();
			int major;
			int minor;
			int step;
			app->getConnectedFirmwareVersion(major, minor, step);
			if (m_deviceType == SIGNETDEV_DEVICE_ORIGINAL && major == 1 && ((minor > 3) || (minor == 3 && step >= 2))) {
				m_passwordSlots->setVisible(true);
			}
		}
	}
	break;
	default:
		break;
	}

	if (!m_loggedIn) {
		m_autoBackupCheckPerformed = false;
	}

	bool fileActionsEnabled = (m_deviceState == SignetApplication::STATE_LOGGED_IN);
	m_exportMenu->menuAction()->setVisible(fileActionsEnabled);
	m_settingsAction->setVisible(fileActionsEnabled);
	m_importMenu->setDisabled(databaseFile || m_deviceState == SignetApplication::STATE_BOOTLOADER);
	m_importKeePassAction->setEnabled(fileActionsEnabled);
#ifdef Q_OS_UNIX
	m_importPassAction->setEnabled(fileActionsEnabled && m_passDatabaseFound);
#endif
	m_importCSVAction->setEnabled(fileActionsEnabled);
}

void MainWindow::openUi()
{
	QFileDialog *fd = new QFileDialog(this);
	QStringList filters;
	fd->setOption(QFileDialog::DontUseNativeDialog, true);
	fd->setDirectory(m_settings.localBackupPath);

	QDir backupPath(m_settings.localBackupPath);
	if (backupPath.exists()) {
		QStringList nameFilters;
		nameFilters.push_back(backupFilter());
		QFileInfoList files = backupPath.entryInfoList(nameFilters, QDir::Files, QDir::Time);
		if (files.size()) {
			fd->selectFile(files.first().fileName());
		}
	}
	filters.append(backupFilter());
	filters.append("*");
	fd->setNameFilters(filters);
	fd->setFileMode(QFileDialog::AnyFile);
	fd->setAcceptMode(QFileDialog::AcceptOpen);
	m_openFileDialog = fd;
	connect(m_openFileDialog, SIGNAL(finished(int)), this, SLOT(openFileDialogFinished(int)));
	m_openFileDialog->show();
}

void MainWindow::openFileDialogFinished(int rc)
{
	QFileDialog *fd = m_openFileDialog;
	m_openFileDialog->deleteLater();
	m_openFileDialog = nullptr;
	if (!rc)
		return;
	QStringList sl = fd->selectedFiles();
	if (sl.empty()) {
		return;
	}
	QString fn = sl.first();
	if (signetdev_emulate_init(fn.toLatin1().data())) {
		m_dbFilename = fn;
		::signetdev_close_connection();
		if (::signetdev_emulate_begin()) {
			enterDeviceState(SignetApplication::STATE_NEVER_SHOWN);
			enterDeviceState(SignetApplication::STATE_CONNECTING);
			::signetdev_startup(nullptr, &m_signetdevCmdToken);
		}
	} else {
		SignetApplication::messageBoxError(QMessageBox::Warning, "Open", "Database file not valid", this);
	}
}

void MainWindow::closeUi()
{
	if (m_dbFilename.size()) {
		::signetdev_emulate_end();
		m_dbFilename.clear();
		enterDeviceState(SignetApplication::STATE_NEVER_SHOWN);
		enterDeviceState(SignetApplication::STATE_CONNECTING);
		m_deviceType = signetdev_open_connection();
		SignetApplication::get()->setDeviceType(m_deviceType);
		if (m_deviceType != SIGNETDEV_DEVICE_NONE) {
			::signetdev_startup(nullptr, &m_signetdevCmdToken);
		}
	}
}

void MainWindow::openSettingsUi()
{
	SettingsDialog *config = new SettingsDialog(this, false);
	config->setWindowModality(Qt::WindowModal);
	config->setAttribute(Qt::WA_DeleteOnClose);
	connect(config, SIGNAL(finished(int)), this, SLOT(settingDialogFinished(int)));
	config->show();
}

void MainWindow::settingDialogFinished(int rc)
{
	if (!rc) {
		saveSettings();
		settingsChanged();
	}
}

void MainWindow::eraseDeviceUi()
{
	ResetDevice *rd = new ResetDevice(m_deviceState != SignetApplication::STATE_UNINITIALIZED, this);
	connect(rd, SIGNAL(enterDeviceState(int)),
		this, SLOT(enterDeviceState(int)));
	connect(rd, SIGNAL(abort()), this, SLOT(abort()));
	connect(rd, SIGNAL(finished(int)), rd, SLOT(deleteLater()));
	rd->show();
}

void MainWindow::abort()
{
	QMessageBox *box = SignetApplication::messageBoxError(QMessageBox::Critical, "Signet", "Unexpected device error. Press okay to exit.", this);
	connect(box, SIGNAL(finished(int)), this, SLOT(quit()));
}

void MainWindow::connectingTimer()
{
	m_connectingLabel->setText("No Signet device detected.\n\nPlease insert device.");
}

void MainWindow::resetTimer()
{
	m_resetTimer.stop();
	enterDeviceState(SignetApplication::STATE_CONNECTING);
	m_wasConnected = false;
	m_deviceType = ::signetdev_open_connection();
	SignetApplication::get()->setDeviceType(m_deviceType);
	if (m_deviceType != SIGNETDEV_DEVICE_NONE) {
		::signetdev_startup(nullptr, &m_signetdevCmdToken);
	}
}

int MainWindow::firmwareSizeHC()
{
	auto app = SignetApplication::get();
	int bootImageSz = 0;
	switch (app->getBootMode()) {
	case HC_BOOT_BOOTLOADER_MODE:
		bootImageSz = HC_BOOT_AREA_B_LEN;
		break;
	case HC_BOOT_APPLICATION_MODE:
		bootImageSz = HC_BOOT_AREA_A_LEN;
		break;
	default:
		break;
	}
	return bootImageSz;
}

void MainWindow::sendFirmwareWriteCmdHC()
{
	auto app = SignetApplication::get();
	unsigned int write_size = 512;
	const u8 *data = nullptr;
	switch (app->getBootMode()) {
	case HC_BOOT_BOOTLOADER_MODE:
		data = m_NewFirmwareBody->firmware_B + m_writingAddr;
		break;
	case HC_BOOT_APPLICATION_MODE:
		data = m_NewFirmwareBody->firmware_A + m_writingAddr;
		break;
	default:
		return;
	}
	::signetdev_write_flash(nullptr, &m_signetdevCmdToken, m_writingAddr, data, write_size);
	m_writingAddr += write_size;
	m_writingSize = write_size;
}

void MainWindow::sendFirmwareWriteCmd()
{
	bool advance = false;
	unsigned int section_lma = m_writingSectionIter->lma;
	int section_size = m_writingSectionIter->size;
	unsigned int section_end = section_lma + section_size;
	unsigned int write_size = 1024;
	if ((m_writingAddr + write_size) >= section_end) {
		write_size = section_end - m_writingAddr;
		advance = true;
	}
	void *data = m_writingSectionIter->contents.data() + (m_writingAddr - section_lma);

	::signetdev_write_flash(nullptr, &m_signetdevCmdToken, m_writingAddr, data, write_size);
	if (advance) {
		m_writingSectionIter++;
		if (m_writingSectionIter != m_fwSections.end()) {
			m_writingAddr = m_writingSectionIter->lma;
		}
	} else {
		m_writingAddr += write_size;
	}
	m_writingSize = write_size;
}

void MainWindow::firmwareFileInvalidMsg(const QString &err)
{
	SignetApplication::messageBoxError(QMessageBox::Warning, "Update firmware", QString("Invalid firmware: ").append(err), this);
}

void MainWindow::updateFirmwareHC(QByteArray &datum)
{
	if (datum.size() < (int)sizeof(struct hc_firmware_file_header)) {
		firmwareFileInvalidMsg("file too short!");
		return;
	}
	struct hc_firmware_file_header *header = (struct hc_firmware_file_header *)datum.data();
	if (header->file_prefix != HC_FIRMWARE_FILE_PREFIX) {
		firmwareFileInvalidMsg("not a firmware file!");
		return;
	}
	if (header->file_version != HC_FIRMWARE_FILE_VERSION) {
		firmwareFileInvalidMsg("wrong version!");
		return;
	}
    if (header->header_size != sizeof (struct hc_firmware_file_header)) {
		firmwareFileInvalidMsg("incorrect header size!");
		return;
	}
	int expected_sz = header->A_len + header->B_len + header->header_size;
	if (datum.size() != expected_sz) {
		firmwareFileInvalidMsg("incorrect file size!");
		return;
	}

	m_NewFirmwareHeader = new (struct hc_firmware_file_header);
	m_NewFirmwareBody = new (struct hc_firmware_file_body);

	memcpy(m_NewFirmwareHeader, datum.data(), sizeof (*m_NewFirmwareHeader));
	memcpy(m_NewFirmwareBody, datum.data() + header->header_size, sizeof (*m_NewFirmwareBody));

	u32 A_crc = crc32(0, m_NewFirmwareBody->firmware_A, sizeof(m_NewFirmwareBody->firmware_A));
	u32 B_crc = crc32(0, m_NewFirmwareBody->firmware_B, sizeof(m_NewFirmwareBody->firmware_B));
	if (A_crc != m_NewFirmwareHeader->A_crc || B_crc != m_NewFirmwareHeader->B_crc) {
		delete m_NewFirmwareBody;
		delete m_NewFirmwareHeader;
		m_NewFirmwareBody = nullptr;
		m_NewFirmwareHeader = nullptr;
		firmwareFileInvalidMsg("CRC error!");
		return;
	}
	updateFirmwareHCIter(true);

}

void MainWindow::updateFirmwareHCIter(bool buttonWait)
{
	SignetApplication *app = SignetApplication::get();

	struct hc_firmware_info info;

	auto bt_mode = app->getBootMode();
	switch (bt_mode) {
	case HC_BOOT_BOOTLOADER_MODE:
		info.firmware_crc = m_NewFirmwareHeader->B_crc;
		info.firmware_len = m_NewFirmwareHeader->B_len;
		memcpy(info.firmware_signature, m_NewFirmwareHeader->B_signature, sizeof (info.firmware_signature));
		break;
	case HC_BOOT_APPLICATION_MODE:
		info.firmware_crc = m_NewFirmwareHeader->A_crc;
		info.firmware_len = m_NewFirmwareHeader->A_len;
		memcpy(info.firmware_signature, m_NewFirmwareHeader->A_signature, sizeof (info.firmware_signature));
		break;
	default:
		return;
	}
	memcpy(info.firmware_signature_pubkey, m_NewFirmwareHeader->signature_pubkey, sizeof(info.firmware_signature_pubkey));

	if (buttonWait) {
		beginButtonWait("Update firmware", true);
	}
	::signetdev_begin_update_firmware_hc(nullptr, &m_signetdevCmdToken, &info);
}

void MainWindow::updateFirmware(QByteArray &datum)
{
	QJsonDocument doc = QJsonDocument::fromJson(datum);

	bool valid_fw = !doc.isNull() && doc.isObject();

	QJsonObject doc_obj;
	QJsonObject sections_obj;
	m_fwSections.clear();

	if (valid_fw) {
		doc_obj = doc.object();
		QJsonValue temp_val = doc_obj.value("sections");
		valid_fw = (temp_val != QJsonValue::Undefined) && temp_val.isObject();
		if (valid_fw) {
			sections_obj = temp_val.toObject();
		}
	}

	if (valid_fw) {
		for (auto iter = sections_obj.constBegin(); iter != sections_obj.constEnd() && valid_fw; iter++) {
			fwSection section;
			section.name = iter.key();
			QJsonValue temp = iter.value();
			if (!temp.isObject()) {
				valid_fw = false;
				break;
			}

			QJsonObject section_obj = temp.toObject();
			QJsonValue lma_val = section_obj.value("lma");
			QJsonValue size_val = section_obj.value("size");
			QJsonValue contents_val = section_obj.value("contents");

			if (lma_val == QJsonValue::Undefined ||
				size_val == QJsonValue::Undefined ||
				contents_val == QJsonValue::Undefined ||
				!lma_val.isDouble() ||
				!size_val.isDouble() ||
				!contents_val.isString()) {
				valid_fw = false;
				break;
			}
			section.lma = static_cast<unsigned int>(lma_val.toDouble());
			section.size = static_cast<int>(size_val.toDouble());
			section.contents = QByteArray::fromBase64(contents_val.toString().toLatin1());
			if (section.contents.size() != section.size) {
				valid_fw = false;
				break;
			}
			m_fwSections.append(section);
		}
	}
	if (valid_fw) {
		beginButtonWait("Update firmware", true);
		::signetdev_begin_update_firmware(nullptr, &m_signetdevCmdToken);
	} else {
		firmwareFileInvalidMsg("wrong format!");
	}
}

QString MainWindow::firmwareSuffix()
{
	if (m_deviceType == SIGNETDEV_DEVICE_HC) {
		return QString(SIGNET_HC_FIRMWARE_SUFFIX);
	} else {
		return QString(SIGNET_FIRMWARE_SUFFIX);
	}
}

QString MainWindow::firmwareFilter()
{
	return "*." + firmwareSuffix();
}

void MainWindow::updateFirmwareUi()
{
	QFileDialog *fd = new QFileDialog(this);
	QStringList filters;
	QString downloadsFolder = QStandardPaths::writableLocation(QStandardPaths::DownloadLocation);

	filters.append(firmwareFilter());
	filters.append("*");
	fd->setNameFilters(filters);
	fd->setDirectory(downloadsFolder);
	fd->setFileMode(QFileDialog::AnyFile);
	fd->setAcceptMode(QFileDialog::AcceptOpen);
	if (!fd->exec())
		return;
	QStringList sl = fd->selectedFiles();
	if (sl.empty()) {
		fd->deleteLater();
		return;
	}

	QFile firmware_update_file(sl.first());
	fd->deleteLater();
	bool result = firmware_update_file.open(QFile::ReadWrite);
	if (!result) {
		firmware_update_file.close();
		SignetApplication::messageBoxError(QMessageBox::Warning, "Update firmware", "Failed to open firmware file", this);
		return;
	}

	QByteArray datum = firmware_update_file.readAll();
	firmware_update_file.close();
	if (m_deviceType == SIGNETDEV_DEVICE_HC) {
		updateFirmwareHC(datum);
	} else {
		updateFirmware(datum);
	}
}

ButtonWaitWidget * MainWindow::beginButtonWait(QString action, bool longPress)
{
	if (m_loggedIn && m_buttonWaitWidget == nullptr) {
		m_buttonWaitWidget = m_loggedInWidget->beginButtonWait(action, longPress);
		connect(m_buttonWaitWidget, SIGNAL(timeout()), this, SLOT(buttonWaitTimeout()));
		connect(m_buttonWaitWidget, SIGNAL(canceled()), this, SLOT(buttonWaitCancel()));
	} else if (m_buttonWaitWidget == nullptr) {
		QStackedWidget *stack = static_cast<QStackedWidget *>(centralWidget());
		m_buttonWaitWidget = new ButtonWaitWidget(action, longPress);
		connect(m_buttonWaitWidget, SIGNAL(timeout()), this, SLOT(buttonWaitTimeout()));
		connect(m_buttonWaitWidget, SIGNAL(canceled()), this, SLOT(buttonWaitCancel()));
		int index = stack->addWidget(m_buttonWaitWidget);
		stack->setCurrentIndex(index);
	}
	return m_buttonWaitWidget;
}

void MainWindow::endButtonWait()
{
	if (m_loggedIn && m_buttonWaitWidget) {
		m_loggedInWidget->endButtonWait();
		m_buttonWaitWidget = nullptr;
	} else if (m_buttonWaitWidget) {
		QStackedWidget *stack = static_cast<QStackedWidget *>(centralWidget());
		stack->removeWidget(m_buttonWaitWidget);
		m_buttonWaitWidget->deleteLater();
		m_buttonWaitWidget = nullptr;
	}
}

void MainWindow::wipeDeviceUi()
{
	m_wipeDeviceDialog = new QMessageBox(QMessageBox::Warning,
						 "Wipe device",
						 "This will permanently erase the contents of the device. Continue?",
						 QMessageBox::Ok | QMessageBox::Cancel,
						 this);
	connect(m_wipeDeviceDialog, SIGNAL(finished(int)), this, SLOT(wipeDeviceDialogFinished(int)));
	connect(m_wipeDeviceDialog, SIGNAL(finished(int)), m_wipeDeviceDialog, SLOT(deleteLater()));
	m_wipeDeviceDialog->setWindowModality(Qt::WindowModal);
	m_wipeDeviceDialog->show();
}

void MainWindow::wipeDeviceDialogFinished(int result)
{
	if (result != QMessageBox::Ok) {
		return;
	}
	beginButtonWait("wipe device", true);
	::signetdev_wipe(nullptr, &m_signetdevCmdToken);
}

void MainWindow::backupDevice(QString fileName)
{
	m_backupFile = new QFile(fileName);
	bool result = m_backupFile->open(QFile::ReadWrite);
	if (!result) {
		SignetApplication::messageBoxError(QMessageBox::Warning, "Backup device", "Failed to create destination file", this);
		return;
	}
	QFileInfo fi(m_backupFile->fileName());
	beginButtonWait("start backing up device to " + fi.fileName(), true);
	::signetdev_begin_device_backup(nullptr, &m_signetdevCmdToken);
}

void MainWindow::aboutUi()
{
	bool connected = (m_deviceState == SignetApplication::STATE_LOGGED_IN) || (m_deviceState == SignetApplication::STATE_LOGGED_OUT)
			 || (m_deviceState == SignetApplication::STATE_UNINITIALIZED);
	QDialog *dlg = new About(connected,this);
	dlg->exec();
}

void MainWindow::backupDeviceUi()
{
	QString backupFileName = m_settings.localBackupPath + "/" + backupFileBaseName() + "." + backupSuffix();
	QDir backupPath(m_settings.localBackupPath);
	if (!backupPath.exists()) {
		QString dirName = backupPath.dirName();
		if (backupPath.cdUp()) {
			backupPath.mkdir(dirName);
			backupPath.setPath(m_settings.localBackupPath);
		}
	}

	QFileDialog * fd = new QFileDialog(this, "Backup device to file");
	QStringList filters;
	filters.append(backupFilter());
	filters.append("*");
	fd->setDirectory(m_settings.localBackupPath);
	fd->selectFile(backupFileName);
	fd->setNameFilters(filters);
	fd->setFileMode(QFileDialog::AnyFile);
	fd->setAcceptMode(QFileDialog::AcceptSave);
	fd->setDefaultSuffix(backupSuffix());
	int rc = fd->exec();
	if (!rc) {
		fd->deleteLater();
		return;
	}
	QStringList sl = fd->selectedFiles();
	fd->deleteLater();
	if (sl.empty())
		return;
	backupDevice(sl.first());
}

void MainWindow::exportCSVUi()
{
	QFileDialog *fd = new QFileDialog(this, "Export device to CSV archive");
	QStringList filters;
	filters.append("*.zip");
	filters.append("*");
	QString fileName = backupFileBaseName();
	QString documentsPath = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);
	QString backupFileName = documentsPath + "/" + fileName + ".zip";
	fd->setNameFilters(filters);

	fd->setDirectory(documentsPath);
	fd->selectFile(fileName);
	fd->setFileMode(QFileDialog::AnyFile);
	fd->setAcceptMode(QFileDialog::AcceptSave);
	fd->setDefaultSuffix(QString("zip"));
	if (!fd->exec()) {
		fd->deleteLater();
		return;
	}
	QStringList sl = fd->selectedFiles();
	fd->deleteLater();
	if (sl.empty())
		return;
	m_backupZipFile = zipOpen(sl.first().toLatin1().data(), APPEND_STATUS_CREATE);
	if (!m_backupZipFile) {
		SignetApplication::messageBoxError(QMessageBox::Warning, "Export to CSV archive", "Failed to create CSV archive", this);
		return;
	}
	beginButtonWait("export to CSV archive", true);
	m_startedExport = true;
	::signetdev_read_all_uids(nullptr, &m_signetdevCmdToken, 0);
}

void MainWindow::importDone(bool success)
{
	Q_UNUSED(success);
	m_dbImportController->deleteLater();
	m_dbImportController = nullptr;
}

void MainWindow::startImport(DatabaseImporter *importer)
{
	SignetApplication *app = SignetApplication::get();
	int majorVer;
	int minorVer;
	int stepVer;
	app->getConnectedFirmwareVersion(majorVer, minorVer, stepVer);
	bool useUpdateUids =
		(
			(m_deviceType == SIGNETDEV_DEVICE_ORIGINAL) &&
			(majorVer == 1) &&
			(
			(minorVer > 3) ||
				(
					(minorVer == 3) &&
					(stepVer >= 3)
				)
			)
		) ||
		(
			m_deviceType == SIGNETDEV_DEVICE_HC
		);
	m_dbImportController = new DatabaseImportController(importer, m_loggedInWidget, useUpdateUids);
	connect(m_dbImportController, SIGNAL(done(bool)), this, SLOT(importDone(bool)));
	m_dbImportController->start();
}

void MainWindow::importKeePassUI()
{
	DatabaseImporter *importer = new KeePassImporter(this);
	startImport(importer);
}

#include "cleartextpasswordselector.h"

void MainWindow::passwordSlotsUi()
{
	::signetdev_read_cleartext_password_names(nullptr, &m_signetdevCmdToken);
}

void MainWindow::signetdevReadCleartextPasswordNames(signetdevCmdRespInfo info, QVector<int> formats, QStringList names)
{
	if (info.resp_code == OKAY) {
		cleartextPasswordSelector *s = new cleartextPasswordSelector(formats, names, this);
		s->setMinimumWidth(300);
		s->setWindowTitle("Password slots");
		s->setWindowModality(Qt::WindowModal);
		s->setAttribute(Qt::WA_DeleteOnClose);
		s->show();
	}
}

#ifdef Q_OS_UNIX
void MainWindow::importPassUI()
{
	DatabaseImporter *importer = new PassImporter(this);
	startImport(importer);
}
#endif

void MainWindow::importCSVUI()
{
	QList<esdbTypeModule *> typeModules = m_loggedInWidget->getTypeModules();

	DatabaseImporter *importer = new CSVImporter(typeModules, this);
	startImport(importer);
}

void MainWindow::restoreDeviceUi()
{
	QFileDialog *fd = new QFileDialog(this, "Restore device from file");
	QStringList filters;
	filters.append(backupFilter());
	filters.append("*");
	fd->setDirectory(m_settings.localBackupPath);
	fd->setNameFilters(filters);
	fd->setFileMode(QFileDialog::AnyFile);
	fd->setAcceptMode(QFileDialog::AcceptOpen);
	fd->setDefaultSuffix(backupSuffix());
	if (!fd->exec()) {
		fd->deleteLater();
		return;
	}
	QStringList sl = fd->selectedFiles();
	fd->deleteLater();
	if (sl.empty()) {
		return;
	}
	m_restoreFile = new QFile(sl.first());
	bool result = m_restoreFile->open(QFile::ReadWrite);
	if (!result) {
		delete m_restoreFile;
		SignetApplication::messageBoxError(QMessageBox::Warning, "Restore device from file", "Failed to open backup file", this);
		return;
	}
	if (m_restoreFile->size() != ::signetdev_device_block_size() * (::signetdev_device_num_data_blocks() + ::signetdev_device_num_root_blocks())) {
		delete m_restoreFile;
		SignetApplication::messageBoxError(QMessageBox::Warning, "Restore device from file", "Backup file has wrong size", this);
		return;
	}

	beginButtonWait("start restoring device", true);
	::signetdev_begin_device_restore(nullptr, &m_signetdevCmdToken);
}
