#ifndef NEWBOOKMARK_H
#define NEWBOOKMARK_H

#include <QDialog>

class DatabaseField;
class ButtonWaitDialog;
class QLineEdit;

#include "signetapplication.h"

struct signetdevCmdRespInfo;
struct esdbEntry;

class NewBookmark : public QDialog
{
	Q_OBJECT
	DatabaseField *m_urlField;
	ButtonWaitDialog *m_buttonWaitDialog;
	QLineEdit *m_nameField;
	esdbEntry *m_entry;
	int m_id;
	int m_signetdevCmdToken;
public:
	explicit NewBookmark(int id, const QString &name, QWidget *parent = 0);
signals:
	void abort();
	void entryCreated(esdbEntry *);
public slots:
	void signetdevCmdResp(signetdevCmdRespInfo info);
	void createButtonPressed();
	void addEntryFinished(int);
};

#endif // NEWBOOKMARK_H
