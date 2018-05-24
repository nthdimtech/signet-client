#ifndef CSVIMPORTCONFIGURE_H
#define CSVIMPORTCONFIGURE_H

#include <QDialog>
class CSVImporter;
class QComboBox;
struct esdbTypeModule;

class CSVImportConfigure : public QDialog
{
	Q_OBJECT
	CSVImporter *m_importer;
	QComboBox *m_dataTypeCombo;
	esdbTypeModule *m_typeModule;
public:
	CSVImportConfigure(CSVImporter *importer, QWidget *parent = NULL);
	esdbTypeModule *typeModule() {
		return m_typeModule;
	}
private slots:
	void okPressed();
	void cancelPressed();
};

#endif // CSVIMPORTCONFIGURE_H
