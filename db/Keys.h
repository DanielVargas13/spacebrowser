#ifndef DB_KEYS_H_
#define DB_KEYS_H_

#include <pqxx/pqxx>

#include <string>

namespace db
{

class Keys
{
public:
    Keys();
    virtual ~Keys();

    void addKey(std::string keyFingerprint, bool def = false);
    std::string getDefaultKey();

private:
    static std::string tableName;
    pqxx::connection conn;

};

} /* namespace db */

#endif /* DB_KEYS_H_ */
