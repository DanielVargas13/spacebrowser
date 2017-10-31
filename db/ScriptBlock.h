#ifndef DB_SCRIPTBLOCK_H_
#define DB_SCRIPTBLOCK_H_

#include <pqxx/pqxx>

#include <string>

namespace db
{

class ScriptBlock
{
public:
    enum class State {
        Blocked,
        Allowed,
        AllowedGlobally,
        AllowedBoth
    };

    ScriptBlock();
    virtual ~ScriptBlock();

    State isAllowed(const std::string& site, const std::string& url, bool earlyReturn = true);
    void whitelistLocal(const std::string& site, const std::string& url);
    void whitelistGlobal(const std::string& url);
    void removeLocal(const std::string& site, const std::string& url);
    void removeGlobal(const std::string& url);

private:
    static std::string globalTableName;
    static std::string localTableName;

    pqxx::connection conn;
};

} /* namespace db */

#endif /* DB_SCRIPTBLOCK_H_ */
