#include <db/Tabs.h>

#include <db/Sql.h>

#include <pqxx/pqxx>
#include <QString>

#include <iostream>

namespace db
{

namespace sql
{

static QString insert("INSERT INTO \"%1\".\"%2\" DEFAULT VALUES RETURNING \"%3\";");

// since url can have %x sequences it has to be filled in last
static QString update("UPDATE \"%1\".\"%2\" SET \"%5\" = \'%6\' WHERE \"%3\" = %4;");

static QString deleteRow("DELETE FROM \"%1\".\"%2\" WHERE \"%3\" = %4;");

}

std::string Tabs::tableName("tabs");

Tabs::Tabs()
{
    sql::Helpers::createSchemaIfNotExists(conn, sql::schemaName);

    sql::Helpers::createTableIfNotExists(conn, sql::schemaName, tableName.c_str(),
            "id serial, parent integer default 0, url varchar default \'\',"
            " title varchar default \'\', icon varchar default \'\',"
            " CONSTRAINT tabs_pkey PRIMARY KEY (id)");
}

Tabs::~Tabs()
{

}

int32_t Tabs::createTab()
{
    pqxx::nontransaction ntx(conn);

    pqxx::result r = ntx.exec(sql::insert.arg(sql::schemaName).arg(Tabs::tableName.c_str())
            .arg("id").toStdString());

    if (r.size() != 1)
    {
        throw std::runtime_error(
                "Tabs::createTab(): PSQL returned " + std::to_string(r.size()) + " rows");
    }

    return r[0][0].as<int>();
}

void Tabs::closeTab(int viewId)
{
    // FIXME: in the future mark as closed, but remember history of past tabs
    pqxx::nontransaction ntx(conn);

    pqxx::result r = ntx.exec(sql::deleteRow.arg(sql::schemaName).arg(Tabs::tableName.c_str())
            .arg("id").arg(viewId).toStdString());
}

std::vector<Tabs::TabInfo> Tabs::getAllTabs()
{
    pqxx::nontransaction ntx(conn);
    // FIXME: ORDER BY id will not work when hierarchy modifications will be possible
    //        a new column describing actual order is necessary
    pqxx::result r = ntx.exec(sql::select.arg("id,parent,url,title,icon").arg(sql::schemaName).
            arg(Tabs::tableName.c_str()).arg("ORDER BY id").toStdString());

    std::vector<TabInfo> tabs;

    for (pqxx::result::size_type i = 0; i < r.size(); ++i)
    {
        TabInfo tab;
        tab.id = r[i][0].as<int>();
        tab.parent = r[i][1].as<int>();
        tab.url = r[i][2].as<std::string>();
        tab.title = r[i][3].as<std::string>();
        tab.icon = r[i][4].as<std::string>();

        tabs.push_back(tab);
    }

    return tabs;
}

void Tabs::setParent(int viewId, int parentId)
{
    pqxx::nontransaction ntx(conn);

    pqxx::result r = ntx.exec(sql::update.arg(sql::schemaName).arg(Tabs::tableName.c_str())
            .arg("id").arg(viewId).arg("parent").arg(parentId).toStdString());
}

void Tabs::setUrl(int viewId, std::string url)
{
    pqxx::nontransaction ntx(conn);

    const std::string& eurl = ntx.esc(url);

    pqxx::result r = ntx.exec(sql::update.arg(sql::schemaName).arg(Tabs::tableName.c_str())
            .arg("id").arg(viewId).arg("url").arg(eurl.c_str()).toStdString());
}

void Tabs::setTitle(int viewId, std::string title)
{
    pqxx::nontransaction ntx(conn);

    const std::string& etitle = ntx.esc(title);

    pqxx::result r = ntx.exec(sql::update.arg(sql::schemaName).arg(Tabs::tableName.c_str())
            .arg("id").arg(viewId).arg("title").arg(etitle.c_str()).toStdString());
}

void Tabs::setIcon(int viewId, std::string icon)
{
    pqxx::nontransaction ntx(conn);

    const std::string& eicon = ntx.esc(icon);

    pqxx::result r = ntx.exec(sql::update.arg(sql::schemaName).arg(Tabs::tableName.c_str())
            .arg("id").arg(viewId).arg("icon").arg(eicon.c_str()).toStdString());
}


} /* namespace db */
