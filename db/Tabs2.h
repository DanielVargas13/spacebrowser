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
class Tabs2 : public DbClient
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

    Tabs2();
    virtual ~Tabs2();

    /**
     * Perform any required db initializations (table creation / update)
     * @param dbName name of database connection
     */
    bool initDatabase(QString dbName);

    /**
     * Create tab entry in the database
     * @param dbName name of database connection
     * @return unique tab id
     */
    int32_t createTab(QString dbName);

    /**
     * Remove database entry for tab
     * @param dbName name of database connection
     * @param tabId id of tab to be removed
     */
    void closeTab(QString dbName, int tabId);

    /**
     * Fetch all saved tab information
     * @param dbName name of database connection
     * @return vector of TabInfo structures describing opened tabs
     */
    std::vector<TabInfo> getAllTabs(QString dbName);

    /**
     * Fetch all saved tab information
     * @return vector of TabInfo structures describing opened tabs
     */
    std::map<QString, std::vector<TabInfo>> getAllTabs();

    /**
     * Set parent for tab
     * @param dbName name of database connection
     * @param tabId id of tab to be updated
     * @param parentId new parent id value
     */
    void setParent(QString dbName, int tabId, int parentId);

    /**
     * Set url entry for tab
     * @param dbName name of database connection
     * @param tabId id of tab to be updated
     * @param url new url value
     */
    void setUrl(QString dbName, int tabId, QString url);

    /**
     * Set title for tab
     * @param dbName name of database connection
     * @param tabId id of tab to be updated
     * @param title new title value
     */
    void setTitle(QString dbName, int tabId, QString title);

    /**
     * Set icon for tab
     * @param dbName name of database connection
     * @param tabId id of tab to be updated
     * @param icon new icon value
     */
    void setIcon(QString dbName, int tabId, QString icon);

private:
    static QString tableName;

};

} /* namespace db */

#endif /* DB_TABS2_H_ */
