#include "groupdatabasefield.h"

#include <QComboBox>
#include <QStringList>

GroupDatabaseField::GroupDatabaseField(int width, QStringList currentGroups, QWidget *parent) :
        DatabaseField("Group", parent)
{
	QList<QWidget *> widgets;
	m_groupCombo = new QComboBox();
	m_groupCombo->setEditable(true);
	m_groupCombo->setInsertPolicy(QComboBox::NoInsert);
	m_groupCombo->insertItems(0, currentGroups);
	m_groupCombo->setDuplicatesEnabled(false);
	m_groupCombo->setEnabled(!SignetApplication::get()->isDeviceEmulated());
	m_groupCombo->setSizeAdjustPolicy(QComboBox::AdjustToContents);
	connect(m_groupCombo, SIGNAL(currentTextChanged(QString)), this, SIGNAL(textEdited(QString)));
	m_customEdit = m_groupCombo;
	m_customEdit->setSizePolicy(QSizePolicy(QSizePolicy::Minimum, QSizePolicy::Preferred));
	init(width, widgets, true);
}

GroupDatabaseField::~GroupDatabaseField()
{
}

QString GroupDatabaseField::text() const
{
	return m_groupCombo->currentText();
}

void GroupDatabaseField::setText(const QString &s)
{
	int idx = m_groupCombo->findText(s);
	if (idx != -1) {
		m_groupCombo->setCurrentIndex(idx);
	} else {
		m_groupCombo->setEditText(s);
	}
	return;
}

QLineEdit *GroupDatabaseField::getEditWidget()
{
	return nullptr;
}
