#ifndef DB_DBCLIENT_H_
#define DB_DBCLIENT_H_

#include <QString>

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
