#include <Tab.h>

#include <QLoggingCategory>

Q_LOGGING_CATEGORY(tabLog, "tabLog")

QHash<int, QByteArray> Tab::roles = {
    {0, "url"},
    {1, "title"},
    {2, "icon"},
    {3, "viewId"},
    {4, "indent"},
    {5, "view"}};

Tab::Tab(int id_)
{
    id = id_;
}

Tab::Tab(const db::Tabs2::TabInfo& ti)
{
    id = ti.id;
    url = ti.url;
    title = ti.title;
    icon = ti.icon;
}

QVariant Tab::data(int column) const
{
    switch (column)
    {
        case 0: return url;
        case 1: return title;
        case 2: return icon;
        case 3: return id;
        case 4: return indent;
        case 5: return view;
        }
    return id;
}

void Tab::updateIndent()
{
    QStandardItem* p;
    Tab* pp;
    if ((p = parent()) == nullptr ||
        (pp = dynamic_cast<Tab*>(p)) == nullptr)
    {
        indent = 0;
        return;
    }

    indent = pp->indent + 1;
}

int Tab::getId() const
{
    return id;
}

QString Tab::getTitle() const
{
    return title;
}

QString Tab::getIcon() const
{
    return icon;
}

QString Tab::getUrl() const
{
    return url;
}

QVariant Tab::getView() const
{
    return view;
}

void Tab::setTitle(QString title_)
{
    title = title_;
    emitDataChanged();
}

void Tab::setIcon(QString icon_)
{
    icon = icon_;
    emitDataChanged();
}

void Tab::setView(QVariant view_)
{
    view = view_;
}
