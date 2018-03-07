#include "signetapplication.h"

#include "crypto/Crypto.h"

#include <QThread>
#include <QCommandLineParser>

int main(int argc, char **argv)
{
	SignetApplication a(argc, argv);

	Crypto::init();

	if (a.isRunning()) {
		QIcon signetIcon = QIcon(":/images/signet.png");
		QMessageBox *box = new QMessageBox(QMessageBox::Information, "Signet",
						   "A Signet client is already running."
						   "Only one instance of the client can run at the same time.",
						   0,
						   NULL,
						   Qt::WindowStaysOnTopHint);
		QPushButton *closeItButton = box->addButton("Replace running instance", QMessageBox::RejectRole);
		box->addButton("Switch to running instance", QMessageBox::AcceptRole);
		box->setWindowIcon(signetIcon);
		box->exec();
		QAbstractButton *clickedButton = box->clickedButton();
		box->deleteLater();
		box = NULL;
		if (clickedButton == (QAbstractButton *)closeItButton) {
			a.sendMessage("close");
			int i = 0;
			while(a.isRunning()) {
				QCoreApplication::processEvents(QEventLoop::AllEvents, 100);
				i++;
				if (i > 200) {
					box = new QMessageBox(QMessageBox::Information, "Signet",
									   "Existing signet client is not closing. Close it and try again",
									   QMessageBox::Ok,
									   NULL,
									   Qt::WindowStaysOnTopHint);
					box->exec();
					box->deleteLater();
					box = NULL;
					return 0;
				}
				QThread::usleep(100000);
			}
		} else {
			a.sendMessage("open");
			return 0;
		}
		box->deleteLater();
	}

#ifdef Q_OS_UNIX
	QCommandLineParser parser;
	QCommandLineOption startInTrayOption("start-in-systray", "Start application minimized in the system tray");

	parser.addOption(startInTrayOption);
        parser.addHelpOption();
	parser.process(a);

        bool startInTray = parser.isSet(startInTrayOption);
#else
        bool startInTray = false;
#endif

	a.init(startInTray);
	a.exec();
	return 0;
}
