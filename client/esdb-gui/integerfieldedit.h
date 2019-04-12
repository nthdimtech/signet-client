#ifndef INTEGERFIELDEDIT_H
#define INTEGERFIELDEDIT_H

#include "genericfieldedit.h"

class QSpinBox;

class integerFieldEdit : public genericFieldEdit
{
	QSpinBox *m_spinBoxEdit;
public:
	integerFieldEdit(const QString &name, bool canRemove, QWidget *parent = nullptr);
	QString toString() const;
	void fromString(const QString &s);
	QString type()
	{
		return QString("Integer");
	}
};

#endif // INTEGERFIELDEDIT_H
