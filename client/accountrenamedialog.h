#ifndef ACCOUNTRENAMEDIALOG_H
#define ACCOUNTRENAMEDIALOG_H

#include <QObject>
#include <QWidget>
#include <QDialog>

class QLabel;
class QLineEdit;

class AccountRenameDialog : public QDialog
{
	Q_OBJECT

	bool m_okayPressed;
	QLabel *m_warningLabel;
	QLineEdit *m_newNameEdit;
public:
	AccountRenameDialog(QString initialName, QWidget *parent = 0);
	QString newName();
	bool isOkayPressed() {
		return m_okayPressed;
	}
public slots:
	void okayPressed();
	void cancelPressed();
	void textEdited(QString);
};

#endif // ACCOUNTRENAMEDIALOG_H
