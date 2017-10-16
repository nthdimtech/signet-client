#include "signetdevserverconnection.h"

#include <QWebSocket>

signetdevServerConnection::signetdevServerConnection(QWebSocket *socket, signetdevServer *parent) :
	QObject((QObject *)parent),
	m_parent(parent),
	m_socket(socket)
{
	m_socket->setParent(this);
	connect(m_socket, SIGNAL(textMessageReceived(QString)), this, SLOT(textMessageReceived(QString)));
}

void signetdevServerConnection::textMessageReceived(const QString &message)
{
}
