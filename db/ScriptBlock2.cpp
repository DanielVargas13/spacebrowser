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
    qCDebug(dbLogs, "ScriptBlock2::isAllowed(dbName=%s): start", dbClient.getDbName().toStdString().c_str());
    auto start = std::chrono::system_clock::now();

    /// This is a little hack, to print timing debug on function exit,
    /// since there are multiple return statements, this seems to be the cleanest way
    std::shared_ptr<int> callOnExit(new int,
                                    [this, &start](auto p)
                                    {
                                        delete p;

                                        auto end = std::chrono::system_clock::now();
                                        std::chrono::duration<double> total = end-start;
                                        qCDebug(dbLogs, "(dbName=%s) setParent: end; time: %lli ms",
                                                dbClient.getDbName().toStdString().c_str(),
                                                std::chrono::duration_cast<std::chrono::milliseconds>(total));
                                    });


    Backend::funRet_t result = backend.performQuery(
        [this, &url, &site, &earlyReturn]()->Backend::funRet_t
        {

            QSqlQuery query(QSqlDatabase::database(dbClient.getDbName()));

            /// With early return we are only interested if script should be blocked,
            /// details about type of allowance are not important
            if (earlyReturn)
            {
                query.prepare(QString("SELECT EXISTS(SELECT * FROM %1.%2 WHERE url=:url) "
                                      "OR EXISTS(SELECT * FROM %1.%3 WHERE site_url=:surl AND url=:url)")
                              .arg(dbClient.getSchemaName())
                              .arg(globalTableName)
                              .arg(localTableName));

                query.bindValue(":url", url);
                query.bindValue(":surl", site);
            }
            else
            {
                query.prepare(QString("SELECT EXISTS(SELECT * FROM %1.%2 WHERE url=:url)")
                              .arg(dbClient.getSchemaName())
                              .arg(globalTableName));

                query.bindValue(":url", url);
            }

            if (!query.exec())
            {
                qCCritical(dbLogs, "(dbName=%s): failed to check entry existence %s",
                           dbClient.getDbName().toStdString().c_str(),
                           url.toStdString().c_str());
                dbClient.logError(query);
            }

            if (!query.next())
                return QVariant();

            return QVariant(query.value(0).toBool()); // need to return as QVariant,
                                                      // we have three states to represent
        }).get();

    QVariant var = std::get<QVariant>(result);
    if (!var.isValid())
        return State::Blocked;

    bool allowedGlobally = var.toBool();

    /// Local table was already checked in earlyReturn case, see above
    if (earlyReturn)
    {
        if (allowedGlobally)
            return State::AllowedGlobally;
        return State::Blocked;
    }


    result = backend.performQuery(
        [this, site, url]()->Backend::funRet_t
        {
            QSqlQuery query(QSqlDatabase::database(dbClient.getDbName()));

            query.prepare(QString("SELECT EXISTS(SELECT * FROM %1.%2 "
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
            }

            if (!query.first())
                return QVariant();

            return QVariant(query.value(0).toBool());
        }).get();

    var = std::get<QVariant>(result);
    if (!var.isValid())
        return State::Blocked;

    bool allowedLocally = var.toBool();

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

            query.clear();
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
