#include "integerfieldedit.h"

#include <QSpinBox>
#include <QString>

integerFieldEdit::integerFieldEdit(const QString &name, bool canRemove, QWidget *parent) :
        genericFieldEdit(name, parent)
{
	m_spinBoxEdit = new QSpinBox();
	m_spinBoxEdit->setReadOnly(SignetApplication::get()->isDeviceEmulated());
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
