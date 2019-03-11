#ifndef GROUPDATABASEFIELD_H
#define GROUPDATABASEFIELD_H

#include "databasefield.h"

class QComboBox;

class GroupDatabaseField : public DatabaseField
{
    Q_OBJECT
    QComboBox *m_groupCombo;
public:
    GroupDatabaseField(int width, QStringList currentGroups, QWidget *parent = nullptr);
    virtual ~GroupDatabaseField();
    virtual QString text() const;
    virtual void setText(const QString &s);
    virtual QLineEdit *getEditWidget();
};

#endif
