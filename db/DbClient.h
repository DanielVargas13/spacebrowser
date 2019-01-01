#ifndef DB_DBCLIENT_H_
#define DB_DBCLIENT_H_

#include <QLoggingCategory>
#include <QSqlQuery>
#include <QString>

Q_DECLARE_LOGGING_CATEGORY(dbLogs)

namespace db
{

/**
 * Class for common functionality of classes accessing db
 */
class DbClient
{

public:

    /**
     * Perform any required db initializations (table creation / update)
     * @param dbName name of database connection
     */
    virtual bool initDatabase(QString dbName);

protected:
    /**
     * Log last error raised by query
     */
    void logError(const QSqlQuery& query);

private:
    /**
     * Create database schema
     */
    bool createSchemaIfNotExists(QString dbName);

protected:
    static const QString schemaName;
};



};

#endif /* DB_DBCLIENT_H_ */
