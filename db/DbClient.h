#ifndef DB_DBCLIENT_H_
#define DB_DBCLIENT_H_

#include <QLoggingCategory>
#include <QSqlQuery>
#include <QString>

#include <db/Backend.h>

#include <vector>

Q_DECLARE_LOGGING_CATEGORY(dbLogs)

namespace db
{

/**
 * Class for common functionality of classes accessing db
 */
class DbClient
{
public:

    DbClient(db::Backend& _backend);

    virtual ~DbClient();

    /**
     * Perform any required db initializations (table creation / update)
     * @param dbName name of database connection
     */
    bool initDatabase(QString _dbName);

    /**
     * Return db name
     * @return db name
     */
    QString getDbName();

    /**
     * Return schema name
     * @return schema name ;)
     */
    QString getSchemaName();

    /**
     * Log last error raised by query
     */
    void logError(const QSqlQuery& query);

    /**
     * Log error
     */
    void logError(const QSqlError& error);

private:
    /**
     * Create database schema
     */
    bool createSchemaIfNotExists();

    /**
     * Tries to read from database current version of schema
     * @return db schema version if read successfully, -1 if failed
     */
    unsigned int fetchSchemaVersion();

    /**
     * Sets current version of schema in db
     * @param version current version of db schema
     */
    void setSchemaVersion(unsigned int version);

    bool setupDbV1();

private:
    QString dbName;
    const QString schemaName = "spacebrowser2";
    const unsigned int schemaVersion = 1;
    std::vector<bool (DbClient::*)()> migrators;
    db::Backend& backend;
};



};

#endif /* DB_DBCLIENT_H_ */
