#ifndef OPENGENERIC_H
#define OPENGENERIC_H

#include <QDialog>
#include <QString>
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

#include "editentrydialog.h"

class EditGeneric : public EditEntryDialog
{
	Q_OBJECT
	QLineEdit *m_genericNameEdit;
	generic *m_generic;
	genericTypeDesc *m_typeDesc;
	GenericFieldsEditor *m_genericFieldsEditor;

	genericFields m_fields;
	virtual QString entryName();
	virtual void applyChanges(esdbEntry *);
	virtual esdbEntry *createEntry(int id);
	virtual void undoChanges();
	void setup(QString name);
public:
	EditGeneric(genericTypeDesc *typeDesc, int id, QString entryName, QWidget *parent = 0);
	EditGeneric(generic *generic, genericTypeDesc *typeDesc, QWidget *parent = 0);
	virtual ~EditGeneric();
};


#endif // OPENGENERIC_H
