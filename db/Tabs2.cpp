#include <db/Tabs2.h>

#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QStringList>
#include <QVariant>

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
        bool result = query.exec("CREATE TABLE IF NOT EXISTS " +
                                 schemaName + "." + tableName +
                                 "(id serial, parent integer default 0,"
                                 "url varchar default \'\',"
                                 "icon varchar default \'\',"
                                 "CONSTRAINT tabs_pkey PRIMARY KEY (id))");

        if (!result)
        {
            qCCritical(dbLogs, "(dbName=%s): failed to create table",
                       dbName.toStdString().c_str());

            logError(query);
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
        qCCritical(dbLogs, "(dbName=%s): failed to create tab",
                   dbName.toStdString().c_str());
        logError(query);
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
        qCCritical(dbLogs, "(dbName=%s, tabId=%i): failed to remove tab",
                   dbName.toStdString().c_str(), tabId);
        logError(query);
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
        qCCritical(dbLogs, "(dbName=%s): failed to fetch tabs",
                   dbName.toStdString().c_str());
        logError(query);
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
        qCCritical(dbLogs, "(dbName=%s, tabId=%i, parentId=%i): "
                   "failed to set parent", dbName.toStdString().c_str(),
                   tabId, parentId);
        logError(query);
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
        qCCritical(dbLogs, "(dbName=%s, tabId=%i, url=%s): "
                   "failed to set url", dbName.toStdString().c_str(),
                   tabId, url.toStdString().c_str());
        logError(query);
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
        qCCritical(dbLogs, "(dbName=%s, tabId=%i, title=%s): "
                   "failed to set title", dbName.toStdString().c_str(),
                   tabId, title.toStdString().c_str());
        logError(query);
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
        qCCritical(dbLogs, "(dbName=%s, tabId=%i, icon=%s): "
                   "failed to set icon", dbName.toStdString().c_str(),
                   tabId, icon.toStdString().c_str());
        logError(query);
        return;
    }
}

}
