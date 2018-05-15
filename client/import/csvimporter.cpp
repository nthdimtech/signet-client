#include "csvimporter.h"
#include "signetapplication.h"

#include <QWidget>
#include <QFileDialog>

#include "qtcsv/reader.h"
#include "esdb.h"
#include "esdbtypemodule.h"
#include "csvimportconfigure.h"

CSVImporter::CSVImporter(QList<esdbTypeModule *> typeModules, QWidget *parent) :
	DatabaseImporter(parent),
	m_parent(parent),
	m_typeModules(typeModules)
{

}

void CSVImporter::start()
{
	QFileDialog *fd = new QFileDialog(m_parent, databaseTypeName() + " Import");
	QStringList filters;
	filters.append("*.csv");
	filters.append("*.txt");
	filters.append("*");
	fd->setNameFilters(filters);
	fd->setFileMode(QFileDialog::AnyFile);
	fd->setAcceptMode(QFileDialog::AcceptOpen);
	fd->setWindowModality(Qt::WindowModal);
	if (!fd->exec()) {
		done(false);
		return;
	}
	QStringList sl = fd->selectedFiles();
	if (sl.empty()) {
		done(false);
		return;
	}
	fd->deleteLater();

	QFile *csvFile = new QFile(sl.first());
	if (!csvFile->open(QFile::ReadOnly)) {
		csvFile->deleteLater();
		auto mb = SignetApplication::messageBoxError(QMessageBox::Warning,
						   databaseTypeName() + " Import",
						   "Failed to open CSV file",
						   m_parent);
		mb->exec();
		done(false);
		return;
	}

	QtCSV::Reader reader;
	QList<QStringList> csvData = reader.readToList(*csvFile);
	csvFile->deleteLater();
	if (csvData.length() < 2) {
		done(false);
		return;
	}

	if (!m_typeModules.size()) {
		done(false);
		return;
	}

	CSVImportConfigure *config = new CSVImportConfigure(this, m_parent);
	config->setWindowTitle("CSV Import Settings");
	int rc = config->exec();
	if (rc != 0) {
		done(false);
		return;
	}

	esdbTypeModule *t = config->typeModule();

	QList<QStringList>::iterator iter = csvData.begin();
	const QStringList &header = *(iter++);
	m_db = new database();
	databaseType *selectedType = new databaseType();

	m_db->insert(t->name(), selectedType);

	for (;iter != csvData.end(); iter++) {
		QVector<genericField> fields;
		QStringList row = *iter;
		for (int j = 0; j < row.length() && j < header.length(); j++) {
			fields.append(genericField(header.at(j),"", row.at(j)));
		}
		esdbEntry *ent = t->decodeEntry(fields);
		if (ent) {
			selectedType->append(ent);
		}
	}
	done(true);
}
