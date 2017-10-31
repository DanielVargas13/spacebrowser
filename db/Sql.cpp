#include <db/Sql.h>

#include <pqxx/pqxx>

#include <iostream>

namespace db {
namespace sql
{

void Helpers::createSchemaIfNotExists(pqxx::connection& conn, const QString& schemaName)
{
    pqxx::work txn(conn);

    pqxx::result r = txn.exec(sql::schemaExists.arg(schemaName).toStdString());

    if (r.size() != 1)
    {
        throw std::runtime_error(
                "Tabs::createSchemaIfNotExists(): PSQL returned " + std::to_string(r.size()) + " rows");
    }

    bool schemaExists = r[0][0].as<bool>();

    if (!schemaExists)
    {
        std::cout << "spaceBrowser schema doesn't exist, creating one\n";
        r = txn.exec(sql::createSchema.arg(schemaName).toStdString());
        txn.commit();
    }
}

void Helpers::createTableIfNotExists(pqxx::connection& conn, const QString& schemaName,
        const QString& tableName, const QString& columnDescription)
{
    pqxx::work txn(conn);

    pqxx::result r = txn.exec(sql::tableExists.arg(schemaName)
            .arg(tableName).toStdString());

    if (r.size() != 1)
    {
        throw std::runtime_error(
                "Tabs::createTableIfNotExists(): PSQL returned " + std::to_string(r.size()) + " rows");
    }

    bool tableExists = r[0][0].as<bool>();

    if (!tableExists)
    {
        std::cout << "spaceBrowser.tabs table doesn't exist, creating one\n";
        r = txn.exec(sql::createTable.arg(schemaName).arg(QString(tableName))
                .arg(columnDescription).toStdString());
        txn.commit();
    }
}

void Helpers::createUIndex(pqxx::connection& conn, const QString& schemaName,
        const QString& tableName, const QString& columnName)
{
    pqxx::work txn(conn);

    pqxx::result r = txn.exec(sql::createUIndex.arg(columnName).arg(schemaName)
            .arg(tableName).toStdString());

    txn.commit();
}

} // namespace sql
} // namespace db
