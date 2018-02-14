#include "searchfilteredit.h"
#include "searchlistbox.h"

#include<QKeyEvent>

SearchFilterEdit::SearchFilterEdit() :
	QLineEdit(),
	m_searchListBox(NULL)
{
#if (QT_VERSION >= QT_VERSION_CHECK(5, 2, 0))
	setClearButtonEnabled(true);
#endif
}

void SearchFilterEdit::setSearchListbox(SearchListbox *slb)
{
	m_searchListBox = slb;
}

void SearchFilterEdit::keyPressEvent(QKeyEvent *event)
{
	int key = event->key();
	switch (key) {
	case Qt::Key_Up:
	case Qt::Key_Down:
	case Qt::Key_Enter:
	case Qt::Key_Return:
		if (m_searchListBox)
			m_searchListBox->keyPressEvent(event);
		break;
	default:
		QLineEdit::keyPressEvent(event);
	}
}
