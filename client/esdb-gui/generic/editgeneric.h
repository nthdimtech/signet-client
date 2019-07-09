#ifndef OPENGENERIC_H
#define OPENGENERIC_H

#include <QDialog>
#include <QString>
#include <QStringList>
#include "signetapplication.h"

class QLineEdit;
class QPushButton;
class QString;
class CommThread;
class DatabaseField;
class PasswordEdit;
struct generic;
struct block;
struct genericTypeDesc;
class GenericFieldsEditor;
class GroupDatabaseField;
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
	QStringList m_groupList;
	GroupDatabaseField *m_groupField;
	genericFields m_fields;
	virtual QString entryName();
	virtual void applyChanges(esdbEntry *);
	virtual esdbEntry *createEntry(int id);
	virtual void undoChanges();
	void setup(QString name);
	void setValues();
public:
	EditGeneric(genericTypeDesc *typeDesc, int id, QString entryName, QStringList groupList, QWidget *parent = 0);
	EditGeneric(generic *generic, genericTypeDesc *typeDesc, QStringList groupList, QWidget *parent = 0);
	virtual ~EditGeneric();
};


#endif // OPENGENERIC_H
