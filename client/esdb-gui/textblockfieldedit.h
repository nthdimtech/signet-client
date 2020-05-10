#ifndef TEXTBLOCKFIELDEDIT_H
#define TEXTBLOCKFIELDEDIT_H

#include "genericfieldedit.h"

class QTextEdit;

class textBlockFieldEdit : public genericFieldEdit
{
	QTextEdit *m_textEdit;
public:
	textBlockFieldEdit(const QString &name, bool canRemove, bool secretField, QWidget *parent = nullptr);
	QString toString() const;
	void fromString(const QString &s);
	QString type()
	{
		return QString("Text block");
	}
};

#endif // TEXTBLOCKFIELDEDIT_H
