#include <db/Tabs2.h>

#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QStringList>
#include <QVariant>

#include <chrono>
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

//            return std::move(query);
            return query.value("id");
        }).get();

    /// Retrieve id of newly created tab entry
    ///
    //QSqlQuery query = std::get<QSqlQuery>(result);
    //QVariant id = query.value("id");
    QVariant id = std::get<QVariant>(result);
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
    //Backend::funRet_t result = backend.performQuery(
    backend.performQueryNR(
        [this, tabId]()->void //Backend::funRet_t
        {
            qCDebug(dbLogs, "(dbName=%s) closeTab: start", dbClient.getDbName().toStdString().c_str());
            auto start = std::chrono::system_clock::now();

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
                //return false;
            }
            auto end = std::chrono::system_clock::now();
            std::chrono::duration<double> total = end-start;
            qCDebug(dbLogs, "(dbName=%s) closeTab: end; time: %lli ms",
                    dbClient.getDbName().toStdString().c_str(),
                    std::chrono::duration_cast<std::chrono::milliseconds>(total));

            //return true;
        }); //.get();
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

            //return std::move(query);

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

            return QVariant::fromValue<std::vector<TabInfo>>(std::move(tabs));

        }).get();

//    std::vector<TabInfo> tabs;

//    QSqlQuery query = std::get<QSqlQuery>(result);
/*
    while (query.next())
    {
        TabInfo tab;
        tab.id = query.value("id").toInt();
        tab.parent = query.value("parent").toInt();
        tab.url = query.value("url").toString();
        tab.title = query.value("title").toString();
        tab.icon = query.value("icon").toString();

        tabs.push_back(tab);
        }*/
    QVariant res = std::get<QVariant>(result);
    std::vector<TabInfo> tabs = res.value<std::vector<TabInfo>>();

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

            //return std::move(query);

            return QVariant::fromValue<tabMap_t>(std::move(tabs));
        }).get();

//    QSqlQuery query = std::get<QSqlQuery>(result);
    QVariant res = std::get<QVariant>(result);
    std::map<int, Tabs2::TabInfo> tabs = res.value<std::map<int, Tabs2::TabInfo>>();


    qCDebug(dbLogs, "(dbName=%s): loaded %lu tabs from db",
            dbClient.getDbName().toStdString().c_str(),
            tabs.size());

    return tabs;
}

void Tabs2::setParent(int tabId, int parentId)
{
    //Backend::funRet_t result = backend.performQuery(
    backend.performQueryNR(
        [this, tabId, parentId]()->void //Backend::funRet_t
        {
            qCDebug(dbLogs, "(dbName=%s) setParent: start", dbClient.getDbName().toStdString().c_str());
            auto start = std::chrono::system_clock::now();

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
                //return false;
            }
            auto end = std::chrono::system_clock::now();
            std::chrono::duration<double> total = end-start;
            qCDebug(dbLogs, "(dbName=%s) setParent: end; time: %lli ms",
                    dbClient.getDbName().toStdString().c_str(),
                    std::chrono::duration_cast<std::chrono::milliseconds>(total));

            //return true;
        }); //.get();
}

void Tabs2::setUrl(int tabId, QString url)
{
//    Backend::funRet_t result = backend.performQuery(
    backend.performQueryNR(
        [this, tabId, url]()->void //Backend::funRet_t
        {
            qCDebug(dbLogs, "(dbName=%s) setUrl: start", dbClient.getDbName().toStdString().c_str());
            auto start = std::chrono::system_clock::now();

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
                //return false;
            }

            auto end = std::chrono::system_clock::now();
            std::chrono::duration<double> total = end-start;
            qCDebug(dbLogs, "(dbName=%s) setUrl: end; time: %lli ms",
                    dbClient.getDbName().toStdString().c_str(),
                    std::chrono::duration_cast<std::chrono::milliseconds>(total));
            //return true;
        }); //.get();
}

void Tabs2::setTitle(int tabId, QString title)
{
//    Backend::funRet_t result = backend.performQuery(
    backend.performQueryNR(
        [this, tabId, title]()->void //Backend::funRet_t
        {
            qCDebug(dbLogs, "(dbName=%s) setTitle: start", dbClient.getDbName().toStdString().c_str());
            auto start = std::chrono::system_clock::now();

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
                //return false;
            }
            auto end = std::chrono::system_clock::now();
            std::chrono::duration<double> total = end-start;
            qCDebug(dbLogs, "(dbName=%s) setTitle: end; time: %lli ms",
                    dbClient.getDbName().toStdString().c_str(),
                    std::chrono::duration_cast<std::chrono::milliseconds>(total));

            //return true;
        }); //.get();
}


void Tabs2::setIcon(int tabId, QString icon)
{
//    Backend::funRet_t result = backend.performQuery(
    backend.performQueryNR(
        [this, tabId, icon]()->void //Backend::funRet_t
        {
            qCDebug(dbLogs, "(dbName=%s) setIcon: start", dbClient.getDbName().toStdString().c_str());
            auto start = std::chrono::system_clock::now();

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
                //return false;
            }
            auto end = std::chrono::system_clock::now();
            std::chrono::duration<double> total = end-start;
            qCDebug(dbLogs, "(dbName=%s) setIcon: end; time: %lli ms",
                    dbClient.getDbName().toStdString().c_str(),
                    std::chrono::duration_cast<std::chrono::milliseconds>(total));

            //return true;
        }); //.get();
}

}
