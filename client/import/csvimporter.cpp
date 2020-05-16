#include "csvimporter.h"
#include "signetapplication.h"

#include <QWidget>
#include <QFileDialog>
#include <QFileInfo>
#include <QBuffer>

#include "qtcsv/reader.h"
#include "esdb.h"
#include "esdbtypemodule.h"
#include "csvimportconfigure.h"
#include "generictypedesc.h"
#include "esdbgenericmodule.h"
#include "unzip.h"
#include "generic.h"

#include <QStandardPaths>

CSVImporter::CSVImporter(QList<esdbTypeModule *> typeModules,
	QWidget *parent) :
	DatabaseImporter(parent),
	m_parent(parent),
	m_typeModules(typeModules)
{

}

void CSVImporter::failedToOpenCSVDialogFinished(int rc)
{
	Q_UNUSED(rc);
	done(false);
}

struct csvData {
	QString basename;
	QList<QStringList> data;
};

void CSVImporter::start()
{
	QFileDialog *fd = new QFileDialog(m_parent, "CSV Import");
	QStringList filters;
	filters.append("CSV archive (*.zip)");
	filters.append("CSV file (*.csv *.txt)");
	filters.append("*");
	QString documentsPath = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);
	fd->setNameFilters(filters);
	fd->setDirectory(documentsPath);
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

	QList<csvData *> csvDataList;

	for (auto fn : sl) {
		QFile *csvFile = new QFile(fn);
		QFileInfo csvFileInfo(*csvFile);
		QString csvBasename = csvFileInfo.baseName();
		QString csvSuffix = csvFileInfo.suffix();
		if (csvSuffix == "zip") {
			csvFile->deleteLater();
			csvFile = nullptr;
			unzFile unzFile = unzOpen(fn.toLatin1().data());
			if (unzFile == nullptr) {
				//TODO
			}
			if (UNZ_OK != unzGoToFirstFile(unzFile)) {
				//TODO
			}
			while(1) {
				QByteArray filename(256, 0);
				unz_file_info finfo;
				unzGetCurrentFileInfo(unzFile, &finfo,
						filename.data(),
						filename.size(),
						NULL, 0, NULL, 0);
				QFile zipInnerFile(QString::fromLatin1(filename));
				QFileInfo zipInnerFileInfo(zipInnerFile);
				QString csvBasename = zipInnerFileInfo.baseName();

				unzOpenCurrentFile(unzFile);
				QByteArray contents(finfo.uncompressed_size, 0);
				unzReadCurrentFile(unzFile, contents.data(), finfo.uncompressed_size);

				QtCSV::Reader reader;
				QBuffer buffer(&contents);
				csvData *d = new csvData();
				d->basename = csvBasename;
				d->data = reader.readToList(buffer);
				if (d->data.length() < 2) {
					delete d;
					done(false); //TODO
					return;
				}
				csvDataList.push_back(d);

				int rc = unzGoToNextFile(unzFile);
				if (rc == UNZ_END_OF_LIST_OF_FILE) {
					break;
				} else if (rc != UNZ_OK) {
					done(false); //TODO
					return;
				}
			}
		} else {
			QString csvBasename = csvFileInfo.baseName();
			if (!csvFile->open(QFile::ReadOnly)) {
				csvFile->deleteLater();
				auto mb = SignetApplication::messageBoxError(QMessageBox::Warning,
						"CSV Import",
						"Failed to open CSV file",
						m_parent);
				connect(mb, SIGNAL(finished(int)), this, SLOT(failedToOpenCSVDialogFinished(int)));
				return;
			}
			QtCSV::Reader reader;
			csvData *d = new csvData();
			d->basename = csvBasename;
			d->data = reader.readToList(*csvFile);
			csvFile->deleteLater();
			csvFile = nullptr;
			if (d->data.length() < 2) {
				delete d;
				done(false); //TODO
				return;
			} else {
				csvDataList.push_back(d);
			}
		}
	}

	genericTypeDesc *typeDesc = new genericTypeDesc(-1);
	typeDesc->typeId = generic::invalidTypeId;
	auto tempGenericModule = new esdbGenericModule(typeDesc);
#if 0
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
#endif
	m_db = new database();

	for (auto csvData : csvDataList) {
		esdbTypeModule *module = nullptr;
		for (auto m : m_typeModules) {
			if (m->name() == csvData->basename) {
				module = m;
				break;
			}
		}
		if (!module) {
			module = tempGenericModule;
		}
		databaseType *dbType = nullptr;
		auto iter = m_db->find(csvData->basename);
		if (iter != m_db->end()) {
			dbType = *iter;
		} else {
			dbType = new databaseType();
			m_db->insert(csvData->basename, dbType);
		}
		auto csvIter = csvData->data.begin();
		auto header = *(csvIter++);

		for (; csvIter != csvData->data.end(); csvIter++) {
			QVector<genericField> fields;
			QStringList row = *csvIter;
			for (int j = 0; j < row.length() && j < header.length(); j++) {
				fields.append(genericField(header.at(j),"", row.at(j)));
			}
			esdbEntry *ent = module->decodeEntry(fields);
			if (ent) {
				dbType->append(ent);
			}
		}
	}
	done(true);
}
