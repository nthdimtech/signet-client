#ifndef SIGNETDEVSERVERCONNECTION_H
#define SIGNETDEVSERVERCONNECTION_H

#include <QObject>

class signetdevServer;
class QWebSocket;

class signetdevServerConnection : public QObject
{
	Q_OBJECT
	signetdevServer *m_parent;
public:
	QWebSocket *m_socket;
	signetdevServerConnection(QWebSocket *socket, signetdevServer *parent);
public slots:
	void textMessageReceived(const QString &message);
};

#endif // SIGNETDEVSERVERCONNECTION_H
