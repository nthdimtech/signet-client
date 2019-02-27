#ifndef KEEPASSIMPORTER_H
#define KEEPASSIMPORTER_H

#include "databaseimporter.h"

#include <QString>

class Database;
class Group;
class databaseType;

class KeePassImporter : public DatabaseImporter
{
Q_OBJECT
	QWidget *m_parent;
	Database *m_keePassDatabase;
	void traverse(QString path, Group *group);
	databaseType *m_accountType;
	void failedToOpenDatabaseDialogFinished(int rc);
public:
	KeePassImporter(QWidget *parent = 0);

	QString databaseTypeName() {
		return QString("KeePass 2.x Database");
	}
public slots:
	void start();
private slots:
	void invalidCredientialsDialogFinished(int rc);
};

#endif // KEEPASSIMPORTER_H
