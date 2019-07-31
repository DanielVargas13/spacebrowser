#include <db/DbGroup.h>

namespace db
{

std::map<QString, std::shared_ptr<DbGroup>> DbGroup::groups;

DbGroup::DbGroup(QString dbName, Backend& _backend) :
        backend(_backend), dbc(backend), config(dbc, backend),
        keys(dbc, backend), pwds(dbc, backend), scb(dbc, backend), tabs(dbc, backend)
{
    dbc.initDatabase(dbName);
}

void DbGroup::createGroup(QString dbName, Backend& backend)
{
    if (groups.count(dbName))
        throw std::runtime_error("DbGroup already created");

    groups[dbName] = std::shared_ptr<DbGroup>(new DbGroup(dbName, backend));
}

std::shared_ptr<DbGroup> DbGroup::getGroup(QString dbName)
{
    if (!groups.count(dbName))
        return nullptr;

    return groups.at(dbName);
}


}
