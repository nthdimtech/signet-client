#ifndef SIGNETDEVSERVERCONNECTION_H
#define SIGNETDEVSERVERCONNECTION_H

#include <QObject>
#include <QQueue>

class signetdevServer;
class QWebSocket;
struct signetdevServerCommand;

class signetdevServerConnection : public QObject
{
	Q_OBJECT
	signetdevServer *m_parent;
public:
	QQueue<signetdevServerCommand *> m_commandQueue;
	QWebSocket *m_socket;
	signetdevServerConnection(QWebSocket *socket, signetdevServer *parent);
public slots:
	void textMessageReceived(const QString &message);
};

#endif // SIGNETDEVSERVERCONNECTION_H
