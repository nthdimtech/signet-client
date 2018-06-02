#ifndef CLEARTEXTPASSWORDSELECTOR_H
#define CLEARTEXTPASSWORDSELECTOR_H

#include <QObject>
#include <QWidget>
#include <QDialog>
#include <QStringList>
#include <QList>

class QRadioButton;
class QPushButton;
struct cleartext_pass;
struct signetdevCmdRespInfo;

#include "signetapplication.h"

extern "C" {
#include "signetdev/host/signetdev.h"
}

#include <QVector>

class ButtonWaitDialog;

class cleartextPasswordSelector : public QDialog
{
	Q_OBJECT
	QList<QRadioButton *> m_slotButtons;
	QPushButton *m_openButton;
	QPushButton *m_deleteButton;
	QVector<int> m_formats;
	int m_signetdevCmdToken;
	int m_index;
	ButtonWaitDialog *m_buttonWaitDialog;
public:
	cleartextPasswordSelector(QVector<int> formats, QStringList names, QWidget *parent = NULL);
private slots:
	void openPressed();
	void deletePressed();
	void selectionPressed();
	void signetdevReadCleartextPassword(struct signetdevCmdRespInfo, cleartext_pass);
	void signetdevCmdResp(signetdevCmdRespInfo);
	void buttonWaitFinished(int);
};

#endif // CLEARTEXTPASSWORDSELECTOR_H
