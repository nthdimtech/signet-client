#ifndef LINEFIELDEDIT_H
#define LINEFIELDEDIT_H

#include "genericfieldedit.h"
#include <QString>

class QLineEdit;
class QWidget;
class QPushButton;

class lineFieldEdit : public genericFieldEdit
{
	QLineEdit *m_lineEdit;
	void showContent();
	void hideContent();
public:
	lineFieldEdit(const QString &name, bool canRemove);
	QString toString() const;
	void fromString(const QString &s);
	QString type()
	{
		return QString("Text");
	}

};

#endif // LINEFIELDEDIT_H
