#include <db/DbClient.h>

#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QString>
#include <QVariant>

#include <iostream>

namespace db
{

const QString DbClient::schemaName("spaceBrowser2");

bool DbClient::initDatabase(QString dbName)
{
    /// Make sure schema exists
    ///
    if (!createSchemaIfNotExists(dbName))
    {
        std::cout << "DbClient::initDatabase(dbName=" << dbName.toStdString()
                  << "): failed to create schema\n";
    }

    return true;
}

bool DbClient::createSchemaIfNotExists(QString dbName)
{
//FIXME: test this with sqlite as it will 99% NOT WORK

    QSqlQuery query(QSqlDatabase::database(dbName));

    query.prepare("CREATE SCHEMA IF NOT EXISTS :schema_name");
    query.bindValue(":schema_name", schemaName);

    bool result = query.exec();

    if (!result)
        std::cout << query.lastError().text().toStdString() << std::endl;

    return result;
}

}
