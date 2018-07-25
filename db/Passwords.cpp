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
            "AND login=\'%7\'").arg(pwd.host).arg(pwd.path).arg(pwd.login)
            .toStdString());

    if (r.size() != 1)
        throw std::runtime_error("Passwords::isSaved(): db returned " +
            std::to_string(r.size()) + " entries.\n");

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

bool Passwords::hasSavedCredentials(QString host, QString path)
{
    return countSavedCredentials(host, path) > 0;
}

int Passwords::countSavedCredentials(QString host, QString path)
{
    pqxx::nontransaction ntx(conn);

    pqxx::result r = ntx.exec(sql::select.arg("COUNT(password)").arg(sql::schemaName)
            .arg(tableName.c_str()).arg("WHERE host=\'%5\' AND path=\'%6\'")
            .arg(host).arg(path).toStdString());

    if (r.size() != 1)
        throw std::runtime_error("Passwords::hasSavedCredentials(): db returned " +
            std::to_string(r.size()) + " entries.\n");

    return r[0][0].as<int>();
}

void Passwords::saveOrUpdate(entry_t pwd)
{
    pqxx::nontransaction ntx(conn);

    pqxx::result r = ntx.exec(sql::insert.arg(sql::schemaName)
        .arg(tableName.c_str()).arg(pwd.host).arg(pwd.path).arg(pwd.login)
        .arg(pwd.password).arg(pwd.fingerprint).toStdString());
}

std::vector<Passwords::entry_t> Passwords::getCredentials(QString host, QString path)
{
    pqxx::nontransaction ntx(conn);

    // For whatever reason path sometimes is size 1 with \0 as the only character
    path.remove(QChar('\0'));
    path = path.trimmed();

    std::vector<entry_t> result;

    try
    {
        pqxx::result r = ntx.exec(sql::select.arg("login, password, key_fp")
                .arg(sql::schemaName).arg(tableName.c_str())
                .arg("WHERE host=\'%5\' AND path=\'%6\'")
                .arg(host).arg(path).toStdString());

        if (r.size() < 1)
            return result;

        for (unsigned int i = 0; i < r.size(); ++i)
        {
            entry_t e;
            e.host = host;
            e.path = path;
            e.login = r[i][0].as<std::string>().c_str();
            e.password = r[i][1].as<std::string>().c_str();
            e.fingerprint = r[i][2].as<std::string>().c_str();

            result.push_back(e);
        }
    }
    catch (std::exception& e)
    {
        std::cout << "Passwords::getCredentials(): exception thrown: " << e.what()
                  << std::endl;
    }

    return result;
}

} /* namespace db */
