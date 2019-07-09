#include <db/Config2.h>

namespace db
{

std::string Config2::tableName("config");

Config2::Config2(DbClient& _dbClient) : dbClient(_dbClient)
{

}

Config2::~Config2()
{

}
bool Config2::setProperty(const std::string& key, const QVariant& value)
{
    QSqlQuery query(QSqlDatabase::database(dbClient.getDbName()));

    query.prepare(QString("INSERT INTO %1.%2 "
                          "VALUES (:key, :value) "
                          "ON CONFLICT (key) DO UPDATE SET value = :value")
                  .arg(dbClient.getSchemaName())
                  .arg(tableName.c_str()));

    query.bindValue(":key", key.c_str());
    query.bindValue(":value", value);

    if (!query.exec())
    {
        qCCritical(dbLogs, "(dbName=%s): failed to set property %s",
                   dbClient.getDbName().toStdString().c_str(),
                   key.c_str());
        dbClient.logError(query);
        return false;
    }

    return true;
}

QVariant Config2::getProperty(const std::string& key)
{
    QSqlQuery query(QSqlDatabase::database(dbClient.getDbName()));

    query.prepare(QString("SELECT value FROM %1.%2 WHERE key=:key")
                  .arg(dbClient.getSchemaName())
                  .arg(tableName.c_str()));

    query.bindValue(":key", key.c_str());

    if (!query.exec())
    {
        qCCritical(dbLogs, "(dbName=%s): failed to fetch property %s",
                   dbClient.getDbName().toStdString().c_str(),
                   key.c_str());
        dbClient.logError(query);
        throw std::runtime_error("Config::getProperty(): failed to fetch property");
    }

    if (query.size() == 0)
    {
        return "";
    }

    query.next();

    return query.value("value");
}


}
