#ifndef OPENGENERIC_H
#define OPENGENERIC_H

#include <QDialog>
#include "signetapplication.h"

class QLineEdit;
class QPushButton;
class ButtonWaitDialog;
class QString;
class CommThread;
class DatabaseField;
class PasswordEdit;
struct generic;
struct block;
struct genericTypeDesc;

struct signetdevCmdRespInfo;

class OpenGeneric : public QDialog
{
	Q_OBJECT
	QLineEdit *m_genericNameEdit;
	QPushButton *m_saveButton;
	QPushButton *m_undoChangesButton;
	generic *m_generic;
	genericTypeDesc *m_typeDesc;

	QList<DatabaseField *> m_typeFields;
	QList<DatabaseField *> m_extraFields;

	ButtonWaitDialog *m_buttonWaitDialog;
	void setGenericValues();
	int m_signetdevCmdToken;
public:
	OpenGeneric(generic *generic, genericTypeDesc *typeDesc, QWidget *parent = 0);
	virtual ~OpenGeneric();
signals:
	void abort();
	void accountChanged(int id);
public slots:
	void signetdevCmdResp(signetdevCmdRespInfo info);
	void saveGenericFinished(int);
	void textEdited(QString);
	void closePressed();
	void savePressed();
	void undoChangesUI();
};


#endif // OPENGENERIC_H
