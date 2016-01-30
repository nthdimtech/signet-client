#ifndef PASSWORDEDIT_H
#define PASSWORDEDIT_H

#include <QWidget>
#include <QString>

class DatabaseField;
class QPushButton;
class QSpinBox;
class QCheckBox;
class QLineEdit;
class CommThread;

class PasswordEdit : public QWidget
{
	Q_OBJECT
	DatabaseField *m_passwordField;
	QWidget *m_generateOptions;
	QPushButton *m_generatePassword;
	QSpinBox *m_numGenChars;
	QCheckBox *m_genSymbols;
	QLineEdit *m_genSymbolSet;
public:
	explicit PasswordEdit(QWidget *parent = 0);
	QString password() const;
	void setPassword(const QString &pass);
signals:
	void textEdited(QString);
public slots:
	void generate_password();
private slots:
	void passwordTextEdited(QString str);
};

#endif // PASSWORDEDIT_H
