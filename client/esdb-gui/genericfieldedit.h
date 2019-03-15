#ifndef GENERICFIELDEDIT_H
#define GENERICFIELDEDIT_H

#include "signetapplication.h"

#include <QString>
#include <QObject>

class QWidget;
class QPushButton;
class ButtonWaitDialog;
class QCheckBox;

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
	QVector<u16> m_keysToType;
	QCheckBox *m_secretCheckbox;
protected:
	void createWidget(bool canRemove, QWidget *editWidget, bool outputEnable = true);
	void createTallWidget(int rows, bool canRemove, QWidget *editWidget);
	virtual void showContent()
	{

	}
	virtual void hideContent()
	{

	}
public:
	genericFieldEdit(const QString &name);

	virtual QString toString() const = 0;

	virtual QString type() = 0;

	QString name() const
	{
		return m_name;
	}

	QString displayName() const
	{
		if (m_name[0] == '.') {
			QString nameCpy = m_name;
			return nameCpy.remove(0,1);
		} else {
			return m_name;
		}
	}

	bool isSecretField() const
	{
		return (m_name[0] == '.');
	}

	virtual void fromString(const QString &s) = 0;

	QWidget *widget() const
	{
		return m_widget;
	}

	QWidget *editWidget() const
	{
		return m_editWidget;
	}

	virtual void setFocus()
	{
		m_editWidget->setFocus();
	}

	virtual ~genericFieldEdit()
	{
	}

	int m_signetdevCmdToken;

public slots:
	void deletePressed();
	void typePressed();
	void copyPressed();
	void typeFieldFinished(int);
	void signetdevCmdResp(signetdevCmdRespInfo info);
	void secretCheckStateChanged(int state);
signals:
	void edited();
	void editingFinished();
	void remove(QString);
};

#endif // GENERICFIELDEDIT_H
