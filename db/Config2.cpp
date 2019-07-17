#include <db/Config2.h>

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
                return std::move(query);
            }
            
            return std::move(query);
        }).get();

    return std::holds_alternative<QSqlQuery>(result) &&
        std::get<QSqlQuery>(result).isActive();
}

QVariant Config2::getProperty(QString key)
{
    Backend::funRet_t result = backend.performQuery(
        [this, &key]()->Backend::funRet_t
        {
            QSqlQuery query(QSqlDatabase::database(dbClient.getDbName()));

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

            return std::move(query);
        }).get();

    QSqlQuery query = std::get<QSqlQuery>(result);
    
    if (query.size() == 0)
        return "";

    query.next();
    return query.value("value");
}

}
