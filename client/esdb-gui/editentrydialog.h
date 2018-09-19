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
	QBoxLayout *m_buttons;
	void setupBase();
	void closeEvent(QCloseEvent *event);
signals:
	void abort();
	void accountChanged(int id);
	void entryCreated(esdbEntry *);
public:
	EditEntryDialog(QString typeName, int id, QWidget *parent);
	EditEntryDialog(QString typeName, esdbEntry *ent, QWidget *parent);
protected:
	int m_id;
	bool m_settingFields;
	esdbEntry *m_entry;
	void setup(QLayout *layout);
	void setup(QWidget *widget);
	virtual QString entryName() = 0;
	virtual void applyChanges(esdbEntry *) = 0;
	virtual esdbEntry *createEntry(int id) = 0;
	virtual void undoChanges() = 0;
private slots:
	void submitButtonPressed();
	void undoButtonPressed();
	void submitEntryFinished(int code);
	void signetdevCmdResp(signetdevCmdRespInfo info);
public slots:
	void edited();
	void entryNameEdited();
};

#endif // EDITENTRYDIALOG_H
