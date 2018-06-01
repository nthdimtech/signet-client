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

class ButtonWaitDialog;

class cleartextPasswordSelector : public QDialog
{
	Q_OBJECT
	QList<QRadioButton *> m_slotButtons;
	QPushButton *m_openButton;
	int m_signetdevCmdToken;
	int m_index;
	ButtonWaitDialog *m_buttonWaitDialog;
public:
	cleartextPasswordSelector(QVector<int> formats, QStringList names, QWidget *parent = NULL);
private slots:
	void openPressed();
	void selectionPressed();
	void signetdevReadCleartextPassword(struct signetdevCmdRespInfo, cleartext_pass);
	void buttonWaitFinished(int);
};

#endif // CLEARTEXTPASSWORDSELECTOR_H
