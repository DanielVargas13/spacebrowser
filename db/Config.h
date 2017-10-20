#ifndef DB_CONFIG_H_
#define DB_CONFIG_H_

#include <pqxx/pqxx>

#include <string>

namespace db
{

/**
 * Class for handling Browser configuration and misc. state information
 */
class Config
{
public:
    Config();
    virtual ~Config();

    void setProperty(const std::string& name, const std::string& value);
    std::string getProperty(const std::string& name);

private:
    static std::string tableName;
    pqxx::connection conn;
};

} /* namespace db */

#endif /* DB_CONFIG_H_ */
