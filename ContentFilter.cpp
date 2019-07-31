#include <ContentFilter.h>

#include <conf/conf.h>
#include <db/DbGroup.h>

#include <QSettings>

ContentFilter::ContentFilter()
{

}

ContentFilter::~ContentFilter()
{

}

void ContentFilter::interceptRequest(QWebEngineUrlRequestInfo& info)
{
    switch(info.resourceType())
    {
    case QWebEngineUrlRequestInfo::ResourceTypeScript : {
        filterScripts(info);
    } break;
    default: {
        return;
    }
    }

    return;
}

std::set<std::string> ContentFilter::getUrlsFor(const std::string& url)
{
    if (requestedScripts.count(url))
        return requestedScripts.at(url);

    return {};
}

void ContentFilter::filterScripts(QWebEngineUrlRequestInfo& info)
{
    QString firstPartyHost = info.firstPartyUrl().host();
    QString requestUrlHost = info.requestUrl().host();
    QStringList firstParty = firstPartyHost.split('.');
    QStringList requestUrl = requestUrlHost.split('.');

    /// Check if requested script is from current page's domain or subdomain
    /// and accept if it is so, continue checks otherwise.
    ///
    if (requestUrl.size() >= firstParty.size())
    {
        bool accept = true;
        for (int i = 1; i < firstParty.size(); ++i)
        {
            if (firstParty[firstParty.size()-i] != requestUrl[requestUrl.size()-i])
            {
                accept = false;
                break;
            }
        }
        if (accept)
            return;
    }

    /// Save info about requested script for given url to allow
    /// configuration of whitelisting
    ///
    requestedScripts[firstPartyHost.toStdString()].insert(requestUrlHost.toStdString());

    /// Check whitelists in database
    ///
    auto dbh = getDefDbGroup();
    if (dbh)
    {
        db::ScriptBlock2::State allowed = dbh->scb.isAllowed(firstPartyHost,
                                                             requestUrlHost);
        
        if (allowed == db::ScriptBlock2::State::Blocked)
            info.block(true);
    }
    else
        info.block(true);

    return;
}

void ContentFilter::whitelistLocal(const QString& site, const QString& url)
{
    auto dbh = getDefDbGroup();
    if (dbh)
    {
        dbh->scb.whitelistLocal(site, url);
    }
}

void ContentFilter::whitelistGlobal(const QString& url)
{
    auto dbh = getDefDbGroup();        
    if (dbh)
    {
        dbh->scb.whitelistGlobal(url);
    }
}

void ContentFilter::removeLocal(const QString& site, const QString& url)
{
    auto dbh = getDefDbGroup();        
    if (dbh)
    {
        dbh->scb.removeLocal(site, url);
    }
}

void ContentFilter::removeGlobal(const QString& url)
{
    auto dbh = getDefDbGroup();
        
    if (dbh)
    {
        dbh->scb.removeGlobal(url);
    }
}

std::shared_ptr<db::DbGroup> ContentFilter::getDefDbGroup()
{
    QSettings settings;
    if (settings.contains(conf::Databases::defContFiltDb))
    {
        QString dbName = settings.value(conf::Databases::defContFiltDb).toString();
        return db::DbGroup::getGroup(dbName);
    }        

    return nullptr;
}
