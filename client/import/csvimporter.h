#ifndef CSVIMPORTER_H
#define CSVIMPORTER_H

#include "databaseimporter.h"

class QWidget;

#include <QList>

class databaseType;
class esdbTypeModule;

class CSVImporter : public DatabaseImporter
{
Q_OBJECT
	QWidget *m_parent;
	databaseType *m_accountType;
	esdbTypeModule *m_typeModule;
	QList<esdbTypeModule *> m_typeModules;
public:
	CSVImporter(QList<esdbTypeModule *> typeModules, QWidget *parent);
	QString databaseTypeName() {
		return QString("CSV");
	}

	const QList<esdbTypeModule *> &typeModules() {
		return m_typeModules;
	}

public slots:
	void start();
};

#endif // CSVIMPORTER_H
