#ifndef DB_KEYS2_H_
#define DB_KEYS2_H_

#include <db/Backend.h>
#include <db/DbClient.h>

namespace db
{

class Keys2
{
public:
    Keys2(DbClient& _dbClient, Backend& _backend);
    virtual ~Keys2();

    bool addKey(std::string keyFingerprint, bool def = false);
    QString getDefaultKey();

private:
    static QString tableName;
    DbClient& dbClient;
    Backend& backend;
};

}

#endif
