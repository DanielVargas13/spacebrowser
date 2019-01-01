#ifndef VIEWHANDLER_H_
#define VIEWHANDLER_H_

#include <db/Tabs.h>
#include <db/Config.h>
#include <db/ScriptBlock.h>
#include <ContentFilter.h>
#include <TreeModel.h>

#include <QObject>
#include <QQuickItem>
#include <QQuickView>

#include <map>
#include <mutex>
#include <vector>

class QQuickWebEngineHistory;

/**
 * Class that handles tab tree structure and WebEngineView objects associated with tabs
 * View ID used in here is the same id as generated by database via Tabs object
 */
class ViewHandler : public QObject
{
    Q_OBJECT

public:
    /**
     * Creates ViewHandler object associated with webViewContainer
     * @param _webViewContainer pointer to QML instantiated WebViewContainer object
     * @param _tabSelector pointer to QML instantiated TabSelector object
     * @param _scriptBlockingView pointer to QML instantiated ScriptBlockingView object
     * @param _cf reference to ContentFilter class that provides interface for handling script blocking etc.
     * @param _qView shared pointer to the main window QQuickView object
     */
    ViewHandler(QQuickItem* _webViewContainer, QQuickItem* _tabSelector,
            QQuickItem* _scriptBlockingView, ContentFilter& _cf, std::shared_ptr<QQuickView> _qView);

    virtual ~ViewHandler();

public slots:

    /**
     * Initializes ViewHandler. This function should be called after at least one
     * database connection is established. Can be called multiple times, e.g. when
     * new database connection is established, or some connection was lost.
     */
    bool init();

    /**
     * Create new QML WebEngineView object
     * @param parent optional id of parent tab
     * @return id of the newly created tab
     */
    int createTab(int parent = 0);

    /**
     * Removes tab entry from the database, destructs associated WebEngineView object,
     * fixes tab hierarchy and indentation. If the last tab is closed it automatically
     * creates new empty tab (call to createTab(0) )
     * @param viewId id of tab to be closed
     */
    void closeTab(int viewId);

    /**
     * Returns WebEngineView QML object associated with viewId
     * @param viewId id of tab / view
     * @return returns QVariant with WebEngineView QML object, throws if viewId is invalid
     */
    QVariant getView(int viewId);

    /**
     * Sets the selected view as currently visible
     * @param viewId viewId id of tab / view
     */
    void viewSelected(int viewId); // FIXME: rename to selectView

    /**
     * Updates url database entry for given tab
     * @param viewId id of tab to be updated
     * @param url new url value
     */
    void urlChanged(int viewId, QUrl url);

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
    void iconChanged(int viewId, QUrl icon);

    /**
     * Load all tabs that were stored in the database.
     * If there are no tabs saved, open new empty one.
     */
    void loadTabs();

    /**
     * Switch to next tab
     */
    void nextTab();

    /**
     * Switcth to previous tab
     */
    void prevTab();

    /**
     * Mark selected tab as active on TabSelectorPanel and make selected view visible
     * @param viewId viewId id of tab / view
     */
    void selectTab(int viewId);

    /**
     * Show view allowing modification of script blocking rules for site opened
     * in selected tab
     * @param viewId id of tab / view for which list of blocked script sources will be shown
     */
    void openScriptBlockingView(int viewId);

    /**
     * Enter fullscreen mode
     * @param fullscreen If true application will enter fullscreen mode, if false - the opposite
     */
    void showFullscreen(bool fullscreen = true);

    // deprecated, does not work
    void historyUpdated(int _viewId, QQuickWebEngineHistory* navHistory);

private:
    db::Tabs tabsDb;                     /// Tabs database abstraction
    db::Config configDb;                 /// Config database abstraction
    db::ScriptBlock sBlockDb;            /// Script blocker database abstraction
    QQuickItem* webViewContainer;        /// Pointer to WebViewContainer QML object
    QQuickItem* tabSelector;             /// Pointer to TabSelector QML object
    QQuickItem* scriptBlockingView;      /// Pointer to ScriptBlockingView QML object
    ContentFilter& cf;                   /// Reference to content filtering class
    std::shared_ptr<QQuickView> qView;   /// Smart pointer to main window object

    TreeModel tabsModel;                 /// Model for holding tab related data

    /// Structure for holding WebViewContainer QML object and
    /// accompanying tab meta information
    ///
    struct viewContainer
    {
        QVariant view;          /// QVariant pointing to QML WebViewContainer object
        int parent;             /// id of parent tab or 0 if there is no parent
        std::vector<int> children;   /// vector of child tab ids
    };

    [[deprecated]] mutable std::recursive_mutex viewsMutex;
    [[deprecated]] std::map<int, viewContainer> views;   /// Structure representing tab tree

    /**
     * Count ancestors of current tab starting with parent
     * @param parent id of parent tab
     * @return 0 if there is no parent (parent==0), number of ancestors otherwise
     */
    int countAncestors(int parent) const;

    /**
     * Remove view from parent's children, and add it's children to parent's
     * @param viewId id of tab for which hierarchy structure needs to be fixed
     */
    void fixHierarchy(int viewId);

    /**
     * Update indentation level for all descendants of this tab
     * @param viewId id of tab for which indentation needs to be fixed
     */
    void fixIndentation(int viewId);
};

#endif /* VIEWHANDLER_H_ */
