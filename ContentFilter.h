#ifndef CONTENTFILTER_H_
#define CONTENTFILTER_H_

#include <db/DbGroup.h>

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

    // FIXME: this is temporary, remove:
    void setGrp(db::DbGroup* grp)
    {
        dbh = grp;
    }

public slots:
    void whitelistLocal(const QString& site, const QString& url);
    void whitelistGlobal(const QString& url);
    void removeLocal(const QString& site, const QString& url);
    void removeGlobal(const QString& url);

private:
    std::map<std::string, std::set<std::string>> requestedScripts;
    db::DbGroup* dbh;

    void filterScripts(QWebEngineUrlRequestInfo& info);
};

#endif /* CONTENTFILTER_H_ */
