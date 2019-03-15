#ifndef PASSIMPORTER_H
#define PASSIMPORTER_H

#include "databaseimporter.h"

#include <QString>
#include <QDir>

class databaseType;

class PassImporter : public DatabaseImporter
{
	Q_OBJECT
	QWidget *m_parent;
	void traverse(QString path, QDir &dir);
	static QString passwordStorePath();
	static QString gpgIdPath();
	QString m_gpgId;
	QString m_passphrase;
	databaseType *m_accountType;
public:
	QString databaseTypeName()
	{
		return QString("Pass Database");
	}
	static QString getGPGId();
	bool passphraseCheck(QString passphrase);
	PassImporter(QWidget *parent);
public slots:
	void start();
private slots:
	void doneFail();
};

#endif // PASSIMPORTER_H
