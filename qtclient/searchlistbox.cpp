#include "searchlistbox.h"

#include<QLineEdit>
#include<QTreeView>
#include<QListView>
#include<QAbstractItemModel>
#include<QBoxLayout>
#include<QKeyEvent>
#include <QLineEdit>

SearchListbox::SearchListbox(QLineEdit *query_edit, QAbstractItemModel *model, QWidget *parent) : QListView(parent),
	m_queryEdit(query_edit)
{
	setModel(model);
}

void SearchListbox::enableHover()
{
	setMouseTracking(true);
	connect(this, SIGNAL(entered(QModelIndex)), this, SLOT(entered(QModelIndex)));
}

void SearchListbox::disableHover()
{
	setMouseTracking(false);
	disconnect(this, SLOT(entered(QModelIndex)));
}

void SearchListbox::entered(QModelIndex idx)
{
	setCurrentIndex(idx);
	emit selected(idx);
}

void SearchListbox::keyPressEvent(QKeyEvent *event)
{
	int row;
	QModelIndex idx;
	int key = event->key();
	switch (key) {
	case Qt::Key_Backspace:
		if (m_filterText.size() > 0) {
			m_filterText.resize(m_filterText.size() - 1);
			emit filterTextChanged(m_filterText);
		}
		break;
	case Qt::Key_Up:
		row = this->currentIndex().row();
		idx = model()->index(row - 1, 0);
		if (idx.isValid()) {
			this->setCurrentIndex(idx);
			emit selected(idx);
		}
		break;
	case Qt::Key_Down:
		row = this->currentIndex().row();
		idx = model()->index(row + 1, 0);
		if (idx.isValid()) {
			this->setCurrentIndex(idx);
			emit selected(idx);
		}
		break;
	case Qt::Key_Return:
	case Qt::Key_Enter:
		idx = this->currentIndex();
		if (idx.isValid())
			emit activated(idx);
		break;
	default:
		if (event->text().size()) {
			m_filterText.append(event->text());
			emit filterTextChanged(m_filterText);
		}
	}
}
