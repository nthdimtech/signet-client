#include "cleartextpasswordeditor.h"

#include "linefieldedit.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>

cleartextPasswordEditor::cleartextPasswordEditor(int index, struct cleartext_pass *p, QWidget *parent) :
	QDialog(parent),
	m_index(index)
{
	setWindowModality(Qt::WindowModal);
	QVBoxLayout *l = new QVBoxLayout();
	m_passwordEdit = new lineFieldEdit(QString::fromUtf8(p->name_utf8), false);
	l->addWidget(m_passwordEdit->widget());
	QPushButton *save = new QPushButton("Save");
	QPushButton *cancel = new QPushButton("Cancel");
	QHBoxLayout *buttons = new QHBoxLayout();
	buttons->addWidget(save);
	buttons->addWidget(cancel);
	l->addLayout(buttons);
	setLayout(l);
}
