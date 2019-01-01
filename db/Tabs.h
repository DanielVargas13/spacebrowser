#ifndef DB_TABS_H_
#define DB_TABS_H_

#include <pqxx/pqxx>

#include <map>
#include <string>
#include <vector>

namespace db
{

/**
 * Class for handling tabs in database
 */
class Tabs
{
public:

    struct TabInfo
    {
        int id;
        int parent;
        std::string url;
        std::string title;
        std::string icon; // FIXME: add zoom level
    };

    Tabs();
    virtual ~Tabs();

    /**
     * Create tab entry in the database
     * @return unique tab id
     */
    int32_t createTab();

    /**
     * Remove database entry for tab
     * @param viewId id of tab to be removed
     */
    void closeTab(int viewId);

    /**
     * Fetch all saved tab information
     * @return vector of TabInfo structures describing opened tabs
     */
    std::vector<TabInfo> getAllTabs();
    std::map<int, TabInfo> getAllTabsMap();

    /**
     * Set parent for tab
     * @param viewId id of tab to be updated
     * @param parentId new parent id value
     */
    void setParent(int viewId, int parentId);

    /**
     * Set url entry for tab
     * @param viewId id of tab to be updated
     * @param url new url value
     */
    void setUrl(int viewId, std::string url);

    /**
     * Set title for tab
     * @param viewId id of tab to be updated
     * @param title new title value
     */
    void setTitle(int viewId, std::string title);

    /**
     * Set icon for tab
     * @param viewId id of tab to be updated
     * @param icon new icon value
     */
    void setIcon(int viewId, std::string icon);

private:
    static std::string tableName;
    pqxx::connection conn;
};

} /* namespace db */

#endif /* DB_TABS_H_ */
