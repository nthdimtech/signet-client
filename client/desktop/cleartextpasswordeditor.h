#ifndef CLEARTEXTPASSWORDEDITOR_H
#define CLEARTEXTPASSWORDEDITOR_H

#include <QObject>
#include <QWidget>
#include <QDialog>

class lineFieldEdit;

class cleartextPasswordEditor : public QDialog
{
	Q_OBJECT
	lineFieldEdit *m_passwordEdit;
	int m_index;
public:
	cleartextPasswordEditor(int index, struct cleartext_pass *p, QWidget *parent = NULL);
};

#endif // CLEARTEXTPASSWORDEDITOR_H
