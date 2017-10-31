#include <ContentFilter.h>
#include <db/ScriptBlock.h>

#include <iostream>

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
        for (unsigned int i = 1; i < firstParty.size(); ++i)
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
    db::ScriptBlock::State allowed = sBlock.isAllowed(firstPartyHost.toStdString(),
            requestUrlHost.toStdString());

    if (allowed == db::ScriptBlock::State::Blocked)
        info.block(true);

    return;
}

void ContentFilter::whitelistLocal(const QString& site, const QString& url)
{
    sBlock.whitelistLocal(site.toStdString(), url.toStdString());
}

void ContentFilter::whitelistGlobal(const QString& url)
{
    sBlock.whitelistGlobal(url.toStdString());
}

void ContentFilter::removeLocal(const QString& site, const QString& url)
{
    sBlock.removeLocal(site.toStdString(), url.toStdString());
}

void ContentFilter::removeGlobal(const QString& url)
{
    sBlock.removeGlobal(url.toStdString());
}
