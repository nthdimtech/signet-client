#ifndef CSVIMPORTER_H
#define CSVIMPORTER_H

#include "databaseimporter.h"

class QWidget;

#include <QList>

class databaseType;
struct esdbTypeModule;

class CSVImporter : public DatabaseImporter
{
	Q_OBJECT
	QWidget *m_parent;
	QList<esdbTypeModule *> m_typeModules;
public:
	CSVImporter(QList<esdbTypeModule *> typeModules,
			QWidget *parent);
	QString databaseTypeName()
	{
		return QString("CSV");
	}

	const QList<esdbTypeModule *> &typeModules()
	{
		return m_typeModules;
	}

public slots:
	void start();
private slots:
	void failedToOpenCSVDialogFinished(int rc);
};

#endif // CSVIMPORTER_H
