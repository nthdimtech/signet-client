#ifndef DATABASEIMPORTER_H
#define DATABASEIMPORTER_H

#include <QObject>
#include <QMap>
#include <QList>
#include <QString>

struct esdbEntry;

class DatabaseImporter : public QObject
{
	Q_OBJECT
	int m_signetdevCmdToken;
public:
	typedef QList<esdbEntry *> databaseType;
	typedef QMap<QString, databaseType *> database;
	typedef database::iterator databaseIter;
	typedef databaseType::iterator databaseTypeIter;
protected:
	database *m_db;
public:
	explicit DatabaseImporter(QObject *parent = 0);
	database *getDatabase()
	{
		return m_db;
	}

	virtual ~DatabaseImporter();

	virtual QString databaseTypeName() = 0;
signals:
	void done(bool success);
public slots:
	virtual void start();
};

#endif // DATABASEIMPORTER_H
