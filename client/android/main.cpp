#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include "signetapplication.h"
#include "esdbmodel.h"
#include <android/log.h>

#include <QStringListModel>
#include <QQmlContext>
#include <esdb/account/esdbaccountmodule.h>
#include <QList>
#include <QStringList>

int main(int argc, char *argv[])
{
	QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);

	SignetApplication app(argc, argv);
	__android_log_print(ANDROID_LOG_DEBUG, "SIGNET_ACTIVITY", "App start\n");

	app.qmlEngine().load(QUrl(QStringLiteral("qrc:/main.qml")));
	if (app.qmlEngine().rootObjects().isEmpty())
		return -1;

	app.init(false);

	int rc = app.exec();
	__android_log_print(ANDROID_LOG_DEBUG, "SIGNET_ACTIVITY", "App exit %d\n", rc);
	return rc;
}
