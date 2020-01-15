#include <db/Passwords2.h>

#include <QSqlDatabase>
#include <QSqlQuery>

namespace db
{

QString Passwords2::tableName("passwords");

Passwords2::Passwords2(DbClient& _dbClient, Backend& _backend) :
    dbClient(_dbClient), backend(_backend)
{

}

Passwords2::~Passwords2()
{

}

Passwords2::SaveState Passwords2::isSaved(entry_t pwd)
{
    Backend::funRet_t result = backend.performQuery(
        [this, pwd]()->Backend::funRet_t
        {
            QSqlQuery query(QSqlDatabase::database(dbClient.getDbName()));

            query.prepare(QString("SELECT COUNT(password) FROM %1.%2 "
                                  "WHERE host=:host AND path=:path AND login=:login")
                          .arg(dbClient.getSchemaName())
                          .arg(tableName));

            query.bindValue(":host", pwd.host);
            query.bindValue(":path", pwd.path);
            query.bindValue(":login", pwd.login);

            if (!query.exec() || !query.first())
            {
                qCCritical(dbLogs, "(dbName=%s): failed to check if password was saved (%s%s, %s)",
                           dbClient.getDbName().toStdString().c_str(),
                           pwd.host.toStdString().c_str(),
                           pwd.path.toStdString().c_str(),
                           pwd.login.toStdString().c_str());
                dbClient.logError(query);
                throw std::runtime_error("Passwords::isSaved(): failed to check if password was saved");
            }

            return query.value(0);
            //return std::move(query);
        }).get();

//    QSqlQuery query = std::get<QSqlQuery>(result);
    QVariant var = std::get<QVariant>(result);

    switch(var.toInt())
    {
        case 0: return SaveState::Absent;
        case 1: return SaveState::Outdated;
        // case ?: return SaveState::Current - there is no way to know
        //         if saved password is current without decrypting it first
        default: throw std::runtime_error(QString("Passwords::isSaved(): "
                                                  "password count returned: %i")
                                          .arg(var.toInt()).toStdString());
    }
}

bool Passwords2::hasSavedCredentials(QString host, QString path)
{
    return countSavedCredentials(host, path) > 0;
}

int Passwords2::countSavedCredentials(QString host, QString path)
{
    Backend::funRet_t result = backend.performQuery(
        [this, host, path]()->Backend::funRet_t
        {
            QSqlQuery query(QSqlDatabase::database(dbClient.getDbName()));

            query.prepare(QString("SELECT COUNT(password) FROM %1.%2 "
                                  "WHERE host=:host AND path=:path")
                          .arg(dbClient.getSchemaName())
                          .arg(tableName));

            query.bindValue(":host", host);
            query.bindValue(":path", path);

            if (!query.exec() || !query.first())
            {
                qCCritical(dbLogs, "(dbName=%s): failed to check password count (%s%s)",
                           dbClient.getDbName().toStdString().c_str(),
                           host.toStdString().c_str(),
                           path.toStdString().c_str());
                dbClient.logError(query);
                throw std::runtime_error("Passwords::isSaved(): failed to check password count");
            }

            return query.value(0);
            //return std::move(query);
        }).get();

//    QSqlQuery query = std::get<QSqlQuery>(result);
    QVariant var = std::get<QVariant>(result);

    return var.toInt();
}

std::vector<Passwords2::entry_t> Passwords2::getCredentials(QString host, QString path)
{
    Backend::funRet_t result = backend.performQuery(
        [this, host, path]()->Backend::funRet_t
        {
            QSqlQuery query(QSqlDatabase::database(dbClient.getDbName()));

            query.prepare(QString("SELECT (login, password, key_fp) FROM %1.%2 "
                                  "WHERE host=:host AND path=:path")
                          .arg(dbClient.getSchemaName())
                          .arg(tableName));

            query.bindValue(":host", host);
            query.bindValue(":path", path);

            if (!query.exec())
            {
                qCCritical(dbLogs, "(dbName=%s): failed to fetch credentials (%s%s)",
                           dbClient.getDbName().toStdString().c_str(),
                           host.toStdString().c_str(),
                           path.toStdString().c_str());
                dbClient.logError(query);
            }

            std::vector<entry_t> r;
            while(query.next())
            {
                entry_t e;
                e.host = host;
                e.path = path;
                e.login = query.value("login").toString();
                e.password = query.value("password").toString();
                e.fingerprint = query.value("key_fp").toString();

                r.push_back(e);
            }

            return QVariant::fromValue<std::vector<entry_t>>(std::move(r));
            //return std::move(query);
        }).get();

    //QSqlQuery query = std::get<QSqlQuery>(result);
    QVariant var = std::get<QVariant>(result);

    return var.value<std::vector<entry_t>>();
}

bool Passwords2::saveOrUpdate(entry_t pwd)
{
    Backend::funRet_t result = backend.performQuery(
        [this, pwd]()->Backend::funRet_t
        {
            QSqlQuery query(QSqlDatabase::database(dbClient.getDbName()));

            query.prepare(QString("INSERT INTO %1.%2 "
                                  "VALUES (:host :path :login :pass :fp) "
                                  "ON CONFLICT (host, path, login) "
                                  "DO UPDATE SET (password, key_fp) = :pass :fp")
                          .arg(dbClient.getSchemaName())
                          .arg(tableName));

            query.bindValue(":host", pwd.host);
            query.bindValue(":path", pwd.path);
            query.bindValue(":login", pwd.login);
            query.bindValue(":pass", pwd.password);
            query.bindValue(":fp", pwd.fingerprint);

            if (!query.exec())
            {
                qCCritical(dbLogs, "(dbName=%s): failed to fetch credentials (%s%s)",
                           dbClient.getDbName().toStdString().c_str(),
                           pwd.host.toStdString().c_str(),
                           pwd.path.toStdString().c_str());
                dbClient.logError(query);

                return false;
            }

            return true;
        }).get();

    return std::get<bool>(result);
}

}
