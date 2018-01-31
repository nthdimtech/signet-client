#include "databaseimporter.h"

DatabaseImporter::DatabaseImporter(QObject *parent) : QObject(parent),
	m_db(NULL)
{

}

DatabaseImporter::~DatabaseImporter()
{
	if (m_db)
		delete m_db;
}

void DatabaseImporter::start()
{

}
