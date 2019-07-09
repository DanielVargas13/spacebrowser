#ifndef DB_CONFIG_H_
#define DB_CONFIG_H_

#include <db/DbClient.h>

#include <QVariant>

namespace db
{

class Config2
{
public:
    Config2(DbClient& _dbClient);
    virtual ~Config2();

    bool setProperty(const std::string& key, const QVariant& value);
    QVariant getProperty(const std::string& key);

private:
    static std::string tableName;
    DbClient& dbClient;
};

}

#endif
