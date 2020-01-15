
#ifndef CONF_CONF_H_
#define CONF_CONF_H_

namespace conf
{
namespace MainWindow
{
constexpr const char* geometry = "MainWindow/geometry";
}

namespace Databases
{
constexpr const char* dbArray = "Databases/array";
constexpr const char* currentPanel = "Databases/currentPanel";

namespace array
{
constexpr const char* connName = "connName";
constexpr const char* driverType = "driverType";
constexpr const char* hostname = "hostname";
constexpr const char* dbName = "dbName";
constexpr const char* username = "username";
constexpr const char* password = "password";
constexpr const char* isEncrypted = "isEncrypted";
constexpr const char* connIcon = "connIcon";
constexpr const char* schemaName = "schemaName";
constexpr const char* port = "port";
}
}

}

#endif /* CONF_CONF_H_ */
