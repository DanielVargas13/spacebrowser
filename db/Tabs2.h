#ifndef DB_TABS2_H_
#define DB_TABS2_H_

#include <db/Backend.h>
#include <db/DbClient.h>

#include <QString>

#include <map>
#include <vector>

namespace db
{

/**
 * Class for handling tabs in database
 */
class Tabs2
{
public:

    struct TabInfo
    {
        int id;
        int parent;
        QString url;
        QString title;
        QString icon; // FIXME: add zoom level
        std::vector<int> children;
    };

    Tabs2(DbClient& _dbClient, Backend& _backend);
    virtual ~Tabs2();

    /**
     * Create tab entry in the database
     * @return unique tab id
     */
    int32_t createTab();

    /**
     * Remove database entry for tab
     * @param tabId id of tab to be removed
     */
    void closeTab(int tabId);

    /**
     * Fetch all saved tab information
     * @return vector of TabInfo structures describing opened tabs
     */
    std::vector<TabInfo> getAllTabs();
    std::map<int, TabInfo> getAllTabsMap();

    /**
     * Set parent for tab
     * @param tabId id of tab to be updated
     * @param parentId new parent id value
     */
    void setParent(int tabId, int parentId);

    /**
     * Set url entry for tab
     * @param tabId id of tab to be updated
     * @param url new url value
     */
    void setUrl(int tabId, QString url);

    /**
     * Set title for tab
     * @param tabId id of tab to be updated
     * @param title new title value
     */
    void setTitle(int tabId, QString title);

    /**
     * Set icon for tab
     * @param tabId id of tab to be updated
     * @param icon new icon value
     */
    void setIcon(int tabId, QString icon);

private:
    static QString tableName;
    DbClient& dbClient;
    Backend& backend;

};

} /* namespace db */

Q_DECLARE_METATYPE(std::vector<db::Tabs2::TabInfo>)
typedef std::map<int, db::Tabs2::TabInfo> tabMap_t;
Q_DECLARE_METATYPE(tabMap_t)

#endif /* DB_TABS2_H_ */
