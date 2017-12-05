#include "signetapplication.h"

int main(int argc, char **argv)
{
	SignetApplication a(argc, argv);
	if (a.isRunning()) {
		QIcon signetIcon = QIcon(":/images/signet.png");
		QMessageBox *box = new QMessageBox(QMessageBox::Information, "Signet",
						   "An instance of the Signet client is already running",
						   0,
						   NULL,
						   Qt::WindowStaysOnTopHint);
		QPushButton *closeItButton = box->addButton("Close it", QMessageBox::RejectRole);
		box->addButton("Open it", QMessageBox::AcceptRole);
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
			}
		} else {
			a.sendMessage("open");
			return 0;
		}
		box->deleteLater();
	}
	a.init();
	a.exec();
	return 0;
}
