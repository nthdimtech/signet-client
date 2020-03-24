#include "editbookmark.h"

#include <QBoxLayout>
#include <QLineEdit>
#include <QLabel>
#include <QPushButton>

#include "databasefield.h"
#include "buttonwaitdialog.h"

extern "C" {
#include "signetdev/host/signetdev.h"
}

#include "bookmark.h"

#include "signetapplication.h"
#include "style.h"

EditBookmark::EditBookmark(int id, const QString &name, QWidget *parent) :
	EditEntryDialog("Bookmark", id, parent),
	m_bookmark(NULL)
{
	setup(name);
}


EditBookmark::EditBookmark(bookmark *bookmark, QWidget *parent) :
	EditEntryDialog("Bookmark", bookmark, parent),
	m_bookmark(bookmark)
{
	setup(bookmark->name);
	m_settingFields = true;
	m_urlField->setText(bookmark->url);
	m_settingFields = false;
}

void EditBookmark::setup(QString name)
{
	QBoxLayout *nameLayout = new QBoxLayout(QBoxLayout::LeftToRight);
	m_nameField = new QLineEdit(name);
	m_nameField->setReadOnly(SignetApplication::get()->isDeviceEmulated());

	nameLayout->addWidget(new genericText("Name"));
	nameLayout->addWidget(m_nameField);

	m_urlField = new DatabaseField("URL", 140);

	QBoxLayout *layout = new QBoxLayout(QBoxLayout::TopToBottom);
	layout->setAlignment(Qt::AlignTop);
	layout->addLayout(nameLayout);
	layout->addWidget(m_urlField);
	EditEntryDialog::setup(layout);

	connect(m_urlField, SIGNAL(textEdited(QString)), this, SLOT(edited()));
	connect(m_nameField, SIGNAL(textEdited(QString)), this, SLOT(edited()));
	connect(m_nameField, SIGNAL(textEdited(QString)), this, SLOT(entryNameEdited()));
}

QString EditBookmark::entryName()
{
	return m_nameField->text();
}

void EditBookmark::applyChanges(esdbEntry *e)
{
	bookmark *b = static_cast<bookmark *>(e);
	b->name = entryName();
	b->url = m_urlField->text();
}

void EditBookmark::undoChanges()
{
	if (m_bookmark) {
		m_nameField->setText(m_bookmark->name);
		m_urlField->setText(m_bookmark->url);
	}
}

esdbEntry *EditBookmark::createEntry(int id)
{
	return new bookmark(id);
}
