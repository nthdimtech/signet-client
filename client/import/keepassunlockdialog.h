#ifndef KEEPASSUNLOCKDIALOG_H
#define KEEPASSUNLOCKDIALOG_H

#include <QDialog>

class Database;
class QLineEdit;
class QPushButton;
class QLabel;
class QFile;
class QCheckBox;

class KeePassUnlockDialog : public QDialog
{
	Q_OBJECT
	Database *m_keePassDatabase;
	QCheckBox *m_passwordCheckBox;
	QLineEdit *m_passwordEdit;

	QCheckBox *m_keyFileCheckBox;
	QLineEdit *m_keyPathEdit;
	QPushButton *m_keyPathBrowse;
	QLabel *m_warnLabel;
	QFile *m_databaseFile;
public:
	KeePassUnlockDialog(QFile *file, QWidget *parent);
	Database *database()
	{
		return m_keePassDatabase;
	}
public slots:
	void okayPressed();
	void cancelPressed();
	void keyPathBrowse();
	void passwordTextEdited();
	void keyFileTextEdited();
	void passwordCheckBoxToggled(bool);
	void keyFileCheckBoxToggled(bool);
};

#endif // KEEPASSUNLOCKDIALOG_H
