#include "integerfieldedit.h"

#include <QSpinBox>
#include <QString>

integerFieldEdit::integerFieldEdit(const QString &name, bool canRemove) :
	genericFieldEdit(name)
{
	m_spinBoxEdit = new QSpinBox();
	connect(m_spinBoxEdit, SIGNAL(valueChanged(QString)),
		this, SIGNAL(edited()));
	connect(m_spinBoxEdit, SIGNAL(editingFinished()),
		this, SIGNAL(editingFinished()));
	createWidget(canRemove, m_spinBoxEdit);
}

QString integerFieldEdit::toString() const
{
	return m_spinBoxEdit->text();
}

void integerFieldEdit::fromString(const QString &s)
{
	int value = s.toInt();
	m_spinBoxEdit->setValue(value);
}
