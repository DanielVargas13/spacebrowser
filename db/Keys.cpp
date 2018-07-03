#include <db/Keys.h>

#include <QString>

#include <db/Sql.h>

namespace db
{

namespace sql
{
static QString insert("INSERT INTO \"%1\".\"%2\" VALUES (\'%3\', \'%4\');");
}

std::string Keys::tableName("keys");

Keys::Keys()
{
    sql::Helpers::createSchemaIfNotExists(conn, sql::schemaName);

    sql::Helpers::createTableIfNotExists(conn, sql::schemaName, tableName.c_str(),
            "fingerprint varchar, def boolean DEFAULT false, "
            "CONSTRAINT keys_pkey PRIMARY KEY (fingerprint)");
}
    
Keys::~Keys()
{

}

void Keys::addKey(std::string keyFingerprint, bool def)
{
    pqxx::nontransaction ntx(conn);

    pqxx::result r = ntx.exec(sql::insert.arg(sql::schemaName)
            .arg(tableName.c_str()).arg(keyFingerprint.c_str()).arg(def).toStdString());
}
    
std::string Keys::getDefaultKey()
{
    pqxx::nontransaction ntx(conn);

    pqxx::result r = ntx.exec(sql::select.arg("fingerprint").arg(sql::schemaName)
            .arg(tableName.c_str()).arg("WHERE def=\'%5'").arg(true).toStdString());

    if (r.size() > 1)
    {
        throw std::runtime_error("Keys::getDefaultKey(): PSQL returned " +
                std::to_string(r.size()) + " rows");
    }

    if (r.size() == 0)
        return std::string();

    return r[0][0].as<std::string>();
}

} /* namespace db */
