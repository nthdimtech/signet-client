#ifndef SEARCHFILTEREDIT_H
#define SEARCHFILTEREDIT_H

#include <QLineEdit>

class SearchListbox;

class SearchFilterEdit : public QLineEdit
{
	SearchListbox *m_searchListBox;
protected:
	void keyPressEvent(QKeyEvent *event);
public:
	SearchFilterEdit();
	void setSearchListbox(SearchListbox *slb);
};

#endif // SEARCHFILTEREDIT_H
