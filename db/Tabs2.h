#ifndef DB_TABS2_H_
#define DB_TABS2_H_

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
    };

    Tabs2(DbClient& _dbClient);
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

    /**
     * Fetch all saved tab information
     * @return vector of TabInfo structures describing opened tabs
     */
//    std::map<QString, std::vector<TabInfo>> getAllTabs();

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

};

} /* namespace db */

#endif /* DB_TABS2_H_ */
