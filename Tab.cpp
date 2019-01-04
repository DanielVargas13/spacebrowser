#include <Tab.h>

Q_LOGGING_CATEGORY(tabModel, "tabModel")

QHash<int, QByteArray> Tab::roles = {
    {0, "url"},
    {1, "title"},
    {2, "icon"},
    {3, "viewId"},
    {4, "indent"}};

Tab::Tab(const db::Tabs::TabInfo& ti)
{
    id = ti.id;
    url = QString(ti.url.c_str());
    title = QString(ti.title.c_str());
    icon = QString(ti.icon.c_str());
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
        }
    return id;
}

void Tab::updateIndent()
{
    QStandardItem* p;
    Tab* pp;
    if ((p = parent()) == nullptr ||
        (pp = dynamic_cast<Tab*>(p)) == nullptr)
        return;

    indent = pp->indent + 1;
    qCDebug(tabModel, "Tab id %i indent: %i", id, indent);
}

void Tab::setId(int id_)
{
    id = id_;
}

void Tab::setTitle(QString title_)
{
    title = title_;
}

void Tab::setIcon(QString icon_)
{
    icon = icon_;
}
