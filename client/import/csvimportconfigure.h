#ifndef CSVIMPORTCONFIGURE_H
#define CSVIMPORTCONFIGURE_H

#include <QDialog>
template <typename T> class QSet;
class CSVImporter;
class QComboBox;
struct esdbTypeModule;
class QLabel;
class QLineEdit;

class CSVImportConfigure : public QDialog
{
	Q_OBJECT
	CSVImporter *m_importer;
	QComboBox *m_dataTypeCombo;
	QLineEdit *m_dataTypeEdit;
	esdbTypeModule *m_typeModule;
	QLabel *m_errorLabel;
	QString m_basename;
	int m_selectionIndex;
	const QSet<QString> &m_usedTypeNames;
private slots:
	void currentSelectionIndexChanged(int idx);
	void dataTypeTextEdited();
public:
	QString selectedType(bool &isNew);
	static const int m_skippedResponseCode = QDialog::Accepted + 1;
	CSVImportConfigure(CSVImporter *importer, const QSet<QString> &usedTypeNames, QString basename, QString filename, QWidget *parent = nullptr);
	esdbTypeModule *typeModule()
	{
		return m_typeModule;
	}
private slots:
	void okPressed();
	void skipPressed();
	void cancelPressed();
};

#endif // CSVIMPORTCONFIGURE_H
