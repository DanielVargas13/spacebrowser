#ifndef CONTENTFILTER_H_
#define CONTENTFILTER_H_

#include <db/ScriptBlock.h>

#include <QWebEngineUrlRequestInterceptor>

#include <map>
#include <set>

class ContentFilter: public QWebEngineUrlRequestInterceptor
{
    Q_OBJECT

public:
    ContentFilter();
    virtual ~ContentFilter();

    void interceptRequest(QWebEngineUrlRequestInfo &info);

    std::set<std::string> getUrlsFor(const std::string& url);

public slots:
    void whitelistLocal(const QString& site, const QString& url);
    void whitelistGlobal(const QString& url);
    void removeLocal(const QString& site, const QString& url);
    void removeGlobal(const QString& url);

private:
    std::map<std::string, std::set<std::string>> requestedScripts;
    db::ScriptBlock sBlock;

    void filterScripts(QWebEngineUrlRequestInfo& info);
};

#endif /* CONTENTFILTER_H_ */
