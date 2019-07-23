#include <ViewHandler.h>

#ifndef TEST_BUILD
#include <Tab.h>
#include <misc/DebugHelpers.h>
#else
#include <test/ViewHandler_test.h>
#endif

#include <QAbstractListModel>
#include <QJSEngine>
#include <QJSValue>
#include <QModelIndex>
//#include <QtWebEngine/5.9.1/QtWebEngine/private/qquickwebenginehistory_p.h>

#include <chrono>
#include <deque>

Q_LOGGING_CATEGORY(vhLog, "viewHandler")

ViewHandler::ViewHandler(ContentFilter* _cf, std::shared_ptr<QQuickView> _qView) :
    cf(_cf), qView(_qView), tabsModel(qView)
{
#ifndef TEST_BUILD
    webViewContainer = qobject_cast<QQuickItem*>(
        qView->rootObject()->findChild<QObject*>("webViewContainer"));
    if (!webViewContainer)
        throw std::runtime_error("No webViewContainer object found");

    scriptBlockingView = qobject_cast<QQuickItem*>(
        qView->rootObject()->findChild<QObject*>("scriptBlockingView"));
    if (!scriptBlockingView)
        throw std::runtime_error("No scriptBlockingView object found");
#else
    webViewContainer = new QQuickItem_mock();
    tabSelector = new QQuickItem_mock();
    scriptBlockingView = new QQuickItem_mock();
#endif
}

ViewHandler::~ViewHandler()
{
#ifndef TEST_BUILD
#else
    delete webViewContainer;
    delete tabSelector;
    delete scriptBlockingView;
#endif
}

bool ViewHandler::init(QGuiApplication& app)
{
    // FIXME: here initialize db::Tabs, and other db:: in the future

    tabsModel.loadTabs();
    app.processEvents();
    tabsModel.selectCurrentTab();

    return true;
}

void ViewHandler::showFullscreen(bool fullscreen)
{
    qCDebug(vhLog, "showFullscreen(fullscreen=%i)", fullscreen);

    if (fullscreen)
        qView->showFullScreen();
    else
        qView->showNormal();
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



void ViewHandler::openScriptBlockingView(int viewId)
{
    /// Find current url for viewId and fetch list of script source urls
    /// that were accessed while loading the view
    ///

    QObject* view = qvariant_cast<QObject *>(tabsModel.getView(viewId));

    if (!view)
        throw std::runtime_error("ViewHandler::openScriptBlockingView(): there is no view "
                "associated with this viewId: " + std::to_string(viewId));

    QString url = view->property("url").toUrl().host();
    std::set<std::string> urls = cf->getUrlsFor(url.toStdString());

    QJSEngine engine;

    /// Clear old entries and add fetched urls to view's model
    ///
    QVariant noValue;
    QMetaObject::invokeMethod(scriptBlockingView, "clearEntries",
            Q_RETURN_ARG(QVariant, noValue));

    using bst = db::ScriptBlock2::State;
    for (const std::string& u: urls)
    {
        QJSValue val = engine.newObject();
        bst blockState = dbh->scb.isAllowed(url, u.c_str(), false);

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
