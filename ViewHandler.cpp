#include <ViewHandler.h>

#include <Tab.h>
#include <misc/DebugHelpers.h>

#include <QAbstractListModel>
#include <QJSEngine>
#include <QJSValue>
#include <QModelIndex>
//#include <QtWebEngine/5.9.1/QtWebEngine/private/qquickwebenginehistory_p.h>

#include <chrono>
#include <deque>

Q_LOGGING_CATEGORY(vhLog, "viewHandler")

ViewHandler::ViewHandler(QQuickItem* _webViewContainer, QQuickItem* _tabSelector,
        QQuickItem* _scriptBlockingView, ContentFilter& _cf, std::shared_ptr<QQuickView> _qView)
    : webViewContainer(_webViewContainer), tabSelector(_tabSelector),
      scriptBlockingView(_scriptBlockingView), cf(_cf), qView(_qView)
{

}

ViewHandler::~ViewHandler()
{

}

bool ViewHandler::init()
{
    // FIXME: here initialize db::Tabs, and other db:: in the future

    return true;
}

void ViewHandler::viewSelected(int viewId)
{
    qCDebug(vhLog, "Selecting tab: %i", viewId);

    std::lock_guard<std::recursive_mutex> lock(views2Mutex);

    struct viewData& vd = views2[viewId];

    if (vd.tabData && !vd.tabData->getView().isValid())
    {
        QVariant newView;
        QMetaObject::invokeMethod(webViewContainer, "createViewObject",
            Q_RETURN_ARG(QVariant, newView),
            Q_ARG(QVariant, viewId));

        QObject* v = qvariant_cast<QObject *>(newView);

        if (vd.tabData)
        {
            const Tab* td = vd.tabData;
            v->setProperty("targetTitle", td->getTitle());
            v->setProperty("targetIcon", td->getIcon());
            v->setProperty("targetUrl", td->getUrl());
        }

        vd.tabData->setView(newView);
    }
    else
    {
        if (!vd.tabData)
        {
            qCWarning(vhLog, "There was no tabData, "
                "not sure if this should happen");
            qCWarning(vhLog, "viewId: %i", viewId);
        }
    }

    webViewContainer->setProperty("currentView", vd.tabData->getView());
    configDb.setProperty("currentTab", std::to_string(viewId));
}
/*
int ViewHandler::countAncestors(int parent) const
{
    if (parent < 0)
        throw std::runtime_error("ViewHandler::countAncestors(): parent id cannot be negative");

    if (!parent)
        return 0;

    {
        std::lock_guard<std::recursive_mutex> lock(viewsMutex);

        int indentLevel = 1;

        while (parent && views.count(parent) && views.at(parent).parent)
        {
            ++indentLevel;
            parent = views.at(parent).parent;
        }

        return indentLevel;
    }
}
*/
int ViewHandler::createTab(int parent)
{
    std::lock_guard<std::recursive_mutex> lock(views2Mutex);

    /// Call webViewContainer to create new QML Web View object and
    /// create associated entry in tabSelectorModel
    ///
    QVariant newView;
    int viewId = tabsDb.createTab();
    tabsDb.setParent(viewId, parent);

    QMetaObject::invokeMethod(webViewContainer, "createViewObject",
        Q_RETURN_ARG(QVariant, newView),
        Q_ARG(QVariant, viewId));

    struct viewData& vd = views2[viewId];

    if (vd.tabData)
    {
        qCCritical(vhLog, "vd.tabData should be nullptr");
    }

    Tab* par = nullptr;
    if (parent)
        par = views2.at(parent).tabData;

    Tab* item = new Tab(viewId);

    if (par)
        par->appendRow(item);

    item->updateIndent();
    vd.tabData = item;
    vd.tabData->setView(newView);

    return viewId;
}

QVariant ViewHandler::getView(int viewId)
{
    std::lock_guard<std::recursive_mutex> lock(viewsMutex);

    return views.at(viewId).view;
}

/*
void ViewHandler::fixHierarchy(int viewId)
{
    auto& thisView = views.at(viewId);

    /// Fix parent entries for children of this view
    ///
    for (auto& child: thisView.children)
    {
        tabsDb.setParent(child, thisView.parent);
        views.at(child).parent = thisView.parent;
    }

    /// Move children of this view to the parent's list of children
    ///
    if (thisView.parent)
    {
        auto& children = views.at(thisView.parent).children;
        auto pos = std::find(children.begin(), children.end(), viewId);
        children.insert(pos, thisView.children.begin(), thisView.children.end());
        children.erase(std::remove(children.begin(), children.end(), viewId));
    }
}
*/

void ViewHandler::showFullscreen(bool fullscreen)
{
    qCDebug(vhLog, "showFullscreen(fullscreen=%i)", fullscreen);

    if (fullscreen)
        qView->showFullScreen();
    else
        qView->showNormal();
}

/*
void ViewHandler::fixIndentation(int viewId)
{
    std::deque<int> toReindent;
    QVariant novalue;

    auto& thisView = views.at(viewId);

// FIXME: this can be refactored to include only one while loop

    for (int childId : thisView.children)
    {
        views.at(childId).parent = thisView.parent;
        toReindent.insert(toReindent.end(), views.at(childId).children.begin(),
                views.at(childId).children.end());
//        std::cout << "Children of " << childId << " added for reindentation:\n";
//        std::cout << "Vector: ";
        misc::DebugHelpers::printIntVector(views.at(childId).children);
        int indent = countAncestors(views.at(childId).parent);
        QMetaObject::invokeMethod(tabSelector, "fixIndentation",
                Q_RETURN_ARG(QVariant, novalue),
                Q_ARG(QVariant, childId),
                Q_ARG(QVariant, indent));
    }
    //std::cout << "------\n";
    while (!toReindent.empty())
    {
        int childId = toReindent.front();
        toReindent.pop_front();
        QVariant novalue1;
        toReindent.insert(toReindent.end(), views.at(childId).children.begin(),
                views.at(childId).children.end());
//        std::cout << "Children of " << childId << " added for reindentation:\n";
//        std::cout << "Vector: ";
        misc::DebugHelpers::printIntVector(views.at(childId).children);
        int indent = countAncestors(views.at(childId).parent);
        QMetaObject::invokeMethod(tabSelector, "fixIndentation",
                Q_RETURN_ARG(QVariant, novalue1),
                Q_ARG(QVariant, childId),
                Q_ARG(QVariant, indent));
    }
}
*/

void ViewHandler::closeTab(int viewId)
{
    qCDebug(vhLog, "Closing tab: %i", viewId);
    std::lock_guard<std::recursive_mutex> lock(views2Mutex);

    if (views2.count(viewId))
    {
        qCCritical(vhLog, "Trying to close nonexistent tab: %i", viewId);
        return;
    }

    /// Remove tab entry from database and from tabSelector component
    ///
    tabsDb.closeTab(viewId);

    struct viewData vd = views2.at(viewId);
    QStandardItem* item = vd.tabData->parent();
    item->removeRow(vd.tabData->index().row());

//    item->removeRow(vd.tabData->getRowId());

    QVariant novalue;
//    QMetaObject::invokeMethod(tabSelector, "removeTabEntry",
//            Q_RETURN_ARG(QVariant, novalue),
//            Q_ARG(QVariant, viewId));

// remove from model

    /// Move tab's children to parent, fix parent entries, fix indentation levels
    ///
//    fixHierarchy(viewId);
//    fixIndentation(viewId);

    /// Destroy QML view object and erase entry from tabs / views structure
    ///
    //auto& thisView = views.at(viewId);
    //QObject* qo = qvariant_cast<QObject *>(thisView.view);
    QMetaObject::invokeMethod(webViewContainer, "destroyView",
        Q_RETURN_ARG(QVariant, novalue),
        Q_ARG(QVariant, vd.tabData->getView()));

    views2.erase(viewId);

    /// Optionally create new empty tab if last tab was closed
    ///
    if (views2.empty())
        viewSelected(createTab());
}

void ViewHandler::historyUpdated(int _viewId, QQuickWebEngineHistory* navHistory)
{
//    std::cout << "\n\nhistory: " << _viewId << std::endl;


//    QVariant items = navHistory->property("backItems");
//    if (items.isNull() || !items.isValid())
//        std::cout << "null or invalid\n";
//    QAbstractListModel* p = qvariant_cast<QQuickWebEngineHistoryListModel *>(items);

    /*
    QQuickWebEngineHistoryListModel*  items = navHistory->items();

    std::cout << "~~~~~>>>>\n";
    try
    {
        std::cout << "ITEMS: \n";
        for (int i = 0; i < items->rowCount(); ++i)
        {
            QUrl v = items->index(i).data(QQuickWebEngineHistory::UrlRole).toUrl();
            QString t = items->index(i).data(QQuickWebEngineHistory::TitleRole).toString();

            std::cout << "URL: " << v.toString().toStdString() << std::endl;
            std::cout << "TITLE: " << t.toStdString() << std::endl;
        }

        items = navHistory->backItems();
        std::cout << "PAST: \n";
        for (int i = 0; i < items->rowCount(); ++i)
        {
            QUrl v = items->index(i).data(QQuickWebEngineHistory::UrlRole).toUrl();
            QString t = items->index(i).data(QQuickWebEngineHistory::TitleRole).toString();

            std::cout << "URL: " << v.toString().toStdString() << std::endl;
            std::cout << "TITLE: " << t.toStdString() << std::endl;
        }

        items = navHistory->forwardItems();
        std::cout << "FUTURE: \n";
        for (int i = 0; i < items->rowCount(); ++i)
        {
            QUrl v = items->index(i).data(QQuickWebEngineHistory::UrlRole).toUrl();
            QString t = items->index(i).data(QQuickWebEngineHistory::TitleRole).toString();

            std::cout << "URL: " << v.toString().toStdString() << std::endl;
            std::cout << "TITLE: " << t.toStdString() << std::endl;
        }


    }
    catch (std::exception& e)
    {
        std::cout << e.what() << std::endl;
    }
    std::cout << "~~~~~<<<<\n";
*/
//    std::cout << "history items:\n";
//    inspectMethods(p->metaObject());

    //QObject* qv = qvariant_cast<QObject*>(views.at(_viewId).view);

//    std::cout << "properties: \n";
//    auto meta = navHistory->metaObject();
//    inspectProperties(meta);
//    std::cout << "methods:\n";
//    inspectMethods(meta);

    //std::cout << "navHistory: " << navHistory << std::endl;

//    std::cout << qv << std::endl;
}


void ViewHandler::urlChanged(int viewId, QUrl url)
{
    tabsDb.setUrl(viewId, url.toString().toStdString());
}

void ViewHandler::titleChanged(int viewId, QString title)
{
    tabsDb.setTitle(viewId, title.toStdString());
}

void ViewHandler::iconChanged(int viewId, QUrl icon)
{
    tabsDb.setIcon(viewId, icon.toString().toStdString());
}

void ViewHandler::selectTab(int viewId)
{
    QVariant selected;

    int modelId = flatModel.getModelId(viewId);

    QQuickItem* tabSelector = qobject_cast<QQuickItem*>(
        qView->rootObject()->findChild<QObject*>("tabSelector"));

    QMetaObject::invokeMethod(tabSelector, "selectView",
            Q_ARG(QVariant, modelId));

    if (selected.toInt() < 0)
        return;

    viewSelected(viewId);
}

void ViewHandler::loadTabs()
{
    std::vector<db::Tabs::TabInfo> tabs = tabsDb.getAllTabs();
    auto start = std::chrono::system_clock::now();
    std::map<int, db::Tabs::TabInfo> tabsMap = tabsDb.getAllTabsMap();
    /// Open new empty tab if no tabs were retrieved from database
    ///
    if (tabs.empty() || tabsMap.empty())
    {
        viewSelected(createTab());
        return;
    }

    tabsModel.setItemRoleNames(Tab::roles);
    flatModel.setRoleNames(Tab::roles);

    // Fill model and assign to tab container
    QStandardItem* parent = tabsModel.invisibleRootItem();

    std::deque<std::pair<int, QStandardItem*>> toAdd;

    for (auto child: tabsMap[0].children)
    {
        toAdd.push_back(std::pair<int, QStandardItem*>(child, parent));
    }

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

    flatModel.setSourceModel(&tabsModel);

    QQuickItem* visualModel = qobject_cast<QQuickItem*>(
        qView->rootObject()->findChild<QObject*>("tabSelectorPanel"));

    if (!visualModel)
        throw std::runtime_error("No visualModel object found");

    QVariant qv = QVariant::fromValue<QObject*>(&flatModel);
    QMetaObject::invokeMethod(visualModel, "setModel",
                              Qt::ConnectionType::QueuedConnection,
                              Q_ARG(QVariant, qv));

    auto end = std::chrono::system_clock::now();

    /// WHAT FOLLOWS WILL BE DEPRECATED WHEN NEW MODEL LOADING IS DONE
/*
    {
        std::lock_guard<std::recursive_mutex> lock(viewsMutex);

        for (const auto& tab: tabs)
        {
            /// Call webViewContainer to create new QML Web View object and
            /// create associated entry in tabSelectorModel
            ///
            int indent = countAncestors(tab.parent);

            QVariant newView;
            QMetaObject::invokeMethod(webViewContainer, "createNewView",
                    Q_RETURN_ARG(QVariant, newView),
                    Q_ARG(QVariant, tab.id),
                    Q_ARG(QVariant, indent),
                    Q_ARG(QVariant, tab.parent));

            QObject* v = qvariant_cast<QObject *>(newView);

            v->setProperty("targetTitle", tab.title.c_str());
            v->setProperty("targetIcon", tab.icon.c_str());
            v->setProperty("targetUrl", tab.url.c_str()); // FIXME: lazy page loading

            /// Put the view in proper container, and if parent was defined
            /// update it's children list
            ///
            views[tab.id] = viewContainer{newView, tab.parent};
        }

        /// We have to iterate again to fill in the children vector because
        /// only now we can be sure, that all parents are there to begin with.
        ///
        for (const auto& view: views)
        { // FIXME: this will not preserve order between restarts
            if (int parent = view.second.parent)
            {
                views.at(parent).children.push_back(view.first);
            }
        }
    }
*/

    std::chrono::duration<double> total = end-start;
    qCDebug(vhLog, "Model load time: %lli ns",
            std::chrono::duration_cast<std::chrono::nanoseconds>(total));


    /// Restore currentTab from previous session
    ///
    std::string currentTab = configDb.getProperty("currentTab");

    if (!currentTab.empty())
    {
        int viewId = std::stoi(currentTab);

        selectTab(viewId);
    }
}

void ViewHandler::nextTab()
{
    std::string currentTab = configDb.getProperty("currentTab");

    if (!currentTab.empty())
    {
        int viewId = std::stoi(currentTab);

        QVariant nTab;
        QMetaObject::invokeMethod(tabSelector, "getNextTab",
                Q_RETURN_ARG(QVariant, nTab),
                Q_ARG(QVariant, viewId));

        if (!nTab.isValid())
            return;

        int nextId = nTab.toInt();

        selectTab(nextId);
    }
}

void ViewHandler::prevTab()
{
    std::string currentTab = configDb.getProperty("currentTab");

    if (!currentTab.empty())
    {
        int viewId = std::stoi(currentTab);

        QVariant pTab;
        QMetaObject::invokeMethod(tabSelector, "getPrevTab",
                Q_RETURN_ARG(QVariant, pTab),
                Q_ARG(QVariant, viewId));

        if (!pTab.isValid())
            return;

        int prevId = pTab.toInt();

        selectTab(prevId);
    }
}

void ViewHandler::openScriptBlockingView(int viewId)
{
    /// Find current url for viewId and fetch list of script source urls
    /// that were accessed while loading the view
    ///

    QObject* view = qvariant_cast<QObject *>(views.at(viewId).view);
    if (!view)
        throw std::runtime_error("ViewHandler::openScriptBlockingView(): there is no view "
                "associated with this viewId: " + std::to_string(viewId));

    QString url = view->property("url").toUrl().host();
    std::set<std::string> urls = cf.getUrlsFor(url.toStdString());

    QJSEngine engine;

    /// Clear old entries and add fetched urls to view's model
    ///
    QVariant noValue;
    QMetaObject::invokeMethod(scriptBlockingView, "clearEntries",
            Q_RETURN_ARG(QVariant, noValue));

    using bst = db::ScriptBlock::State;
    for (const std::string& u: urls)
    {
        QJSValue val = engine.newObject();
        bst blockState = sBlockDb.isAllowed(url.toStdString(), u, false);

        bool allowed = blockState == bst::AllowedBoth || blockState == bst::Allowed;
        bool gallowed = blockState == bst::AllowedBoth || blockState == bst::AllowedGlobally;

        val.setProperty("url", u.c_str());
        val.setProperty("allowed", allowed);
        val.setProperty("gallowed", gallowed);

        QMetaObject::invokeMethod(scriptBlockingView, "addEntry",
                Q_RETURN_ARG(QVariant, noValue),
                Q_ARG(QVariant, val.toVariant()));
    }

    /// Show scriptBlockingView and hide webViewContainer and
    /// also TabSelector (which is hidden by webViewContainer
    ///
    scriptBlockingView->setProperty("targetUrl", url);
    scriptBlockingView->setProperty("visible", true);
    webViewContainer->setProperty("visible", false);

}
