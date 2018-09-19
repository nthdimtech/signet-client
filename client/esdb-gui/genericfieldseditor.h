#ifndef GENERICFIELDSEDITOR_H
#define GENERICFIELDSEDITOR_H

#include <QObject>
#include <QWidget>
#include <QMap>
#include <QString>
#include <QDialog>
#include "genericfields.h"
#include "generictypedesc.h"

class genericFieldEdit;
class QPushButton;
class QLineEdit;
class QComboBox;
class QFrame;

class GenericFieldsEditor : public QWidget
{
	Q_OBJECT
	QList<fieldSpec> m_requiredFieldSpecs;
	QMap<QString, genericFieldEdit *> m_extraFields;
	QWidget *m_requiredFieldsWidget;
	QWidget *m_extraFieldsWidget;
	QDialog *m_newField;
	QFrame *m_fieldFrame;
	QFrame *m_newFieldFrame;
	QPushButton *m_newFieldAddButton;
	QLineEdit *m_newFieldNameEdit;
	QComboBox *m_newFieldTypeCombo;

	virtual genericFieldEdit *createFieldEdit(QString name, QString type, bool canRemove);
	genericFieldEdit *addNewField(QString name, QString type);
protected:
	QMap<QString, genericFieldEdit *> m_fieldEditMap;
public:
	GenericFieldsEditor(QList<fieldSpec> requiredFieldSpecs,
				     QWidget *parent = 0);
	void loadFields(genericFields &fields);
	void saveFields(genericFields &fields);
signals:
	void edited();
public slots:
	void addNewFieldUI();
	void newFieldNameEdited(QString);
	void removeField(QString name);
};

#endif // GENERICFIELDSEDITOR_H
