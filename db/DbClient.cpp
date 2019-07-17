#include <db/DbClient.h>

#include <db/Config2.h>

#include <QLoggingCategory>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QString>
#include <QVariant>

Q_LOGGING_CATEGORY(dbLogs, "db")

namespace db
{
DbClient::DbClient(db::Backend& _backend) : backend(_backend)
{
    migrators.push_back(&DbClient::setupDbV1);
}

DbClient::~DbClient()
{
}

bool DbClient::initDatabase(QString _dbName)
{
    dbName = _dbName;

    /// Make sure schema exists
    ///
    if (!createSchemaIfNotExists())
    {
        qCCritical(dbLogs, "(dbname=%s): failed to create schema",
                   dbName.toStdString().c_str());
        return false;
    }

    /// Fetch schema version
    ///
    unsigned int dbVersion = fetchSchemaVersion();

    /// No need to update db
    ///
    if (dbVersion == schemaVersion)
        return true;

    /// Open a transaction
    ///
    QSqlDatabase db = QSqlDatabase::database(dbName);
    if (!db.transaction())
    {
        qCCritical(dbLogs, "(dbname=%s): failed to open transaction",
                   dbName.toStdString().c_str());
        return false;
    }

    /// Perform db migrations if necessary
    ///
    for (std::size_t i = dbVersion; i < migrators.size(); ++i)
    {
        if (!(this->*migrators[i])())
        {
            qCCritical(dbLogs, "(dbname=%s): failed to perform migration from version %i to %lu",
                       dbName.toStdString().c_str(), dbVersion, i+1);
            if (!db.rollback())
                qCCritical(dbLogs, "(dbname=%s): rollback failed", dbName.toStdString().c_str());

            return false;
        }
    }

    unsigned int newestVersion = migrators.size();
    if (newestVersion > 0 && dbVersion < newestVersion)
        setSchemaVersion(newestVersion);

    /// All processing was successful, commit changes to db
    ///
    if (!db.commit())
    {
        qCCritical(dbLogs, "(dbname=%s): commit failed", dbName.toStdString().c_str());
        logError(db.lastError());
        return false;

        // FIXME: mark if initialization succeeded, refuse to use db if it didn't
    }
    qCDebug(dbLogs, "(dbname=%s): db initialized properly", dbName.toStdString().c_str());

    return true;
}

unsigned int DbClient::fetchSchemaVersion()
{
    int dbVersion = 0;

    Config2 cfg(*this, backend);
    Backend::funRet_t result = backend.performQuery(
        [this]()->Backend::funRet_t
        {
            QSqlDatabase db = QSqlDatabase::database(dbName);
            return db.tables().contains(schemaName + "." + "config");
        }).get();

    if (std::holds_alternative<bool>(result) && std::get<bool>(result))
    {
        bool ok = false;
        int dbv = cfg.getProperty("schemaVersion").toInt(&ok);

        if (ok)
            dbVersion = dbv;
    }

    qCDebug(dbLogs, "(dbname=%s): schema version = %i",
            dbName.toStdString().c_str(), dbVersion);
    return dbVersion;
}

void DbClient::setSchemaVersion(unsigned int version)
{
    Config2 cfg(*this, backend);

    if (cfg.setProperty("schemaVersion", version))
        qCDebug(dbLogs, "(dbname=%s): schema version set to = %i",
                dbName.toStdString().c_str(), version);
    else
        qCCritical(dbLogs, "(dbname=%s): failed to set schema version",
                   dbName.toStdString().c_str());
}


bool DbClient::createSchemaIfNotExists()
{
//FIXME: test this with sqlite as it will 99% NOT WORK

    QSqlQuery query = backend.performQuery(
        dbName, "CREATE SCHEMA IF NOT EXISTS " + schemaName).get();
    bool result = query.isActive();

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

void DbClient::logError(const QSqlError& error)
{
    QString msg = "error text: " + error.text();
    qCCritical(dbLogs) << msg.toStdString().c_str();
}

QString DbClient::getSchemaName()
{
    return schemaName;
}

bool DbClient::setupDbV1()
{
    /// Set up Tabs table
    ///
    QSqlDatabase db = QSqlDatabase::database(dbName);

    if (!db.tables().contains(schemaName + "." + "tabs"))
    {
        QSqlQuery query = backend.performQuery(
            dbName,
            "CREATE TABLE IF NOT EXISTS " +
            schemaName + "." + "tabs" +
            "(id serial PRIMARY KEY,"
            "parent integer default 0,"
            "url varchar default \'\',"
            "title varchar default \'\',"
            "icon varchar default \'\')").get();
        bool result = query.isActive();

        if (!result)
        {
            qCCritical(dbLogs, "(dbName=%s): failed to create tabs table",
                       dbName.toStdString().c_str());

            logError(query);
            return false;
        }
    }

    /// Set up Config table
    ///
    if (!db.tables().contains(schemaName + "." + "config"))
    {
        QSqlQuery query = backend.performQuery(
            dbName,
            "CREATE TABLE IF NOT EXISTS " +
            schemaName + "." + "config" +
            "(key varchar PRIMARY KEY, value varchar)").get();
        bool result = query.isActive();

        if (!result)
        {
            qCCritical(dbLogs, "(dbName=%s): failed to create config table",
                       dbName.toStdString().c_str());

            logError(query);
            return false;
        }
    }

    /// Set up keys table
    ///
    if (!db.tables().contains(schemaName + "." + "keys"))
    {
        QSqlQuery query = backend.performQuery(
            dbName,
            "CREATE TABLE IF NOT EXISTS " +
            schemaName + "." + "keys" +
            "(fingerprint varchar PRIMARY KEY,"
            "def boolean DEFAULT false)").get();
        bool result = query.isActive();

        if (!result)
        {
            qCCritical(dbLogs, "(dbName=%s): failed to create keys table",
                       dbName.toStdString().c_str());

            logError(query);
            return false;
        }
    }

    /// Set up passwords table
    ///
    if (!db.tables().contains(schemaName + "." + "passwords"))
    {
        QSqlQuery query = backend.performQuery(
            dbName,
            "CREATE TABLE IF NOT EXISTS " +
            schemaName + "." + "passwords" +
            "(host varchar NOT NULL, path varchar,"
            "login varchar NOT NULL,"
            "password varchar NOT NULL,"
            "key_fp varchar REFERENCES " + schemaName +
            ".keys(fingerprint),"
            "CONSTRAINT pass_pkey "
            "PRIMARY KEY(host, path, login))").get();
        bool result = query.isActive();

        if (!result)
        {
            qCCritical(dbLogs, "(dbName=%s): failed to create passwords table",
                       dbName.toStdString().c_str());

            logError(query);
            return false;
        }
    }

    /// Set up global_script_whitelist table
    ///
    if (!db.tables().contains(schemaName + "." + "global_script_whitelist"))
    {
        QSqlQuery query = backend.performQuery(
            dbName,
            "CREATE TABLE IF NOT EXISTS " +
            schemaName + "." + "global_script_whitelist" +
            "(url varchar PRIMARY KEY)").get();
        bool result = query.isActive();

        if (!result)
        {
            qCCritical(dbLogs, "(dbName=%s): failed to create global_script_whitelist"
                       " table", dbName.toStdString().c_str());

            logError(query);
            return false;
        }
    }

    /// Set up site_list table
    ///
    if (!db.tables().contains(schemaName + "." + "site_list"))
    {
        QSqlQuery query = backend.performQuery(
            dbName,
            "CREATE TABLE IF NOT EXISTS " +
            schemaName + "." + "site_list" +
            "(url varchar PRIMARY KEY)").get();
        bool result = query.isActive();

        if (!result)
        {
            qCCritical(dbLogs, "(dbName=%s): failed to create site_list"
                       " table", dbName.toStdString().c_str());

            logError(query);
            return false;
        }
    }

    /// Set up script_whitelist table
    ///
    if (!db.tables().contains(schemaName + "." + "script_whitelist"))
    {
        QSqlQuery query = backend.performQuery(
            dbName,
            "CREATE TABLE IF NOT EXISTS " +
            schemaName + "." + "script_whitelist" +
            "(id serial PRIMARY KEY,"
            "site_url varchar REFERENCES " + schemaName +
            ".site_list, url varchar)").get();
        bool result = query.isActive();

        if (!result)
        {
            qCCritical(dbLogs, "(dbName=%s): failed to create script_whitelist"
                       " table", dbName.toStdString().c_str());

            logError(query);
            return false;
        }
    }

    qCDebug(dbLogs, "(dbName=%s): setupDbV1() finished successfully",
               dbName.toStdString().c_str());

    return true;
}

QString DbClient::getDbName()
{
    return dbName;
}

}
