#ifndef PASSIMPORTUNLOCKDIALOG_H
#define PASSIMPORTUNLOCKDIALOG_H

#include <QObject>
#include <QWidget>
#include <QDialog>

class PassImporter;
class QLabel;
class QLineEdit;

class PassImportUnlockDialog : public QDialog
{
	Q_OBJECT
	PassImporter *m_importer;
	QLabel *m_warnLabel;
	QLineEdit *m_passphraseEdit;
public:
	PassImportUnlockDialog(PassImporter *importer, QWidget *parent);
	QString passphrase();
public slots:
	void okayPressed();
	void cancelPressed();
	void passphraseTextEdited();
};

#endif // PASSIMPORTUNLOCKDIALOG_H
