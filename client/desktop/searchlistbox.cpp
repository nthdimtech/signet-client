#include "searchlistbox.h"

#include<QLineEdit>
#include<QTreeView>
#include<QListView>
#include<QAbstractItemModel>
#include<QBoxLayout>
#include<QKeyEvent>
#include <QLineEdit>

#include "esdbmodel.h"

SearchListbox::SearchListbox(QLineEdit *query_edit, QAbstractItemModel *model, QWidget *parent) :
	QTreeView(parent),
	m_queryEdit(query_edit)
{
	setIconSize(QSize(24,24));
	setHeaderHidden(true);
	setRootIsDecorated(false);
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
	if (model()->rowCount(idx)) {
		setExpanded(idx, true);
	} else {
		emit selected(idx);
	}
}

void SearchListbox::currentChanged(const QModelIndex &current, const QModelIndex &previous)
{
	Q_UNUSED(previous);
	emit selected(current);
}

void SearchListbox::keyPressEvent(QKeyEvent *event)
{
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
	case Qt::Key_Down:
		QTreeView::keyPressEvent(event);
		break;
	case Qt::Key_Return:
	case Qt::Key_Enter: {
		QModelIndex index = currentIndex();
		if (index.isValid()) {
			if (model()->rowCount(index)) {
				setExpanded(index, !isExpanded(index));
			} else {
				emit activated(index);
			}
		}
	} break;
	default:
		if (event->text().size()) {
			m_filterText.append(event->text());
			emit filterTextChanged(m_filterText);
		}
	}
}
