#ifndef GENERICFIELDSEDITOR_H
#define GENERICFIELDSEDITOR_H

#include <QObject>
#include <QWidget>
#include <QMap>
#include <QString>

#include "genericfields.h"
#include "generictypedesc.h"

class genericFieldEdit;
class QPushButton;
class QLineEdit;
class QComboBox;

class GenericFieldsEditor : public QWidget
{
	Q_OBJECT
	genericFields &m_fields;
	QList<fieldSpec> m_requiredFieldSpecs;
	QList<genericFieldEdit *> m_requiredFields;
	QList<genericFieldEdit *> m_extraFields;
	QWidget *m_requiredFieldsWidget;
	QWidget *m_extraFieldsWidget;
	QWidget *m_newField;
	QPushButton *m_newFieldAddButton;
	QLineEdit *m_newFieldNameEdit;
	QComboBox *m_newFieldTypeCombo;

	virtual genericFieldEdit *createFieldEdit(QString name, QString type, bool canRemove);
	genericFieldEdit *addNewField(QString name, QString type);
protected:
	QMap<QString, genericFieldEdit *> m_fieldEditMap;
public:
	GenericFieldsEditor(genericFields &fields,
				     QList<fieldSpec> requiredFieldSpecs,
				     QWidget *parent = 0);
	void loadFields();
	void saveFields();
signals:
	void edited();
public slots:
	void addNewFieldUI();
	void newFieldNameEdited(QString);
	void removeField(QString name);
};

#endif // GENERICFIELDSEDITOR_H
