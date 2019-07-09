#ifndef NEWBOOKMARK_H
#define NEWBOOKMARK_H

#include "editentrydialog.h"

class DatabaseField;
class QLineEdit;


struct esdbEntry;
struct bookmark;

class EditBookmark : public EditEntryDialog
{
	Q_OBJECT
	DatabaseField *m_urlField;
	QLineEdit *m_nameField;
	bookmark *m_bookmark;
	void setup(QString name);
	QString entryName();
	void applyChanges(esdbEntry *e);
	void undoChanges();
	esdbEntry *createEntry(int id);
public:
	EditBookmark(bookmark *bookmark, QWidget *parent = 0);
	EditBookmark(int id, const QString &name, QWidget *parent = 0);
};

#endif // NEWBOOKMARK_H
