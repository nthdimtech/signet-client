#include "websockethandler.h"

#include <QWebSocket>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

websocketHandler::websocketHandler(QWebSocket *socket, int socketId, QObject *parent) : QObject(parent),
        m_socketId(socketId),
        m_socket(socket)
{
	socket->setParent(this);
	connect(socket, SIGNAL(textMessageReceived(QString)), this, SLOT(textMessageReceived(QString)));
	connect(socket, SIGNAL(disconnected()), this, SLOT(disconnected()));
}

void websocketHandler::websocketResponse(const QString &response)
{
	m_socket->sendTextMessage(response);
}

void websocketHandler::textMessageReceived(QString message)
{
	websocketMessage(m_socketId, message);
}

void websocketHandler::disconnected()
{
	done(this);
}
