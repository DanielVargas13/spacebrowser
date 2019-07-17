#include <db/Tabs2.h>

#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QStringList>
#include <QVariant>

#include <future>

namespace db
{

QString Tabs2::tableName("tabs");

Tabs2::Tabs2(DbClient& _dbClient, Backend& _backend) :
    dbClient(_dbClient), backend(_backend)
{

}

Tabs2::~Tabs2()
{

}

int32_t Tabs2::createTab()
{
    Backend::funRet_t result = backend.performQuery(
        [this]()->Backend::funRet_t
        {
            QSqlQuery query(QSqlDatabase::database(dbClient.getDbName()));

            query.prepare(QString("INSERT INTO %1.%2 DEFAULT VALUES "
                                  "RETURNING id")
                          .arg(dbClient.getSchemaName())
                          .arg(tableName));

            if (!query.exec() || !query.first())
            {
                qCCritical(dbLogs, "(dbName=%s): failed to create tab",
                           dbClient.getDbName().toStdString().c_str());
                dbClient.logError(query);
            }

            return std::move(query);
        }).get();

    /// Retrieve id of newly created tab entry
    ///
    QSqlQuery query = std::get<QSqlQuery>(result);
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
    Backend::funRet_t result = backend.performQuery(
        [this, tabId]()->Backend::funRet_t
        {
            QSqlQuery query(QSqlDatabase::database(dbClient.getDbName()));

            query.prepare(QString("DELETE FROM %1.%2 WHERE id = :tab_id")
                          .arg(dbClient.getSchemaName())
                          .arg(tableName));
            query.bindValue(":tab_id", tabId);

            if (!query.exec())
            {
                qCCritical(dbLogs, "(dbName=%s, tabId=%i): failed to remove tab",
                           dbClient.getDbName().toStdString().c_str(), tabId);
                dbClient.logError(query);
                return false;
            }

            return true;
        }).get();
}

std::vector<Tabs2::TabInfo> Tabs2::getAllTabs()
{
    Backend::funRet_t result = backend.performQuery(
        [this]()->Backend::funRet_t
        {
            QSqlQuery query(QSqlDatabase::database(dbClient.getDbName()));

            query.prepare(QString("SELECT id,parent,url,title,icon "
                                  "FROM %1.%2 ORDER BY id")
                          .arg(dbClient.getSchemaName())
                          .arg(tableName));

            if (!query.exec())
            {
                qCCritical(dbLogs, "(dbName=%s): failed to fetch tabs",
                           dbClient.getDbName().toStdString().c_str());
                dbClient.logError(query);
                throw std::runtime_error("Failed to fetch tabs");
            }

            return std::move(query);
        }).get();

    std::vector<TabInfo> tabs;

    QSqlQuery query = std::get<QSqlQuery>(result);

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

std::map<int, Tabs2::TabInfo> Tabs2::getAllTabsMap()
{
    Backend::funRet_t result = backend.performQuery(
        [this]()->Backend::funRet_t
        {
            QSqlQuery query(QSqlDatabase::database(dbClient.getDbName()));

            query.prepare(QString("SELECT id,parent,url,title,icon "
                                  "FROM %1.%2 ORDER BY id")
                          .arg(dbClient.getSchemaName())
                          .arg(tableName));
            
            if (!query.exec())
            {
                qCCritical(dbLogs, "(dbName=%s): failed to fetch tabs",
                           dbClient.getDbName().toStdString().c_str());
                dbClient.logError(query);
                throw std::runtime_error("Failed to fetch tabs");
            }

            return std::move(query);
        }).get();

    QSqlQuery query = std::get<QSqlQuery>(result);

    std::map<int, TabInfo> tabs;

    while (query.next())
    {
        TabInfo tab;
        tab.id = query.value("id").toInt();
        tab.parent = query.value("parent").toInt();
        tab.url = query.value("url").toString();
        tab.title = query.value("title").toString();
        tab.icon = query.value("icon").toString();

        tabs[tab.id] = tab;
    }

    for (auto& tab: tabs)
        tabs[tab.second.parent].children.push_back(tab.second.id);


    qCDebug(dbLogs, "(dbName=%s): loaded %lu tabs from db",
            dbClient.getDbName().toStdString().c_str(),
            tabs.size());

    return tabs;
}

void Tabs2::setParent(int tabId, int parentId)
{
    Backend::funRet_t result = backend.performQuery(
        [this, tabId, parentId]()->Backend::funRet_t
        {
            QSqlQuery query(QSqlDatabase::database(dbClient.getDbName()));

            query.prepare(QString("UPDATE %1.%2 "
                                  "SET parent=:parent_id WHERE id = :tab_id")
                          .arg(dbClient.getSchemaName())
                          .arg(tableName));
            query.bindValue(":tab_id", tabId);
            query.bindValue(":parent_id", parentId);

            if (!query.exec())
            {
                qCCritical(dbLogs, "(dbName=%s, tabId=%i, parentId=%i): "
                           "failed to set parent", dbClient.getDbName().toStdString().c_str(),
                           tabId, parentId);
                dbClient.logError(query);
                return false;
            }

            return true;
        }).get();
}

void Tabs2::setUrl(int tabId, QString url)
{
    Backend::funRet_t result = backend.performQuery(
        [this, tabId, url]()->Backend::funRet_t
        {
            QSqlQuery query(QSqlDatabase::database(dbClient.getDbName()));

            query.prepare(QString("UPDATE %1.%2 "
                                  "SET url=:url WHERE id = :tab_id")
                          .arg(dbClient.getSchemaName())
                          .arg(tableName));
            query.bindValue(":tab_id", tabId);
            query.bindValue(":url", url);

            if (!query.exec())
            {
                qCCritical(dbLogs, "(dbName=%s, tabId=%i, url=%s): "
                           "failed to set url", dbClient.getDbName().toStdString().c_str(),
                           tabId, url.toStdString().c_str());
                dbClient.logError(query);
                return false;
            }

            return true;
        }).get();
}

void Tabs2::setTitle(int tabId, QString title)
{
    Backend::funRet_t result = backend.performQuery(
        [this, tabId, title]()->Backend::funRet_t
        {
            QSqlQuery query(QSqlDatabase::database(dbClient.getDbName()));
            
            query.prepare(QString("UPDATE %1.%2 "
                                  "SET title=:title WHERE id = :tab_id")
                          .arg(dbClient.getSchemaName())
                          .arg(tableName));
            query.bindValue(":tab_id", tabId);
            query.bindValue(":title", title);
            
            if (!query.exec())
            {
                qCCritical(dbLogs, "(dbName=%s, tabId=%i, title=%s): "
                           "failed to set title", dbClient.getDbName().toStdString().c_str(),
                           tabId, title.toStdString().c_str());
                dbClient.logError(query);
                return false;
            }

            return true;
        }).get();
}


void Tabs2::setIcon(int tabId, QString icon)
{
    Backend::funRet_t result = backend.performQuery(
        [this, tabId, icon]()->Backend::funRet_t
        {
            QSqlQuery query(QSqlDatabase::database(dbClient.getDbName()));
            
            query.prepare(QString("UPDATE %1.%2 "
                                  "SET icon=:icon WHERE id = :tab_id")
                          .arg(dbClient.getSchemaName())
                          .arg(tableName));
            query.bindValue(":tab_id", tabId);
            query.bindValue(":icon", icon);

            if (!query.exec())
            {
                qCCritical(dbLogs, "(dbName=%s, tabId=%i, icon=%s): "
                           "failed to set icon", dbClient.getDbName().toStdString().c_str(),
                           tabId, icon.toStdString().c_str());
                dbClient.logError(query);
                return false;
            }

            return true;
        }).get();
}

}
