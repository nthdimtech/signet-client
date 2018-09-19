#ifndef NEWGENERICTYPE_H
#define NEWGENERICTYPE_H

#include <QDialog>

class QLineEdit;
class ButtonWaitDialog;
class esdbEntry;

#include "signetapplication.h"
#include "editentrydialog.h"

struct genericTypeDesc;

class EditGenericType : public EditEntryDialog
{
	Q_OBJECT
	QLineEdit *m_typeNameEdit;
	QString entryName();
	void applyChanges(esdbEntry *);
	esdbEntry *createEntry(int id);
	void undoChanges();
	genericTypeDesc *m_genericTypeDesc;
public:
	EditGenericType(int id, const QString &name, QWidget *parent);
};

#endif // NEWGENERICTYPE_H
