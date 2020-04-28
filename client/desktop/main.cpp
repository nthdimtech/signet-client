#include "signetapplication.h"

#include "crypto/Crypto.h"

#include <QThread>
#include <QCommandLineParser>
#include <QPushButton>

int main(int argc, char **argv)
{
#if QT_VERSION >= 0x00050600
	QCoreApplication::setAttribute(Qt::AA_DisableHighDpiScaling);
#endif
	SignetApplication *app = new SignetApplication(argc, argv);

	Crypto::init();

	if (app->isRunning()) {
		QIcon signetIcon = QIcon(":/images/signet.png");
		QMessageBox *box = new QMessageBox(QMessageBox::Information, "Signet",
						   "A Signet client is already running."
						   "Only one instance of the client can run at the same time.",
                           nullptr,
                           nullptr,
						   Qt::WindowStaysOnTopHint);
		QPushButton *closeItButton = box->addButton("Replace running instance", QMessageBox::RejectRole);
		box->addButton("Switch to running instance", QMessageBox::AcceptRole);
		box->setWindowIcon(signetIcon);
		box->exec();
		QAbstractButton *clickedButton = box->clickedButton();
		box->deleteLater();
		box = nullptr;
		if (clickedButton == static_cast<QAbstractButton *>(closeItButton)) {
			app->sendMessage("close");
			int i = 0;
			while(app->isRunning()) {
				QCoreApplication::processEvents(QEventLoop::AllEvents, 100);
				i++;
				if (i > 200) {
					box = new QMessageBox(QMessageBox::Information, "Signet",
                                  "Existing signet client is not closing. Close it and try again",
                                  QMessageBox::Ok,
                                  nullptr,
                                  Qt::WindowStaysOnTopHint);
					box->exec();
					box->deleteLater();
					box = NULL;
					return 0;
				}
				QThread::usleep(100000);
			}
		} else {
			app->sendMessage("open");
			return 0;
		}
		box->deleteLater();
	}

	QString dbFile;

	QCommandLineParser parser;
	QCommandLineOption file("file", "Read database from specified file", "file-name");
	parser.addOption(file);
	parser.addHelpOption();
#ifdef Q_OS_UNIX
	QCommandLineOption startInTrayOption("start-in-systray", "Start application minimized in the system tray");
	parser.addOption(startInTrayOption);
#endif
	parser.process(*app);

#ifdef Q_OS_UNIX
	bool startInTray = parser.isSet(startInTrayOption);
#else
	bool startInTray = false;
#endif
	if (parser.isSet(file)) {
		dbFile = parser.value(file);
	}

	app->setQuitOnLastWindowClosed(false);

	app->init(startInTray, dbFile);
	app->exec();

	signetdev_deinitialize_api();

	delete app;

	return 0;
}
