#ifndef DB_SCRIPTBLOCK2_H_
#define DB_SCRIPTBLOCK2_H_

#include <db/Backend.h>
#include <db/DbClient.h>

#include <QString>

namespace db
{

class ScriptBlock2
{
public:
    enum class State {
        Blocked,
        Allowed,
        AllowedGlobally,
        AllowedBoth
    };

    ScriptBlock2(DbClient& _dbClient, Backend& _backend);
    virtual ~ScriptBlock2();

    /**
     * @param earlyReturn if detailed information on allowance type is not needed
     *                    (=true) this allows to optimize db communication
     */
    State isAllowed(const QString& site, const QString& url, bool earlyReturn = true);
    void whitelistLocal(const QString& site, const QString& url);
    void whitelistGlobal(const QString& url);
    void removeLocal(const QString& site, const QString& url);
    void removeGlobal(const QString& url);


private:
    static QString globalTableName;
    static QString localTableName;
    static QString siteTableName;
    DbClient& dbClient;
    Backend& backend;
};

}

#endif
