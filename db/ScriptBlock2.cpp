#include <db/ScriptBlock2.h>

#include <QVariant>

namespace db
{

QString ScriptBlock2::globalTableName("global_script_whitelist");
QString ScriptBlock2::localTableName("script_whitelist");
QString ScriptBlock2::siteTableName("site_list");

ScriptBlock2::ScriptBlock2(DbClient& _dbClient, Backend& _backend) :
    dbClient(_dbClient), backend(_backend)
{

}

ScriptBlock2::~ScriptBlock2()
{

}

ScriptBlock2::State ScriptBlock2::isAllowed(const QString& site,
                                            const QString& url,
                                            bool earlyReturn)
{
    Backend::funRet_t result = backend.performQuery(
        [this, url]()->Backend::funRet_t
        {
            QSqlQuery query(QSqlDatabase::database(dbClient.getDbName()));
            
            query.prepare(QString("SELECT EXISTS(SELECT 1 FROM %1.%2 WHERE url=:url)")
                          .arg(dbClient.getSchemaName())
                          .arg(globalTableName));
            
            query.bindValue(":url", url);
            
            if (!query.exec())
            {
                qCCritical(dbLogs, "(dbName=%s): failed to check entry existence %s",
                           dbClient.getDbName().toStdString().c_str(),
                           url.toStdString().c_str());
                dbClient.logError(query);
                //return State::Blocked;
            }
            
            return std::move(query);
        }).get();

    QSqlQuery query = std::get<QSqlQuery>(result);

    if (!query.next())
        return State::Blocked;

    bool allowedGlobally = query.value(0).toBool();

    if (earlyReturn && allowedGlobally)
        return State::AllowedGlobally;


    result = backend.performQuery(
        [this, site, url]()->Backend::funRet_t
        {
            QSqlQuery query(QSqlDatabase::database(dbClient.getDbName()));

            query.prepare(QString("SELECT EXISTS(SELECT 1 FROM %1.%2 "
                                  "WHERE site_url=:surl AND url=:url)")
                          .arg(dbClient.getSchemaName())
                          .arg(localTableName));
            
            query.bindValue(":surl", site);
            query.bindValue(":url", url);
            

            if (!query.exec())
            {
                qCCritical(dbLogs, "(dbName=%s): failed to check entry existence %s",
                           dbClient.getDbName().toStdString().c_str(),
                           url.toStdString().c_str());
                dbClient.logError(query);
                //return State::Blocked;
            }

            return std::move(query);
        }).get();

    query = std::get<QSqlQuery>(result);

    if (!query.first())
        return State::Blocked;

    bool allowedLocally = query.value(0).toBool();

    if (allowedGlobally && allowedLocally)
        return State::AllowedBoth;
    else if (allowedGlobally)
        return State::AllowedGlobally;
    else if (allowedLocally)
        return State::Allowed;

    return State::Blocked;
}

void ScriptBlock2::whitelistLocal(const QString& site, const QString& url)
{
    Backend::funRet_t result = backend.performQuery(
        [this, site, url]()->Backend::funRet_t
        {
            QSqlQuery query(QSqlDatabase::database(dbClient.getDbName()));

            query.prepare(QString("INSERT INTO %1.%2 (url) VALUES (:site) "
                                  "ON CONFLICT (url) DO NOTHING")
                          .arg(dbClient.getSchemaName())
                          .arg(siteTableName));
            
            query.bindValue(":site", site);

            if (!query.exec())
            {
                qCCritical(dbLogs, "(dbName=%s): failed to add to site table %s",
                           dbClient.getDbName().toStdString().c_str(),
                           url.toStdString().c_str());
                dbClient.logError(query);
                return false;
            }

            query.prepare(QString("INSERT INTO %1.%2 (site_url, url) VALUES (:site, :url) ")
                          .arg(dbClient.getSchemaName())
                          .arg(localTableName));
            
            query.bindValue(":site", site);
            query.bindValue(":url", url);
            
            if (!query.exec())
            {
                qCCritical(dbLogs, "(dbName=%s): failed to add to local whitelist %s",
                           dbClient.getDbName().toStdString().c_str(),
                           url.toStdString().c_str());
                dbClient.logError(query);
                return false;
            }

            return true;
        }).get();
}

void ScriptBlock2::whitelistGlobal(const QString& url)
{
    Backend::funRet_t result = backend.performQuery(
        [this, url]()->Backend::funRet_t
        {
            QSqlQuery query(QSqlDatabase::database(dbClient.getDbName()));

            query.prepare(QString("INSERT INTO %1.%2 (url) VALUES (:url) "
                                  "ON CONFLICT (url) DO NOTHING")
                          .arg(dbClient.getSchemaName())
                          .arg(globalTableName));
            
            query.bindValue(":url", url);
            
            if (!query.exec())
            {
                qCCritical(dbLogs, "(dbName=%s): failed to whitelist globally %s",
                           dbClient.getDbName().toStdString().c_str(),
                           url.toStdString().c_str());
                dbClient.logError(query);
                return false;
            }

            return true;
        }).get();
}

void ScriptBlock2::removeLocal(const QString& site, const QString& url)
{
    Backend::funRet_t result = backend.performQuery(
        [this, site, url]()->Backend::funRet_t
        {
            QSqlQuery query(QSqlDatabase::database(dbClient.getDbName()));
            
            query.prepare(QString("DELETE FROM %1.%2 "
                                  "WHERE site_url=:surl AND url=:url")
                          .arg(dbClient.getSchemaName())
                          .arg(localTableName));
            
            query.bindValue(":surl", site);
            query.bindValue(":url", url);
            
            if (!query.exec())
            {
                qCCritical(dbLogs, "(dbName=%s): failed to remove from local whitelist %s",
                           dbClient.getDbName().toStdString().c_str(),
                           url.toStdString().c_str());
                dbClient.logError(query);
                return false;
            }

            return true;
        }).get();
}

void ScriptBlock2::removeGlobal(const QString& url)
{
    Backend::funRet_t result = backend.performQuery(
        [this, url]()->Backend::funRet_t
        {
            QSqlQuery query(QSqlDatabase::database(dbClient.getDbName()));

            query.prepare(QString("DELETE FROM %1.%2 "
                                  "WHERE url=:url")
                          .arg(dbClient.getSchemaName())
                          .arg(globalTableName));
            
            query.bindValue(":url", url);

            if (!query.exec())
            {
                qCCritical(dbLogs, "(dbName=%s): failed to remove from global whitelist %s",
                           dbClient.getDbName().toStdString().c_str(),
                           url.toStdString().c_str());
                dbClient.logError(query);
                return false;
            }

            return true;
        }).get();
}

}
