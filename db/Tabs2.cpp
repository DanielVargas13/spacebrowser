#include <db/Tabs2.h>

#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QStringList>
#include <QVariant>

#include <iostream>

namespace db
{

QString Tabs2::tableName("tabs");

Tabs2::Tabs2()
{

}

Tabs2::~Tabs2()
{

}

bool Tabs2::initDatabase(QString dbName)
{
    /// Perform common db initialization
    ///
    if (!DbClient::initDatabase(dbName))
        return false;

    /// Make sure table exists
    ///
    QSqlDatabase db = QSqlDatabase::database(dbName);

    if (!db.tables().contains(schemaName + "." + tableName))
    {
        QSqlQuery query(db);
        query.prepare("CREATE TABLE IF NOT EXISTS :schema_name.:table_name"
            "(id serial, parent integer default 0,"
            "url varchar default \'\', icon varchar default \'\',"
            "CONSTRAINT tabs_pkey PRIMARY KEY (id))");
        query.bindValue(":schema_name", schemaName);
        query.bindValue(":table_name", tableName);

        if (!query.exec())
        {
            std::cout << "Tabs2::initDatabase(dbName=" << dbName.toStdString()
                << "): failed to create table\n";
            std::cout << query.lastError().text().toStdString() << std::endl;
            return false;
        }
    }

    return true;
}

int32_t Tabs2::createTab(QString dbName)
{
    QSqlQuery query(QSqlDatabase::database(dbName));

    query.prepare("INSERT INTO :schema_name.:table_name DEFAULT VALUES "
        "RETURNING id");
    query.bindValue(":schema_name", schemaName);
    query.bindValue(":table_name", tableName);

    if (!query.exec())
    {
        std::cout << "Tabs2::createTab(dbName=" << dbName.toStdString()
            << "): failed to create tab\n";
        std::cout << query.lastError().text().toStdString() << std::endl;
        return -1;
    }

    /// Retrieve id of newly created tab entry
    ///
    QVariant id = query.value("id");
    bool ok = false;
    if (id.isValid() && id.toInt(&ok) >= 0 && ok)
        return id.toInt();
    else
    {
        throw std::runtime_error("Tabs2::createTab(): "
            "failed to parse db answer");
    }
}

void Tabs2::closeTab(QString dbName, int tabId)
{
    QSqlQuery query(QSqlDatabase::database(dbName));

    query.prepare("DELETE FROM :schema_name.:table_name WHERE id = :tab_id");
    query.bindValue(":schema_name", schemaName);
    query.bindValue(":table_name", tableName);
    query.bindValue(":tab_id", tabId);

    if (!query.exec())
    {
        std::cout << "Tabs2::closeTab(dbName=" << dbName.toStdString()
            << ", tabId=" << tabId << "): failed to remove tab\n";
        std::cout << query.lastError().text().toStdString() << std::endl;
    }
}

std::vector<Tabs2::TabInfo> Tabs2::getAllTabs(QString dbName)
{
    QSqlQuery query(QSqlDatabase::database(dbName));

    query.prepare("SELECT id,parent,url,title,icon"
        "FROM :schema_name.:table_name ORDER BY id");
    query.bindValue(":schema_name", schemaName);
    query.bindValue(":table_name", tableName);

    if (!query.exec())
    {
        std::cout << "Tabs2::getAllTabs(dbName=" << dbName.toStdString()
            << "): failed to fetch tabs\n";
        std::cout << query.lastError().text().toStdString() << std::endl;

        throw std::runtime_error("Failed to fetch tabs");
    }

    std::vector<TabInfo> tabs;

    while (query.next())
    {
        TabInfo tab;
        tab.id = query.value("id").toInt();
        tab.parent = query.value("parent").toInt();
        tab.url = query.value("url").toString();
        tab.title = query.value("title").toString();
        tab.icon = query.value("icon").toString();

        tabs.push_back(tab);
    }

    return tabs;
}

std::map<QString, std::vector<Tabs2::TabInfo>> Tabs2::getAllTabs()
{
    std::map<QString, std::vector<Tabs2::TabInfo>> result;

    for (auto& connName: QSqlDatabase::connectionNames())
        result[connName] = getAllTabs(connName);

    return result;
}

void Tabs2::setParent(QString dbName, int tabId, int parentId)
{
    QSqlQuery query(QSqlDatabase::database(dbName));

    query.prepare("UPDATE :schema_name.:table_name"
        "SET parent=:parent_id WHERE id = :tab_id");
    query.bindValue(":schema_name", schemaName);
    query.bindValue(":table_name", tableName);
    query.bindValue(":tab_id", tabId);
    query.bindValue(":parent_id", parentId);

    if (!query.exec())
    {
        std::cout << "Tabs2::setParent(dbName=" << dbName.toStdString()
            << ", tabId:" << tabId << ", parentId:" << parentId
            << "): failed to set parent\n";
        std::cout << query.lastError().text().toStdString() << std::endl;
        return;
    }
}

void Tabs2::setUrl(QString dbName, int tabId, QString url)
{
    QSqlQuery query(QSqlDatabase::database(dbName));

    query.prepare("UPDATE :schema_name.:table_name"
        "SET url=:url WHERE id = :tab_id");
    query.bindValue(":schema_name", schemaName);
    query.bindValue(":table_name", tableName);
    query.bindValue(":tab_id", tabId);
    query.bindValue(":url", url);

    if (!query.exec())
    {
        std::cout << "Tabs2::setUrl(dbName=" << dbName.toStdString()
            << ", tabId:" << tabId << ", url:" << url.toStdString()
            << "): failed to set url\n";
        std::cout << query.lastError().text().toStdString() << std::endl;
        return;
    }
}

void Tabs2::setTitle(QString dbName, int tabId, QString title)
{
    QSqlQuery query(QSqlDatabase::database(dbName));

    query.prepare("UPDATE :schema_name.:table_name"
        "SET title=:title WHERE id = :tab_id");
    query.bindValue(":schema_name", schemaName);
    query.bindValue(":table_name", tableName);
    query.bindValue(":tab_id", tabId);
    query.bindValue(":title", title);

    if (!query.exec())
    {
        std::cout << "Tabs2::setParent(dbName=" << dbName.toStdString()
            << ", tabId:" << tabId << ", title:" << title.toStdString()
            << "): failed to set parent\n";
        std::cout << query.lastError().text().toStdString() << std::endl;
        return;
    }
}

void Tabs2::setIcon(QString dbName, int tabId, QString icon)
{
    QSqlQuery query(QSqlDatabase::database(dbName));

    query.prepare("UPDATE :schema_name.:table_name"
        "SET icon=:icon WHERE id = :tab_id");
    query.bindValue(":schema_name", schemaName);
    query.bindValue(":table_name", tableName);
    query.bindValue(":tab_id", tabId);
    query.bindValue(":icon", icon);

    if (!query.exec())
    {
        std::cout << "Tabs2::setParent(dbName=" << dbName.toStdString()
            << ", tabId:" << tabId << ", icon:" << icon.toStdString()
            << "): failed to set parent\n";
        std::cout << query.lastError().text().toStdString() << std::endl;
        return;
    }
}

}
