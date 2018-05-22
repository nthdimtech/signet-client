#ifndef SEARCHPOPUP_H
#define SEARCHPOPUP_H

#include <QObject>
#include <QTreeView>
#include <QListView>
#include <QModelIndex>

class QLineEdit;
class QListView;
class QAbstractItemModel;

class SearchListbox : public QTreeView
{
	Q_OBJECT
	QLineEdit *m_queryEdit;
	void keyPressEvent(QKeyEvent *event);
	friend class SearchFilterEdit;
	QString m_filterText;
public:
	explicit SearchListbox(QLineEdit *query_edit, QAbstractItemModel *model, QWidget *parent = 0);
	void enableHover();
	void disableHover();
	QString filterText() const
	{
		return m_filterText;
	}

	void setFilterText(QString text)
	{
		m_filterText = text;
	}
signals:
	void filterTextChanged(QString filter_text);
	void selected(QModelIndex idx);
public slots:
	void entered(QModelIndex idx);
};

#endif // SEARCHPOPUP_H
