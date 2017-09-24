#ifndef NEWGENERIC_H
#define NEWGENERIC_H

#include <QDialog>

class DatabaseField;
class ButtonWaitDialog;
class QLineEdit;

#include "signetapplication.h"

struct signetdevCmdRespInfo;
struct esdbEntry;

struct genericTypeDesc;

class GenericFieldsEditor;

#include "genericfields.h"

class NewGeneric : public QDialog
{
	Q_OBJECT
	ButtonWaitDialog *m_buttonWaitDialog;
	QLineEdit *m_nameField;
	genericTypeDesc *m_typeDesc;
	esdbEntry *m_entry;
	int m_id;
	int m_signetdevCmdToken;
	GenericFieldsEditor *m_genericFieldsEditor;
	genericFields m_genericFields;
public:
	explicit NewGeneric(int id, genericTypeDesc *typeDesc,const QString &name, QWidget *parent = 0);
signals:
	void abort();
	void entryCreated(esdbEntry *);
public slots:
	void signetdevCmdResp(signetdevCmdRespInfo info);
	void createButtonPressed();
	void addEntryFinished(int);
};

#endif // NEWGENERIC_H
