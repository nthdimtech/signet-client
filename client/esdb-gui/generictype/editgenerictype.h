#ifndef NEWGENERICTYPE_H
#define NEWGENERICTYPE_H

#include <QDialog>
#include <QString>

class QLineEdit;
class ButtonWaitDialog;
class esdbEntry;

#include "signetapplication.h"
#include "editentrydialog.h"

struct genericTypeDesc;
class GenericFieldsEditor;
class genericFields;

class EditGenericType : public EditEntryDialog
{
	Q_OBJECT
	QLineEdit *m_typeNameEdit;
	QString entryName();
	void applyChanges(esdbEntry *);
	esdbEntry *createEntry(int id);
	void undoChanges();
	genericTypeDesc *m_genericTypeDesc;
	GenericFieldsEditor *m_genericFieldsEditor;
	void copyFromGenericFields(genericTypeDesc *g, const genericFields &gf);
	void copyToGenericFields(const genericTypeDesc *g, genericFields &gf);
	void setup(QString name);
	u16 m_typeId;
public:
	EditGenericType(int id, u16 typeId, const QString &name, QWidget *parent);
	EditGenericType(genericTypeDesc *g, QWidget *parent);
};

#endif // NEWGENERICTYPE_H
