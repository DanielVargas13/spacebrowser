#include <db/Keys2.h>

#include <QSqlDatabase>
#include <QSqlQuery>
#include <QVariant>

namespace db
{

QString Keys2::tableName("keys");

Keys2::Keys2(DbClient& _dbClient, Backend& _backend) :
    dbClient(_dbClient), backend(_backend)
{

}

Keys2::~Keys2()
{

}

bool Keys2::addKey(std::string keyFingerprint, bool def)
{
    Backend::funRet_t result = backend.performQuery(
        [this, &keyFingerprint, def]()->Backend::funRet_t
        {
            QSqlQuery query(QSqlDatabase::database(dbClient.getDbName()));

            query.prepare(QString("INSERT INTO %1.%2 "
                                  "VALUES (:keyfp, :def) ")
                          .arg(dbClient.getSchemaName())
                          .arg(tableName));

            query.bindValue(":keyfp", keyFingerprint.c_str());
            query.bindValue(":def", def);

            if (!query.exec())
            {
                qCCritical(dbLogs, "(dbName=%s): failed to add key %s",
                           dbClient.getDbName().toStdString().c_str(),
                           keyFingerprint.c_str());
                dbClient.logError(query);
                return false;
            }

            return true;
        }).get();

    return std::get<bool>(result);
}

QString Keys2::getDefaultKey()
{
    Backend::funRet_t result = backend.performQuery(
        [this]()->Backend::funRet_t
        {
            QSqlQuery query(QSqlDatabase::database(dbClient.getDbName()));

            query.prepare(QString("SELECT fingerprint FROM %1.%2 "
                                  "WHERE def=:def")
                          .arg(dbClient.getSchemaName())
                          .arg(tableName));

            query.bindValue(":def", true);

            if (!query.exec())
            {
                qCCritical(dbLogs, "(dbName=%s): failed to fetch default key",
                           dbClient.getDbName().toStdString().c_str());
                dbClient.logError(query);
//                return "";
            }

            if (!query.next())
                return QVariant(QString(""));

            return query.value("fingerprint");
            //return std::move(query);
        }).get();

//    QSqlQuery query = std::get<QSqlQuery>(result);
    QVariant var = std::get<QVariant>(result);
    return var.toString();
//    if (!query.next())
//    if (!var.isValid())
//        return "";

//    return query.value("fingerprint").toString();
}

}
