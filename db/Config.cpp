#include <db/Config.h>

#include <db/Sql.h>

#include <pqxx/pqxx>
#include <QString>

#include <iostream>

namespace db
{

namespace sql
{
static QString insertOrUpdate("INSERT INTO \"%1\".\"%2\" VALUES (\'%3\', \'%4\') "
        "ON CONFLICT (%5) DO UPDATE SET \"%6\" = \'%4\';");
}

std::string Config::tableName("config");

Config::Config()
{
    sql::Helpers::createSchemaIfNotExists(conn, sql::schemaName);

    sql::Helpers::createTableIfNotExists(conn, sql::schemaName, tableName.c_str(),
            "key varchar, value varchar, CONSTRAINT config_pkey PRIMARY KEY (key)");
}

Config::~Config()
{

}

void Config::setProperty(const std::string& name, const std::string& value)
{
    pqxx::nontransaction ntx(conn);

    pqxx::result r = ntx.exec(sql::insertOrUpdate.arg(sql::schemaName)
            .arg(tableName.c_str()).arg(name.c_str()).arg(value.c_str())
            .arg("key").arg("value").toStdString());
}

std::string Config::getProperty(const std::string& name)
{
    pqxx::nontransaction ntx(conn);

    pqxx::result r = ntx.exec(sql::select.arg("value").arg(sql::schemaName)
            .arg(tableName.c_str()).arg("WHERE key=\'%5\'")
            .arg(name.c_str()).toStdString());

    if (r.size() > 1)
    {
        throw std::runtime_error("Config::getProperty(): "
                "PSQL returned " + std::to_string(r.size()) + " rows");
    }

    if (r.size() == 0)
        return std::string();

    return r[0][0].as<std::string>();
}

} /* namespace db */
