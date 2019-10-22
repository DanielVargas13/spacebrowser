#ifndef DB_CONFIG_H_
#define DB_CONFIG_H_

#include <db/Backend.h>
#include <db/DbClient.h>

#include <QString>
#include <QVariant>

namespace db
{

class Config2
{
public:
    Config2(DbClient& _dbClient, Backend& _backend);
    virtual ~Config2();

    bool setProperty(QString key, QVariant value);
    QVariant getProperty(QString key);

private:
    static QString tableName;
    DbClient& dbClient;
    Backend& backend;
};

}

#endif
