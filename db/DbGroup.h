#ifndef DB_DBGROUP_H_
#define DB_DBGROUP_H_

#include <db/Backend.h>
#include <db/DbClient.h>
#include <db/Config2.h>
#include <db/Keys2.h>
#include <db/Passwords2.h>
#include <db/ScriptBlock2.h>
#include <db/Tabs2.h>

namespace db
{

struct DbGroup
{
    DbGroup(QString dbName, Backend& _backend) :
        backend(_backend), dbc(backend), config(dbc, backend),
        keys(dbc, backend), pwds(dbc, backend), scb(dbc, backend), tabs(dbc, backend)
    {
        dbc.initDatabase(dbName);
    }

    db::Backend& backend;
    db::DbClient dbc;
    db::Config2 config;
    db::Keys2 keys;
    db::Passwords2 pwds;
    db::ScriptBlock2 scb;
    db::Tabs2 tabs;
};

}

#endif
