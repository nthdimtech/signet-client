#ifndef SIGNETDEVICEMANAGER_H
#define SIGNETDEVICEMANAGER_H

#include "signetapplication.h"
#include "esdb.h"

#include <QObject>
#include <QTimer>
#include <QQmlApplicationEngine>
#include <QSet>
#include <QStringList>

#include "esdb/account/esdbaccountmodule.h"
#include "esdb-gui/esdbmodel.h"
#include "esdb/esdb.h"

class EsdbEntryModel;
class EsdbGroupModel;

class KeyGeneratorThread;
#include <QString>

class SignetDeviceManager : public QObject
{
	Q_OBJECT
	enum SignetApplication::device_state m_deviceState;
	int m_signetdevCmdToken;
	QTimer m_connectingTimer;
	QQmlApplicationEngine &m_qmlEngine;
	KeyGeneratorThread *m_keyGeneratorThread;
	void setLoaderSource(QString str);
	QObject *findQMLObject(QString name);
	QList<esdbEntry *> m_entries;
	QList<esdbEntry *> m_entriesFiltered;
	esdbAccountModule m_acctTypeModule;
	EsdbEntryModel *m_model;
	EsdbGroupModel *m_groupModel;
	QSet<QString> m_groups;
	QStringList m_groupsSorted;
	QObject *m_loadingProgress;
	QString m_filterGroup;
	QString m_filterEntry;
	int m_entriesLoaded;
	void filterEntries(QString groupName, QString search);
public:
	explicit SignetDeviceManager(QQmlApplicationEngine &engine, QObject *parent = nullptr);

signals:
	void abort();
	void badPasswordEntered();
public slots:
	void connectingTimer();
	void connectionError();
	void enterDeviceState(int);
	void deviceOpened();
	void deviceClosed();
	void signetdevCmdResp(signetdevCmdRespInfo info);
	void signetdevEvent(int eventType);
	void signetdevStartupResp(signetdevCmdRespInfo info, signetdev_startup_resp_data resp);
	void signetdevReadBlockResp(signetdevCmdRespInfo info, QByteArray block);
	void signetdevGetProgressResp(signetdevCmdRespInfo info, signetdev_get_progress_resp_data data);
	void signetdevReadUIdResp(signetdevCmdRespInfo info, QByteArray data, QByteArray mask);
	void signetdevReadAllUIdsResp(signetdevCmdRespInfo info, int id, QByteArray data, QByteArray mask);
private slots:
	void loaded();
	void loginSignal(QString password);
	void keyGenerationFinished();
	void lockSignal();
	void filterTextChangedSignal(QString text);
	void filterGroupChangedSignal(int index);
	void copyUsernameSignal(int index);
	void copyPasswordSignal(int index);
};

#endif // SIGNETDEVICEMANAGER_H
