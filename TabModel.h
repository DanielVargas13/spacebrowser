#ifndef TABMODEL_H_
#define TABMODEL_H_

#include <TreeModel2.h>
#include <TreeToListProxyModel.h>
#include <db/DbGroup.h>

#include <QObject>
#include <QQuickItem>
#include <QQuickView>

#include <memory>
#include <mutex>

class Tab;  /// Defined in Tab.h
class QQuickWebEngineProfile;

class TabModel: public TreeModel2
{
    Q_OBJECT

public:
// FIXME: qView doesnt belong here, rip it out
    TabModel(std::shared_ptr<QQuickView> _qView, QString _dbName,
             std::shared_ptr<QQuickWebEngineProfile> _webProfile);
    virtual ~TabModel();

public slots:
    /**
     * Create new QML WebEngineView object
     * @param dbName name of database backend
     * @param parent optional id of parent tab
     * @param select if true, newly created tab will be selected
     * @param scroll if true, tabSelector will be repositioned, to show new tab
     * @return id of the newly created tab
     */
    void createTab(QString _dbName, int parent = 0, bool select = false, bool scroll = false);
    int createTab(int parent = 0, bool select = false, bool scroll = false);

    /**
     * Removes tab entry from the database, destructs associated WebEngineView object,
     * fixes tab hierarchy and indentation. If the last tab is closed it automatically
     * creates new empty tab (call to createTab(0) )
     * @param dbName name of db backend
     * @param viewId id of tab to be closed
     */
    void closeTab(QString _dbName, int viewId);
    void closeTab(int viewId);

    /**
     * Sets the selected view as currently visible
     * @param dbName name of db backend
     * @param viewId viewId id of tab / view
     */
    void viewSelected(QString _dbName, int viewId);
    void viewSelected(int viewId);

    /**
     * Updates url database entry for given tab
     * @param viewId id of tab to be updated
     * @param url new url value
     */
    void urlChanged(int viewId, QString url);

    /**
     * Updates title database entry for given tab
     * @param viewId id of tab to be updated
     * @param title new title value
     */
    void titleChanged(int viewId, QString title);

    /**
     * Updates icon database entry for given tab
     * @param viewId id of tab to be updated
     * @param icon new icon value
     */
    void iconChanged(int viewId, QString icon);

    /**
     * Switch to next tab
     * @param dbName name of db backend
     */
    void nextTab(QString _dbName);
    void nextTab();

    /**
     * Switcth to previous tab
     * @param dbName name of db backend
     */
    void prevTab(QString _dbName);
    void prevTab();

    /**
     * Returns WebEngineView QML object associated with viewId
     * @param viewId id of tab / view
     * @return returns QVariant with WebEngineView QML object, throws if viewId is invalid
     */
    QVariant getView(int viewId);

public:
    /**
     * Load all tabs that were stored in the database.
     * If there are no tabs saved, open new empty one.
     */
    void loadTabs();

    /**
     * Select current tab.
     * This function needs to be called after Qt events are processed after
     * calling loadTabs()
     */
    void selectCurrentTab();

    /**
     * Mark selected tab as active on TabSelectorPanel and make selected view visible
     * @param viewId viewId id of tab / view
     */
    void selectTab(int viewId);

    /**
     * Find flatModel id of viewId
     * @param viewId id of tab / view
     */
    int getFlatModelId(int viewId) const;

    /**
     * Returns flat representation of model
     */
    QAbstractItemModel* getFlatModel();

private:
#ifndef TEST_BUILD
// FIXME: not sure if QQuickItems belong here
    QQuickItem* webViewContainer;             /// Pointer to WebViewContainer QML object
    QQuickItem* tabSelector;                  /// Pointer to TabSelector QML object
    QQuickItem* tabSelectorPanel;             /// Pointer to TabSelectorPanel QML object

#else
#endif
    std::shared_ptr<QQuickView> qView;        /// Smart pointer to main window object
    QString dbName;                           /// Name of db backend to be used
    std::shared_ptr<QQuickWebEngineProfile> webProfile; /// Profile to be set upon creating view

    struct viewData
    {
        Tab* tabData = nullptr;
    };

    std::map<int, viewData> views2;           /// viewId to Tab item mapping
    mutable std::recursive_mutex views2Mutex;
    TreeToListProxyModel flatModel;           /// List model for ListView
};

#endif
