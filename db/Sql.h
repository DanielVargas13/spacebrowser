#ifndef DB_SQL_H_
#define DB_SQL_H_

#include <pqxx/pqxx>

#include <QString>

namespace db {
namespace sql
{
static QString schemaName("spaceBrowser");

static QString schemaExists("SELECT EXISTS("
        "SELECT 1 FROM information_schema.schemata "
        "WHERE schema_name = '%1');");

static QString tableExists("SELECT EXISTS ("
        "SELECT 1 FROM information_schema.tables "
        "WHERE table_schema = '%1' "
        "AND table_name = '%2');");

static QString createSchema("CREATE SCHEMA \"%1\";");

static QString createTable("CREATE TABLE \"%1\".\"%2\" (%3);");

static QString select("SELECT %1 FROM \"%2\".\"%3\" %4;");

static QString createUIndex("CREATE UNIQUE INDEX IF NOT EXISTS \"%1_idx\" ON \"%2\".\"%3\"(\"%1\");");

static QString exists("SELECT EXISTS(SELECT 1 FROM \"%1\".\"%2\" %3);");

class Helpers
{
    public:

    static void createSchemaIfNotExists(pqxx::connection& conn, const QString& schemaName);
    static void createTableIfNotExists(pqxx::connection& conn, const QString& schemaName,
            const QString& tableName, const QString& columnDescription);
    static void createUIndex(pqxx::connection& conn, const QString& schemaName,
            const QString& tableName, const QString& columnName);
};

} /* namespace sql */
} /* namespace db */

#endif /* DB_SQL_H_ */
