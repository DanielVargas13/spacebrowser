#ifndef CONTENTFILTER_H_
#define CONTENTFILTER_H_

#include <QWebEngineUrlRequestInterceptor>

#include <map>
#include <memory>
#include <set>

namespace db
{
struct DbGroup;
}

class ContentFilter: public QWebEngineUrlRequestInterceptor
{
    Q_OBJECT

public:
    ContentFilter(QString _dbName);
    virtual ~ContentFilter();

    void interceptRequest(QWebEngineUrlRequestInfo &info);

    std::set<std::string> getUrlsFor(const std::string& url);

public slots:
    void whitelistLocal(QString _dbName, QString site, QString url);
    void whitelistGlobal(QString _dbName, QString url);
    void removeLocal(QString _dbName, QString site, QString url);
    void removeGlobal(QString _dbName, QString url);

private:
    std::map<std::string, std::set<std::string>> requestedScripts;
    QString dbName;

    void filterScripts(QWebEngineUrlRequestInfo& info);
};

#endif /* CONTENTFILTER_H_ */
