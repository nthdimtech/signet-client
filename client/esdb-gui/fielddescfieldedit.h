#ifndef FIELDDESCFIELDEDIT_H
#define FIELDDESCFIELDEDIT_H

#include "genericfieldedit.h"

class fieldDescFieldEdit : public genericFieldEdit
{
public:
	fieldDescFieldEdit(const QString &name);
private:
	QString toString() const;
	QString type();
	void fromString(const QString &str);
};

#endif // FIELDDESCFIELDEDIT_H
