#ifndef EDITENTRYDIALOG_H
#define EDITENTRYDIALOG_H

#include "signetapplication.h"

#include <QDialog>

class QPushButton;
class QLabel;
class ButtonWaitDialog;
class QBoxLayout;
struct esdbEntry;
struct block;
struct signetdevCmdRespInfo;

class EditEntryDialog : public QDialog
{
	Q_OBJECT
	QString m_typeName;
	int m_signetdevCmdToken;
	bool m_isOversized;
	QLabel *m_dataOversized;
	void oversizedDialog();
	ButtonWaitDialog *m_buttonWaitDialog;
	QPushButton *m_undoChangesButton;
	QPushButton *m_submitButton;
	bool m_isNew;
	bool m_changesMade;
	bool m_closeOnSave;
	bool m_settingFields;
	QBoxLayout *m_buttons;
	void setupBase();
signals:
	void abort();
	void accountChanged(int id);
	void entryCreated(esdbEntry *);
public:
	EditEntryDialog(QString typeName, int id, QWidget *parent);
	EditEntryDialog(QString typeName, esdbEntry *ent, QWidget *parent);
protected:
	int m_id;
	esdbEntry *m_entry;
	void setup(QLayout *layout);
	void setup(QWidget *widget);
	virtual QString entryName() = 0;
	virtual void applyChanges(esdbEntry *) = 0;
	virtual esdbEntry *createEntry(int id) = 0;
private slots:
	void submitButtonPressed();
	void submitEntryFinished(int code);
public slots:
	void signetdevCmdResp(signetdevCmdRespInfo info);
	void edited();
};

#endif // EDITENTRYDIALOG_H
