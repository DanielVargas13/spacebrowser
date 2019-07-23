#include <TabModel.h>

#include <Tab.h>

#include <QLoggingCategory>

Q_LOGGING_CATEGORY(tabModelLog, "tabModelLog")


TabModel::TabModel(std::shared_ptr<QQuickView> _qView) : qView(_qView)
{
#ifndef TEST_BUILD
    QObject::connect(qView->rootObject(), SIGNAL(createTab(int, bool, bool)),
                     this, SLOT(createTab(int, bool, bool)));
    QObject::connect(qView->rootObject(), SIGNAL(closeTab(int)),
                     this, SLOT(closeTab(int)));
    QObject::connect(qView->rootObject(), SIGNAL(nextTab()),
                     this, SLOT(nextTab()));
    QObject::connect(qView->rootObject(), SIGNAL(prevTab()),
                     this, SLOT(prevTab()));

    webViewContainer = qobject_cast<QQuickItem*>(
        qView->rootObject()->findChild<QObject*>("webViewContainer"));
    if (!webViewContainer)
        throw std::runtime_error("No webViewContainer object found");

    QObject::connect(webViewContainer, SIGNAL(titleChanged(int, QString)),
                     this, SLOT(titleChanged(int, QString)));
    QObject::connect(webViewContainer, SIGNAL(iconChanged(int, QString)),
                     this, SLOT(iconChanged(int, QString)));
    QObject::connect(webViewContainer, SIGNAL(urlChanged(int, QString)),
                     this, SLOT(urlChanged(int, QString)));

    tabSelector = qobject_cast<QQuickItem*>(
        qView->rootObject()->findChild<QObject*>("tabSelector"));
    if (!tabSelector)
        throw std::runtime_error("No tabSelector object found");

    tabSelectorPanel = qobject_cast<QQuickItem*>(
        qView->rootObject()->findChild<QObject*>("tabSelectorPanel"));

    if (!tabSelector)
        throw std::runtime_error("No tabSelectorPanel object found");

    // FIXME: multiple tab selectors for multiple db backends
    QObject::connect(tabSelector, SIGNAL(viewSelected(int)), this, SLOT(viewSelected(int)));
    QObject::connect(tabSelector, SIGNAL(closeTab(int)), this, SLOT(closeTab(int)));

#else
#endif
}

TabModel::~TabModel()
{

}

int TabModel::createTab(int parent, bool select, bool scroll)
{
    std::lock_guard<std::recursive_mutex> lock(views2Mutex);

    /// Call webViewContainer to create new QML Web View object and
    /// create associated entry in tabSelectorModel
    ///
    QVariant newView;
    int viewId = dbh->tabs.createTab();
    dbh->tabs.setParent(viewId, parent);

    QMetaObject::invokeMethod(webViewContainer, "createViewObject",
        Q_RETURN_ARG(QVariant, newView),
        Q_ARG(QVariant, viewId));

    QObject* v = qvariant_cast<QObject *>(newView);
    v->setProperty("tabModel", QVariant::fromValue<QObject*>(this));

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
    dbh->tabs.closeTab(viewId);

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
    else if (dbh->config.getProperty("currentTab").toInt() == viewId)
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

void TabModel::viewSelected(int viewId)
{
    qCDebug(tabModelLog, "Selecting tab: %i", viewId);

    std::lock_guard<std::recursive_mutex> lock(views2Mutex);

    struct viewData& vd = views2[viewId];

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

    webViewContainer->setProperty("currentView", vd.tabData->getView());
    dbh->config.setProperty("currentTab", viewId);
}

void TabModel::urlChanged(int viewId, QString url)
{
    dbh->tabs.setUrl(viewId, url);
}

void TabModel::titleChanged(int viewId, QString title)
{
    std::lock_guard<std::recursive_mutex> lock(views2Mutex);
    dbh->tabs.setTitle(viewId, title);
    views2.at(viewId).tabData->setTitle(title);
}

void TabModel::iconChanged(int viewId, QString icon)
{
    std::lock_guard<std::recursive_mutex> lock(views2Mutex);
    dbh->tabs.setIcon(viewId, icon);
    views2.at(viewId).tabData->setIcon(icon);
}

void TabModel::loadTabs()
{
    auto start = std::chrono::system_clock::now();
    std::map<int, db::Tabs2::TabInfo> tabsMap = dbh->tabs.getAllTabsMap();

    /// Open new empty tab if no tabs were retrieved from database
    ///
    if (tabsMap.empty())
    {
        viewSelected(createTab());
        return;
    }

    setItemRoleNames(Tab::roles);
    flatModel.setRoleNames(Tab::roles);

    /// Fill model and assign to tab container
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
    }

    /// Configure models
    ///
    flatModel.setSourceModel(this);

    QVariant qv = QVariant::fromValue<QObject*>(&flatModel);
    QMetaObject::invokeMethod(tabSelectorPanel, "setModel",
                              Qt::ConnectionType::QueuedConnection,
                              Q_ARG(QVariant, qv));

    /// Log timing
    ///
    auto end = std::chrono::system_clock::now();


    std::chrono::duration<double> total = end-start;
    qCDebug(tabModelLog, "Model load time: %lli ns",
            std::chrono::duration_cast<std::chrono::nanoseconds>(total));

}

void TabModel::selectCurrentTab()
{
    QVariant currentTab = dbh->config.getProperty("currentTab");

    if (currentTab.isValid())
    {
        int viewId = currentTab.toInt();

        selectTab(viewId);
    }

    QMetaObject::invokeMethod(tabSelectorPanel, "scrollToCurrent",
                              Qt::ConnectionType::QueuedConnection);
}

void TabModel::nextTab()
{
    QVariant currentTab = dbh->config.getProperty("currentTab");

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

void TabModel::prevTab()
{
    QVariant currentTab = dbh->config.getProperty("currentTab");

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
    QVariant selected;
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

    if (selected.toInt() < 0)
        return;

    viewSelected(viewId);
}

int TabModel::getFlatModelId(int viewId) const
{
    return flatModel.getModelId(viewId);
}
