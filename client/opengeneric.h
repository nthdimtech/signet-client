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
class GenericFieldsEditor;

struct signetdevCmdRespInfo;

#include "genericfields.h"

class OpenGeneric : public QDialog
{
	Q_OBJECT
	QLineEdit *m_genericNameEdit;
	QPushButton *m_saveButton;
	QPushButton *m_undoChangesButton;
	generic *m_generic;
	genericTypeDesc *m_typeDesc;
	GenericFieldsEditor *m_genericFieldsEditor;

	ButtonWaitDialog *m_buttonWaitDialog;
	genericFields m_fields;
	int m_signetdevCmdToken;
	bool m_settingFields;
	bool m_changesMade;
	bool m_closeOnSave;
	void closeEvent(QCloseEvent *);
public:
	OpenGeneric(generic *generic, genericTypeDesc *typeDesc, QWidget *parent = 0);
	virtual ~OpenGeneric();
signals:
	void abort();
	void accountChanged(int id);
public slots:
	void signetdevCmdResp(signetdevCmdRespInfo info);
	void saveGenericFinished(int);
	void edited();
	void savePressed();
	void undoChangesUI();
};


#endif // OPENGENERIC_H
