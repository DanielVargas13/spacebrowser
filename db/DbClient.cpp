#include <db/DbClient.h>

#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QString>
#include <QVariant>

Q_LOGGING_CATEGORY(dbLogs, "db")

namespace db
{

const QString DbClient::schemaName("spacebrowser2");

bool DbClient::initDatabase(QString dbName)
{
    /// Make sure schema exists
    ///
    if (!createSchemaIfNotExists(dbName))
    {
        qCCritical(dbLogs, "(dbname=%s): failed to create schema",
                   dbName.toStdString().c_str());
    }

    return true;
}

bool DbClient::createSchemaIfNotExists(QString dbName)
{
//FIXME: test this with sqlite as it will 99% NOT WORK

    QSqlQuery query(QSqlDatabase::database(dbName));

    bool result = query.exec("CREATE SCHEMA IF NOT EXISTS " + schemaName);

    if (!result)
    {
        QString msg = QString("(dbname=%1): failed to create schema").arg(dbName);
        qCCritical(dbLogs) << msg.toStdString().c_str();
        logError(query);
    }

    return result;
}

void DbClient::logError(const QSqlQuery& query)
{
    QString msg = "error text: " + query.lastError().text();
    qCCritical(dbLogs) << msg.toStdString().c_str();

    msg = "query text: " + query.executedQuery();
    qCCritical(dbLogs) << msg.toStdString().c_str();
}

}
