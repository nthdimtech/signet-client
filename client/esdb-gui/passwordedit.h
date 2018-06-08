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

#include "signetapplication.h"

class PasswordEdit : public QWidget
{
	Q_OBJECT
	DatabaseField *m_passwordField;
	QWidget *m_generateOptions;
	QPushButton *m_generatePassword;
	QSpinBox *m_numGenChars;
	QCheckBox *m_genSymbols;
	QCheckBox *m_hide;
	QDialog *m_generatingDialog;
	QLineEdit *m_genSymbolSet;
	void generatePasswordComplete(QByteArray block);
	int m_signetdevCmdToken;
public:
	explicit PasswordEdit(QString name, QWidget *parent = 0);
	QString password() const;
	void setPassword(const QString &pass);
signals:
	void textEdited(QString);
public slots:
	void generatePassword();
private slots:
	void signetdevGetRandBits(signetdevCmdRespInfo info, QByteArray block);
	void passwordTextEdited(QString str);
	void hideToggled(bool);
};

#endif // PASSWORDEDIT_H
