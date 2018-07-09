#include <db/Passwords.h>

#include <db/Keys.h>
#include <db/Sql.h>

#include <QString>

#include <exception>
#include <iostream>
#include <string>

namespace db
{

namespace sql
{
static QString insert("INSERT INTO \"%1\".\"%2\" VALUES (\'%3\', \'%4\', "
    "\'%5\', \'%6\', \'%7\' ) ON CONFLICT (host, path, login) "
    "DO UPDATE SET (password, key_fp) = (\'%6\', \'%7\');");
}

std::string Passwords::tableName("passwords");

Passwords::Passwords()
{
    sql::Helpers::createSchemaIfNotExists(conn, sql::schemaName);

    sql::Helpers::createTableIfNotExists(conn, sql::schemaName, tableName.c_str(),
        ("host varchar NOT NULL, path varchar, login varchar NOT NULL, "
            "password varchar NOT NULL, key_fp varchar REFERENCES "
            "\"" + sql::schemaName.toStdString() + "\".keys(fingerprint), "
            "CONSTRAINT pass_pkey PRIMARY KEY (host, path, login)").c_str());
}

Passwords::~Passwords()
{

}

Passwords::SaveState Passwords::isSaved(entry_t pwd)
{
    pqxx::nontransaction ntx(conn);

    pqxx::result r = ntx.exec(sql::select.arg("COUNT(password)").arg(sql::schemaName)
        .arg(tableName.c_str()).arg("WHERE host=\'%5\' AND path=\'%6\' "
            "AND login=\'%7\'").toStdString());

    if (r.size() != 1)
        throw std::runtime_error("Passwords::isSaved(): db returned " +
            std::to_string(r.size()) + " entries.\n");

    std::cout << "COUNT: " << r[0][0].as<int>() << std::endl;

    int count = r[0][0].as<int>();
    switch(count)
    {
    case 0: return SaveState::Absent;
    case 1: return SaveState::Outdated;
    // case ?: return SaveState::Current - there is no way to know
    //         if saved password is current without decrypting it first
    default: throw std::runtime_error("Passwords::isSaved(): "
        "password count returned: " + std::to_string(r[0][0].as<int>()));
    }
}

void Passwords::saveOrUpdate(entry_t pwd)
{
    pqxx::nontransaction ntx(conn);

    pqxx::result r = ntx.exec(sql::insert.arg(sql::schemaName)
        .arg(tableName.c_str()).arg(pwd.host).arg(pwd.path).arg(pwd.login)
        .arg(pwd.password).arg(pwd.fingerprint).toStdString());
}

} /* namespace db */
