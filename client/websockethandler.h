#ifndef WEBSOCKETHANDLER_H
#define WEBSOCKETHANDLER_H

#include <QObject>
class QWebSocket;

class websocketHandler : public QObject
{
	Q_OBJECT
	int m_socketId;
	QWebSocket *m_socket;
public:
	explicit websocketHandler(QWebSocket *socket, int id, QObject *parent = nullptr);
	void websocketResponse(const QString &response);
	int id() const
	{
		return m_socketId;
	}
signals:
	void done(websocketHandler *);
	void websocketMessage(int socketId, QString message);
private slots:
	void textMessageReceived(QString message);
	void disconnected();
};

#endif // WEBSOCKETHANDLER_H
