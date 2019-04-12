#ifndef TYPEDESCEDIT_H
#define TYPEDESCEDIT_H

#include "genericfieldedit.h"

class QComboBox;

class typeDescEdit : public genericFieldEdit
{
	QComboBox *m_typeEditCombo;
public:
	typeDescEdit(const QString &name, bool canRemove, QWidget *parent = nullptr);
	QString toString() const;
	void fromString(const QString &s);
	QString type()
	{
		return QString("Text");
	}
};

#endif // TYPEDESCEDIT_H
