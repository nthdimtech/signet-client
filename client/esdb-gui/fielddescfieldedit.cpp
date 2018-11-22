#include "fielddescfieldedit.h"

#include <QLabel>

fieldDescFieldEdit::fieldDescFieldEdit(const QString &name) :
	genericFieldEdit(name)
{
	auto l = new QLabel(name);
	createWidget(false, l);
}

QString fieldDescFieldEdit::toString() const
{
	return QString();
}

QString fieldDescFieldEdit::type()
{
	return QString();
}

void fieldDescFieldEdit::fromString(const QString &str)
{
	Q_UNUSED(str);
	//TODO
	return;
}
