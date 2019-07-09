#include <db/Tabs2.h>

#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QStringList>
#include <QVariant>

namespace db
{

QString Tabs2::tableName("tabs");

Tabs2::Tabs2(DbClient& _dbClient) : dbClient(_dbClient)
{

}

Tabs2::~Tabs2()
{

}

int32_t Tabs2::createTab()
{
    QSqlQuery query(QSqlDatabase::database(dbClient.getDbName()));

    query.prepare("INSERT INTO :schema_name.:table_name DEFAULT VALUES "
        "RETURNING id");
    query.bindValue(":schema_name", dbClient.getSchemaName());
    query.bindValue(":table_name", tableName);

    if (!query.exec())
    {
        qCCritical(dbLogs, "(dbName=%s): failed to create tab",
                   dbClient.getDbName().toStdString().c_str());
        dbClient.logError(query);
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

void Tabs2::closeTab(int tabId)
{
    QSqlQuery query(QSqlDatabase::database(dbClient.getDbName()));

    query.prepare("DELETE FROM :schema_name.:table_name WHERE id = :tab_id");
    query.bindValue(":schema_name", dbClient.getSchemaName());
    query.bindValue(":table_name", tableName);
    query.bindValue(":tab_id", tabId);

    if (!query.exec())
    {
        qCCritical(dbLogs, "(dbName=%s, tabId=%i): failed to remove tab",
                   dbClient.getDbName().toStdString().c_str(), tabId);
        dbClient.logError(query);
    }
}

std::vector<Tabs2::TabInfo> Tabs2::getAllTabs()
{
    QSqlQuery query(QSqlDatabase::database(dbClient.getDbName()));

    query.prepare("SELECT id,parent,url,title,icon"
        "FROM :schema_name.:table_name ORDER BY id");
    query.bindValue(":schema_name", dbClient.getSchemaName());
    query.bindValue(":table_name", tableName);

    if (!query.exec())
    {
        qCCritical(dbLogs, "(dbName=%s): failed to fetch tabs",
                   dbClient.getDbName().toStdString().c_str());
        dbClient.logError(query);
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
/*
std::map<QString, std::vector<Tabs2::TabInfo>> Tabs2::getAllTabs()
{
    std::map<QString, std::vector<Tabs2::TabInfo>> result;

    for (auto& connName: QSqlDatabase::connectionNames())
        result[connName] = getAllTabs(connName);

    return result;
}
*/
void Tabs2::setParent(int tabId, int parentId)
{
    QSqlQuery query(QSqlDatabase::database(dbClient.getDbName()));

    query.prepare("UPDATE :schema_name.:table_name"
        "SET parent=:parent_id WHERE id = :tab_id");
    query.bindValue(":schema_name", dbClient.getSchemaName());
    query.bindValue(":table_name", tableName);
    query.bindValue(":tab_id", tabId);
    query.bindValue(":parent_id", parentId);

    if (!query.exec())
    {
        qCCritical(dbLogs, "(dbName=%s, tabId=%i, parentId=%i): "
                   "failed to set parent", dbClient.getDbName().toStdString().c_str(),
                   tabId, parentId);
        dbClient.logError(query);
        return;
    }
}

void Tabs2::setUrl(int tabId, QString url)
{
    QSqlQuery query(QSqlDatabase::database(dbClient.getDbName()));

    query.prepare("UPDATE :schema_name.:table_name"
        "SET url=:url WHERE id = :tab_id");
    query.bindValue(":schema_name", dbClient.getSchemaName());
    query.bindValue(":table_name", tableName);
    query.bindValue(":tab_id", tabId);
    query.bindValue(":url", url);

    if (!query.exec())
    {
        qCCritical(dbLogs, "(dbName=%s, tabId=%i, url=%s): "
                   "failed to set url", dbClient.getDbName().toStdString().c_str(),
                   tabId, url.toStdString().c_str());
        dbClient.logError(query);
        return;
    }
}

void Tabs2::setTitle(int tabId, QString title)
{
    QSqlQuery query(QSqlDatabase::database(dbClient.getDbName()));

    query.prepare("UPDATE :schema_name.:table_name"
        "SET title=:title WHERE id = :tab_id");
    query.bindValue(":schema_name", dbClient.getSchemaName());
    query.bindValue(":table_name", tableName);
    query.bindValue(":tab_id", tabId);
    query.bindValue(":title", title);

    if (!query.exec())
    {
        qCCritical(dbLogs, "(dbName=%s, tabId=%i, title=%s): "
                   "failed to set title", dbClient.getDbName().toStdString().c_str(),
                   tabId, title.toStdString().c_str());
        dbClient.logError(query);
        return;
    }
}

void Tabs2::setIcon(int tabId, QString icon)
{
    QSqlQuery query(QSqlDatabase::database(dbClient.getDbName()));

    query.prepare("UPDATE :schema_name.:table_name"
        "SET icon=:icon WHERE id = :tab_id");
    query.bindValue(":schema_name", dbClient.getSchemaName());
    query.bindValue(":table_name", tableName);
    query.bindValue(":tab_id", tabId);
    query.bindValue(":icon", icon);

    if (!query.exec())
    {
        qCCritical(dbLogs, "(dbName=%s, tabId=%i, icon=%s): "
                   "failed to set icon", dbClient.getDbName().toStdString().c_str(),
                   tabId, icon.toStdString().c_str());
        dbClient.logError(query);
        return;
    }
}

}
