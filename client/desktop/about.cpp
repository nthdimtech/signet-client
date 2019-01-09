#include "about.h"

#include <QVBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QDesktopServices>
#include <QUrl>
#include <QDebug>

#include "signetapplication.h"

//Remove this to display git revision in about window
#undef GITVERSION

About::About(bool connectedDevice, QWidget *parent):
	QDialog(parent)
{
	setWindowModality(Qt::WindowModal);
	setWindowTitle("About Signet");
	QHBoxLayout *top = new QHBoxLayout();
	QLabel *graphic = new QLabel();
	graphic->setMargin(5);
	graphic->setPixmap(QPixmap(":/images/signet.png"));
	graphic->setScaledContents(true);
	graphic->setFixedSize(128, 128);
	graphic->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
	graphic->setAlignment(Qt::AlignLeft);
	top->addWidget(graphic);

	QVBoxLayout *right = new QVBoxLayout();
	top->addLayout(right);
	top->setAlignment(Qt::AlignTop);
	setLayout(top);

	SignetApplication *app = SignetApplication::get();

	QDate date = app->getReleaseDate();
	int clientMajVer, clientMinVer, clientStepVer, clientSubStepVer;
	int firmwareMajVer, firmwareMinVer, firmwareStepVer;
	int deviceFirwmareMajVer, deviceFirwmareMinVer, deviceFirwmareStepVer;
	app->getClientVersion(clientMajVer, clientMinVer, clientStepVer, clientSubStepVer);
	app->getFirmwareVersion(firmwareMajVer, firmwareMinVer,firmwareStepVer);
	app->getConnectedFirmwareVersion(deviceFirwmareMajVer, deviceFirwmareMinVer, deviceFirwmareStepVer);

	QString versionString = QString("Signet client ") +
			QString::number(clientMajVer) + "." +
			QString::number(clientMinVer) + "." +
			QString::number(clientStepVer);

	if (clientSubStepVer != 0) {
		versionString += "." + QString::number(clientSubStepVer);
	}

#ifdef GITVERSION
	versionString += " g:" + QString(GITVERSION);
	qDebug() << "VERSION: " << versionString;
#endif

	QLabel *title = new QLabel(versionString);

	title->setStyleSheet("font-weight: bold");

	QLabel *releaseDate = new QLabel(QString("Released ") +
					     date.toString());

	QLabel *targetFirmware = new QLabel(QString("Target firmware version ") +
				   QString::number(firmwareMajVer) + "." +
				   QString::number(firmwareMinVer) + "." +
				   QString::number(firmwareStepVer));

	right->addWidget(title);
	right->addWidget(releaseDate);
	right->addWidget(targetFirmware);

	if (connectedDevice) {
		QLabel *deviceFirmware = new QLabel(QString("Device firmware version ") +
					   QString::number(deviceFirwmareMajVer) + "." +
					   QString::number(deviceFirwmareMinVer) + "." +
					   QString::number(deviceFirwmareStepVer));
		right->addWidget(deviceFirmware);
	}

	QHBoxLayout *buttons = new QHBoxLayout();
	buttons->setAlignment(Qt::AlignBottom);
	QPushButton *closeButton = new QPushButton("Close");
	closeButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
	QPushButton *checkNewVersionButton = new QPushButton("Check for new version");
	checkNewVersionButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
	buttons->addWidget(checkNewVersionButton);
	buttons->addWidget(closeButton);
	right->addLayout(buttons);
	connect(closeButton, SIGNAL(clicked(bool)), this, SLOT(close()));
	connect(checkNewVersionButton, SIGNAL(clicked(bool)), this, SLOT(checkNewVersion()));
}

void About::checkNewVersion()
{
	QUrl url("https://nthdimtech.com/signet-releases");
	QDesktopServices::openUrl(url);
}
