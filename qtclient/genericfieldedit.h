#ifndef GENERICFIELDEDIT_H
#define GENERICFIELDEDIT_H

#include "signetapplication.h"

#include <QString>
#include <QObject>

class QWidget;
class QPushButton;
class ButtonWaitDialog;

class genericFieldEdit : public QObject
{
	Q_OBJECT
	QString m_name;
	QWidget *m_widget;
	QPushButton *m_copyButton;
	QPushButton *m_typeButton;
	QPushButton *m_deleteButton;
	QWidget *m_editWidget;
	ButtonWaitDialog *m_buttonWait;
protected:
	void createWidget(bool canRemove, QWidget *editWidget, bool outputEnable = true);
	void createTallWidget(int rows, bool canRemove, QWidget *editWidget);
public:
	genericFieldEdit(const QString &name);

	virtual QString toString() const = 0;

	virtual QString type() = 0;

	QString name() const {
		return m_name;
	}

	virtual void fromString(const QString &s) = 0;

	QWidget *widget() const {
		return m_widget;
	}

	QWidget *editWidget() const {
		return m_editWidget;
	}

	virtual ~genericFieldEdit() {
	}

	int m_signetdevCmdToken;

public slots:
	void deletePressed();
	void typePressed();
	void copyPressed();
	void typeFieldFinished(int);
	void signetdevCmdResp(signetdevCmdRespInfo info);
signals:
	void edited();
	void editingFinished();
	void remove(QString);
};

#endif // GENERICFIELDEDIT_H
