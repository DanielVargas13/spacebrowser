#include <db/Config2.h>

#include <QSqlRecord>

namespace db
{

QString Config2::tableName("config");

Config2::Config2(DbClient& _dbClient, Backend& _backend) :
    dbClient(_dbClient), backend(_backend)
{

}

Config2::~Config2()
{

}

bool Config2::setProperty(QString key, QVariant value)
{
    Backend::funRet_t result = backend.performQuery(
        [this, &key, &value]()->Backend::funRet_t
        {
            QSqlQuery query(QSqlDatabase::database(dbClient.getDbName()));

            query.prepare(QString("INSERT INTO %1.%2 "
                                  "VALUES (:key, :value) "
                                  "ON CONFLICT (key) DO UPDATE SET value = :value")
                          .arg(dbClient.getSchemaName())
                          .arg(tableName));

            query.bindValue(":key", key);
            query.bindValue(":value", value);

            if (!query.exec())
            {
                qCCritical(dbLogs, "(dbName=%s): failed to set property %s",
                           dbClient.getDbName().toStdString().c_str(),
                           key.toStdString().c_str());
                dbClient.logError(query);
                //return std::move(query);
                return false;
            }

            //return std::move(query);
            return query.isActive();
        }).get();

    return std::holds_alternative<bool>(result) &&
        std::get<bool>(result);
}

QVariant Config2::getProperty(QString key)
{
    Backend::funRet_t result = backend.performQuery(
        [this, &key]()->Backend::funRet_t
        {
            auto db = QSqlDatabase::database(dbClient.getDbName());
            if (!db.isOpen() && !db.open())
            {
                qCDebug(dbLogs, "(dbName=%s): connection lost, failed to reconnect",
                        dbClient.getDbName().toStdString().c_str());

            }
            QSqlQuery query(db);

            query.prepare(QString("SELECT value FROM %1.%2 WHERE key = :key")
                          .arg(dbClient.getSchemaName())
                          .arg(tableName));

            query.bindValue(":key", key);

            if (!query.exec())
            {
                qCCritical(dbLogs, "(dbName=%s): failed to fetch property %s",
                           dbClient.getDbName().toStdString().c_str(),
                           key.toStdString().c_str());
                dbClient.logError(query);
                throw std::runtime_error("Config::getProperty(): failed to fetch property");
            }

            QVariant result;
            if (query.size() == 1)
            {
                query.first();
                result = query.value("value");
            }
            return result;
            //return std::move(query);
        }).get();

    //QSqlQuery& query = std::get<QSqlQuery>(result);

//    if (query.size() != 1)
//        return "";

//    query.next();

//    return query.value("value");

    return std::get<QVariant>(result);
}

}
