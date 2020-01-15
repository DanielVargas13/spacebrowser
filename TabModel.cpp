#include <TabModel.h>

#include <Tab.h>

#include <QLoggingCategory>

#include <QQuickWebEngineProfile>

Q_LOGGING_CATEGORY(tabModelLog, "tabModelLog")


TabModel::TabModel(QString _dbName,
                   QQuickItem* _tabSelector,
                   QQuickItem* _tabSelectorPanel,
                   QQuickItem* _webViewContainer,
                   std::shared_ptr<QQuickWebEngineProfile> _webProfile,
                   std::shared_ptr<db::DbGroup> _dbGroup
    ) :
    dbName(_dbName), webProfile(_webProfile), tabSelector(_tabSelector), tabSelectorPanel(_tabSelectorPanel),
    webViewContainer(_webViewContainer), dbg(_dbGroup)
{

}

TabModel::~TabModel()
{

}

void TabModel::createTab(QString _dbName, int parent, bool select, bool scroll)
{
    if (_dbName == dbName)
        createTab(parent, select, scroll);
}

int TabModel::createTab(int parent, bool select, bool scroll)
{
    std::lock_guard<std::recursive_mutex> lock(views2Mutex);

    /// Call webViewContainer to create new QML Web View object and
    /// create associated entry in tabSelectorModel
    ///
    QVariant newView;
    int viewId = dbg->tabs.createTab();
    dbg->tabs.setParent(viewId, parent);

    QMetaObject::invokeMethod(webViewContainer, "createViewObject",
                              Q_RETURN_ARG(QVariant, newView),
                              Q_ARG(QVariant, viewId));

    QObject* v = qvariant_cast<QObject *>(newView);  // newView / v is WebViewComponent
    v->setProperty("tabModel", QVariant::fromValue<QObject*>(this));
    v->setProperty("profile", QVariant::fromValue<QObject*>(webProfile.get()));
    v->setProperty("dbName", dbName);

    QObject::connect(v, SIGNAL(updateTitle(int, QString)),
                     this, SLOT(titleChanged(int, QString)));
    QObject::connect(v, SIGNAL(updateIcon(int, QString)),
                     this, SLOT(iconChanged(int, QString)));
    QObject::connect(v, SIGNAL(updateUrl(int, QString)),
                     this, SLOT(urlChanged(int, QString)));


    struct viewData& vd = views2[viewId];

    if (vd.tabData)
    {
        qCCritical(tabModelLog, "vd.tabData should be nullptr");
    }

    QStandardItem* par = nullptr;

    if (parent)
    {
        qCDebug(tabModelLog, "createTab: found parent tab: %i", parent);
        par = views2.at(parent).tabData;
    }
    else
        par = invisibleRootItem();

    Tab* item = new Tab(viewId);

    if (par)
        par->appendRow(item);
    else
    {
        qCCritical(tabModelLog, "Could not find parent to add tab to");
    }

    item->updateIndent();
    vd.tabData = item;
    vd.tabData->setView(newView);

    qCDebug(tabModelLog, "Tab created: %i", viewId);

    if (select)
        selectTab(viewId);

    if (scroll)
    {
        QMetaObject::invokeMethod(tabSelectorPanel, "scrollToCurrent",
                                  Qt::ConnectionType::QueuedConnection);
    }

    return viewId;
}

void TabModel::closeTab(QString _dbName, int viewId)
{
    if (_dbName == dbName)
        closeTab(viewId);
}

void TabModel::closeTab(int viewId)
{
    qCDebug(tabModelLog, "Closing tab: %i", viewId);
    std::lock_guard<std::recursive_mutex> lock(views2Mutex);

    if (views2.count(viewId) == 0)
    {
        qCCritical(tabModelLog, "Trying to close nonexistent tab: %i", viewId);
        return;
    }

    /// Remove tab entry from database and from tabSelector component
    ///
    dbg->tabs.closeTab(viewId);

    struct viewData toClose = views2.at(viewId);

    qCDebug(tabModelLog, "title: %s\nurl: %s", toClose.tabData->getTitle().toStdString().c_str(),
            toClose.tabData->getUrl().toStdString().c_str());

    int closedItemRow = toClose.tabData->row();
    QStandardItem* parent = toClose.tabData->parent();
    if (!parent)
        parent = invisibleRootItem();

    if (toClose.tabData->hasChildren())
    {
        qCDebug(tabModelLog, "Has children");

        /// Move children tabs to parents' children, inserting them
        /// at removed tabs' position
        ///
        bool res = moveRows(
            toClose.tabData->index(), 0, toClose.tabData->rowCount(),
            parent->index(), closedItemRow+1);

        qCDebug(tabModelLog, "Moving result: %i", res);
    }

    /// Remove qml view object and then remove entry from model and map
    ///
    QVariant novalue;
    QMetaObject::invokeMethod(webViewContainer, "destroyView",
        Q_RETURN_ARG(QVariant, novalue),
        Q_ARG(QVariant, toClose.tabData->getView()));

    parent->removeRow(closedItemRow);
    views2.erase(viewId);

    /// Optionally create new empty tab if last tab was closed
    ///
    if (views2.empty())
        viewSelected(createTab());
    /// or select
    else if (dbg->config.getProperty("currentTab").toInt() == viewId)
    {
        Tab* item;
        if (closedItemRow == 0)
        {
            item = dynamic_cast<Tab*>(parent);
            if (!item)
                item = dynamic_cast<Tab*>(parent->child(0));
        }
        else
            item = dynamic_cast<Tab*>(parent->child(closedItemRow-1));

        selectTab(item->getId());
        QMetaObject::invokeMethod(tabSelectorPanel, "scrollToCurrent",
                                  Qt::ConnectionType::QueuedConnection);
    }
}

QVariant TabModel::getView(int viewId)
{
    std::lock_guard<std::recursive_mutex> lock(views2Mutex);

    return views2.at(viewId).tabData->getView();
}

void TabModel::viewSelected(QString _dbName, int viewId)
{
    if (dbName == _dbName)
        viewSelected(viewId);
}

void TabModel::viewSelected(int viewId)
{
    std::lock_guard<std::recursive_mutex> lock(views2Mutex);
    struct viewData& vd = views2[viewId];

    /// Create view qml object if it doesn't exist already
    ///
    if (vd.tabData && !vd.tabData->getView().isValid())
    {
        QVariant newView;
        QMetaObject::invokeMethod(webViewContainer, "createViewObject",
            Q_RETURN_ARG(QVariant, newView),
            Q_ARG(QVariant, viewId));

        QObject* v = qvariant_cast<QObject *>(newView);

        const Tab* td = vd.tabData;
        v->setProperty("targetUrl", td->getUrl());
        v->setProperty("tabModel", QVariant::fromValue<QObject*>(this));
        v->setProperty("profile", QVariant::fromValue<QObject*>(webProfile.get()));
        v->setProperty("dbName", dbName);

        QObject::connect(v, SIGNAL(updateTitle(int, QString)),
                         this, SLOT(titleChanged(int, QString)));
        QObject::connect(v, SIGNAL(updateIcon(int, QString)),
                         this, SLOT(iconChanged(int, QString)));
        QObject::connect(v, SIGNAL(updateUrl(int, QString)),
                         this, SLOT(urlChanged(int, QString)));

        vd.tabData->setView(newView);
    }
    else
    {
        if (!vd.tabData)
        {
            qCWarning(tabModelLog, "There was no tabData, "
                "not sure if this should happen");
            qCWarning(tabModelLog, "viewId: %i", viewId);
        }
    }

    /// Update properties and db
    ///
    webViewContainer->setProperty("currentView", vd.tabData->getView());
    dbg->config.setProperty("currentTab", viewId);
}

void TabModel::urlChanged(int viewId, QString url)
{
    dbg->tabs.setUrl(viewId, url);
}

void TabModel::titleChanged(int viewId, QString title)
{
    std::lock_guard<std::recursive_mutex> lock(views2Mutex);
    dbg->tabs.setTitle(viewId, title);
    views2.at(viewId).tabData->setTitle(title);
}

void TabModel::iconChanged(int viewId, QString icon)
{
    std::lock_guard<std::recursive_mutex> lock(views2Mutex);
    dbg->tabs.setIcon(viewId, icon);
    views2.at(viewId).tabData->setIcon(icon);
}

void TabModel::loadTabs()
{
    auto start = std::chrono::system_clock::now();
    std::map<int, db::Tabs2::TabInfo> tabsMap = dbg->tabs.getAllTabsMap();

    /// Open new empty tab if no tabs were retrieved from database
    ///
    if (tabsMap.empty())
    {
        viewSelected(createTab());
        return;
    }

    setItemRoleNames(Tab::roles);
    flatModel.setDbName(dbName);
    flatModel.setRoleNames(Tab::roles);

    /// Fill model and assign to tab selector panel
    ///
    QStandardItem* parent = invisibleRootItem();

    std::deque<std::pair<int, QStandardItem*>> toAdd;

    for (auto child: tabsMap[0].children)
    {
        toAdd.push_back(std::pair<int, QStandardItem*>(child, parent));
    }

    {
        std::lock_guard<std::recursive_mutex> lock(views2Mutex);
        while (!toAdd.empty())
        {
            auto id = toAdd.front();
            toAdd.pop_front();

            Tab* item = new Tab(tabsMap[id.first]);
            id.second->appendRow(item);
            item->updateIndent();
            views2[item->getId()].tabData = item;

            for (auto child: tabsMap[id.first].children)
            {
                toAdd.push_back(std::pair<int, QStandardItem*>(child, item));
            }
        }

        if (views2.size() != (tabsMap.size() - 1)) // -1 because tabsMap has dummy root item
        {
            qCCritical(tabModelLog, "There are %li orphaned tabs in the %s model!",
                       tabsMap.size() - views2.size() - 1, dbName.toStdString().c_str());
        }
    }

    /// Configure models
    ///
    flatModel.setSourceModel(this);

    /// Log timing
    ///
    auto end = std::chrono::system_clock::now();


    std::chrono::duration<double> total = end-start;
    qCDebug(tabModelLog, "Model load time: %lli ns",
            std::chrono::duration_cast<std::chrono::nanoseconds>(total));

}

void TabModel::selectCurrentTab()
{
    QVariant currentTab = dbg->config.getProperty("currentTab");

    if (currentTab.isValid())
    {
        int viewId = currentTab.toInt();

        selectTab(viewId);
    }

    QMetaObject::invokeMethod(tabSelectorPanel, "scrollToCurrent",
                              Qt::ConnectionType::QueuedConnection);
}

void TabModel::nextTab(QString _dbName)
{
    if (_dbName == dbName)
        nextTab();
}

void TabModel::nextTab()
{
    QVariant currentTab = dbg->config.getProperty("currentTab");

    if (currentTab.isValid())
    {
        int viewId = currentTab.toInt();

        int flatId = flatModel.getModelId(viewId);
        flatId += 1;

        if (flatId >= flatModel.rowCount())
            flatId = 0;

        /// Map back to viewId from flatId
        QModelIndex tabsModelIndex = flatModel.mapToSource(flatModel.index(flatId, 0));
        Tab* nextTab = dynamic_cast<Tab*>(itemFromIndex(tabsModelIndex));

        selectTab(nextTab->getId());
    }
}

void TabModel::prevTab(QString _dbName)
{
    if (_dbName == dbName)
        prevTab();
}

void TabModel::prevTab()
{
    QVariant currentTab = dbg->config.getProperty("currentTab");

    if (currentTab.isValid())
    {
        int viewId = currentTab.toInt();

        int flatId = flatModel.getModelId(viewId);
        flatId -= 1;

        if (flatId < 0)
            flatId = flatModel.rowCount() - 1;

        /// Map back to viewId from flatId
        QModelIndex tabsModelIndex = flatModel.mapToSource(flatModel.index(flatId, 0));
        Tab* prevTab = dynamic_cast<Tab*>(itemFromIndex(tabsModelIndex));

        selectTab(prevTab->getId());
    }
}

void TabModel::selectTab(int viewId)
{
    int modelId;
    try
    {
        modelId = flatModel.getModelId(viewId);
    }
    catch (std::out_of_range& e)
    {
        qCCritical(tabModelLog, "selectTab(viewId=%i): no view in model", viewId);
        return;
    }

    qCDebug(tabModelLog, "selectTab(viewId=%i, modelId=%i)", viewId, modelId);

    QMetaObject::invokeMethod(tabSelector, "selectView",
            Q_ARG(QVariant, modelId));

    viewSelected(viewId);
}

int TabModel::getFlatModelId(int viewId) const
{
    return flatModel.getModelId(viewId);
}

TreeToListProxyModel* TabModel::getFlatModel()
{
    return &flatModel;
}

void TabModel::updateParent(const Tab& tab, int parentId)
{
    dbg->tabs.setParent(tab.getId(), parentId);
}
