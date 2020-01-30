#include <ViewHandler.h>

#include <conf/conf.h>
#include <db/Backend.h>
#include <db/DbGroup.h>

#ifndef TEST_BUILD
#include <Tab.h>
#include <misc/DebugHelpers.h>

#include <QApplication>
#include <QFileDialog>
#include <QQuickWebEngineProfile>
#include <QSqlDatabase>
#else
#include <test/TabModel_test.h>
#endif

#include <QAbstractListModel>
#include <QIcon>
#include <QJSEngine>
#include <QJSValue>
#include <QModelIndex>
#include <QSettings>
#include <QStandardItem>
#include <QStandardPaths>

//#include <QtWebEngine/5.9.1/QtWebEngine/private/qquickwebenginehistory_p.h>

#include <chrono>
#include <deque>

Q_LOGGING_CATEGORY(vhLog, "viewHandler")

ViewHandler::ViewHandler(std::shared_ptr<QQuickView> _qView, db::Backend& _dbBack) :
    qView(_qView), dbBack(_dbBack), bd(_qView)
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

    QObject* confDbConnDialog = qView->rootObject()->
        findChild<QObject*>("configureDbConnectionDialog");
    if (!confDbConnDialog)
        throw std::runtime_error("No configureDbConnectionDialog object found");

    QObject::connect(confDbConnDialog, SIGNAL(selectIcon()),
                     this, SLOT(iconRequestedDialog()));
    QObject::connect(this, SIGNAL(iconSelected(QVariant)),
                     confDbConnDialog, SLOT(iconSelected(QVariant)));

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

bool ViewHandler::init()
{
    /// Configure model for Panel Selector
    QQuickItem* panelSelector = qobject_cast<QQuickItem*>(
        qView->rootObject()->findChild<QObject*>("panelSelector"));
    QVariant pMod = QVariant::fromValue<QObject*>(&panelModel);
    QMetaObject::invokeMethod(panelSelector, "setModel",
                              Qt::ConnectionType::QueuedConnection,
                              Q_ARG(QVariant, pMod));

    QObject::connect(panelSelector, SIGNAL(panelSelected(QString)),
                     this, SLOT(selectPanel(QString)));
    QObject::connect(&dbBack, SIGNAL(iconUpdated(QString, QString)),
                     this, SLOT(updatePanelIcon(QString, QString)));

    QStandardItem* item = new QStandardItem(addDbText);
    item->setToolTip("qrc:/ui/icons/plus_line.svg"); /// FIXME: this should be proper role in the model
    panelModel.appendRow(item);

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



void ViewHandler::openScriptBlockingView(QString dbName, int viewId)
{
    /// Find current url for viewId and fetch list of script source urls
    /// that were accessed while loading the view
    ///
    QObject* view = qvariant_cast<QObject *>(tabsModels.at(dbName)->getView(viewId));

    if (!view)
        throw std::runtime_error("ViewHandler::openScriptBlockingView(): there is no view "
                "associated with this viewId: " + std::to_string(viewId));

    QString url = view->property("url").toUrl().host();
    std::set<std::string> urls = contentFilters.at(dbName)->getUrlsFor(url.toStdString());

    QJSEngine engine;

    /// Clear old entries and add fetched urls to view's model
    ///
    QVariant noValue;
    QMetaObject::invokeMethod(scriptBlockingView, "clearEntries",
            Q_RETURN_ARG(QVariant, noValue));


    std::shared_ptr<db::DbGroup> dbg;
    dbg = db::DbGroup::getGroup(dbName);

    using bst = db::ScriptBlock2::State;
    for (const std::string& u: urls)
    {
        QJSValue val = engine.newObject();
        bst blockState = dbg->scb.isAllowed(url, u.c_str(), false);

        bool allowed = blockState == bst::AllowedBoth || blockState == bst::Allowed;
        bool gallowed = blockState == bst::AllowedBoth || blockState == bst::AllowedGlobally;

        val.setProperty("url", u.c_str());
        val.setProperty("allowed", allowed);
        val.setProperty("gallowed", gallowed);

        QMetaObject::invokeMethod(scriptBlockingView, "addEntry",
                Q_RETURN_ARG(QVariant, noValue),
                Q_ARG(QVariant, val.toVariant()));
    }

    /// scriptBlockingView needs to propagate dbName with it's signals
    /// so that content filters will be able to filter out which signals
    /// are directed at them
    ///
    QMetaObject::invokeMethod(scriptBlockingView, "setDbName",
            Q_RETURN_ARG(QVariant, noValue),
            Q_ARG(QVariant, dbName));

    /// Show scriptBlockingView and hide webViewContainer and
    /// also TabSelector (which is hidden by webViewContainer
    ///
    scriptBlockingView->setProperty("targetUrl", url);
    scriptBlockingView->setProperty("visible", true);
    webViewContainer->setProperty("visible", false);

}

void ViewHandler::createWebProfile(QString dbName)
{
#ifndef TEST_BUILD
    contentFilters[dbName] = std::shared_ptr<ContentFilter>(new ContentFilter(dbName));
    ContentFilter& cf = *contentFilters[dbName];

    webProfiles[dbName] = std::shared_ptr<QQuickWebEngineProfile>(new QQuickWebEngineProfile());
    webProfiles[dbName]->setStorageName(dbName);
    webProfiles[dbName]->setRequestInterceptor(&cf);

    webProfiles[dbName]->setOffTheRecord(false);

    QObject::connect(scriptBlockingView, SIGNAL(whitelistLocal(QString, QString, QString)),
                     &cf, SLOT(whitelistLocal(QString, QString, QString)));
    QObject::connect(scriptBlockingView, SIGNAL(whitelistGlobal(QString, QString)),
                     &cf, SLOT(whitelistGlobal(QString, QString)));
    QObject::connect(scriptBlockingView, SIGNAL(removeLocal(QString, QString, QString)),
                     &cf, SLOT(removeLocal(QString, QString, QString)));
    QObject::connect(scriptBlockingView, SIGNAL(removeGlobal(QString, QString)),
                     &cf, SLOT(removeGlobal(QString, QString)));

    bd.setupProfile(webProfiles[dbName]);
#endif
}

std::shared_ptr<TabModel> ViewHandler::createTabModel(QString dbName)
{
    QQuickItem* tabSelector = qobject_cast<QQuickItem*>(
        qView->rootObject()->findChild<QObject*>("tabSelector"));

    QQuickItem* tabSelectorPanel = qobject_cast<QQuickItem*>(
        qView->rootObject()->findChild<QObject*>("tabSelectorPanel"));

    auto model = std::make_shared<TabModel>(dbName, tabSelector, tabSelectorPanel, webViewContainer,
                                            webProfiles[dbName], db::DbGroup::getGroup(dbName));

    QObject::connect(qView->rootObject(), SIGNAL(createTab(QString, int, bool, bool)),
                     model.get(), SLOT(createTab(QString, int, bool, bool)));
    QObject::connect(qView->rootObject(), SIGNAL(closeTab(QString, int)),
                     model.get(), SLOT(closeTab(QString, int)));
    QObject::connect(qView->rootObject(), SIGNAL(nextTab(QString)),
                     model.get(), SLOT(nextTab(QString)));
    QObject::connect(qView->rootObject(), SIGNAL(prevTab(QString)),
                     model.get(), SLOT(prevTab(QString)));


    QObject::connect(tabSelector, SIGNAL(viewSelected(QString, int)),
                     model.get(), SLOT(viewSelected(QString, int)));
    QObject::connect(tabSelector, SIGNAL(closeTab(QString, int)),
                     model.get(), SLOT(closeTab(QString, int)));

    return model;

}

void ViewHandler::dbReady(QString dbName, QString schemaName)
{
    qCDebug(dbLogs, "Database %s connected, setting up", dbName.toStdString().c_str());

    /// Create and fill model
    ///
    QSettings settings;
    auto dbConnData = db::Backend::readAllConnectionEntries(settings);

    createWebProfile(dbName);

    std::shared_ptr<TabModel> tabsModel = createTabModel(dbName);

    tabsModels[dbName] = tabsModel;
    tabsModel->loadTabs();

    /// Find icon for connected database
    ///
    auto result = std::find_if(dbConnData.begin(), dbConnData.end(),
                               [&dbName](const struct db::Backend::connData_t& cd)->bool
                               {
                                   if (dbName == cd.connName)
                                       return true;

                                   return false;
                               });
    QString icon;
    if (result != dbConnData.end())
        icon = result->connIcon;

    /// Add row to panel model and select panel if it was the previously selected one
    ///
    QStandardItem* item = new QStandardItem(dbName);
    item->setToolTip(icon); /// FIXME: this should be proper role in the model

    int position = panelModel.rowCount() - 1;
    if (position < 0)
        position = 0;

    panelModel.insertRow(position, item);

    if (settings.contains(conf::Databases::currentPanel) &&
        settings.value(conf::Databases::currentPanel).toString() == dbName)
        selectPanel(dbName);
}

void ViewHandler::selectPanel(QString dbName)
{
    if (dbName == addDbText) // FIXME: check and perhaps do this in a cleaner way
    {
        openDbConfig();
        return;
    }

    // FIXME: consider creating separate TabSelector for each panel
    //        to optimize panel switching time (avoid calls to setModel())
    QQuickItem* tabSelectorPanel = qobject_cast<QQuickItem*>(
        qView->rootObject()->findChild<QObject*>("tabSelectorPanel"));

    auto& tabsModel = tabsModels.at(dbName);

    QVariant qv = QVariant::fromValue<QObject*>(tabsModel->getFlatModel());
    QMetaObject::invokeMethod(tabSelectorPanel, "setModel",
                              Qt::ConnectionType::QueuedConnection,
                              Q_ARG(QVariant, qv));

    QQuickItem* panelSelector = qobject_cast<QQuickItem*>(
        qView->rootObject()->findChild<QObject*>("panelSelector"));

    QMetaObject::invokeMethod(panelSelector, "setCurrentPanel",
                              Qt::ConnectionType::QueuedConnection,
                              Q_ARG(QVariant, dbName));
#ifndef TEST_BUILD
    QApplication::processEvents();
#endif
    tabsModel->selectCurrentTab();

    QSettings settings;
    settings.setValue(conf::Databases::currentPanel, dbName);

    emit panelSelected(dbName);
}

void ViewHandler::openDbConfig()
{
    QObject* confDbConnDialog = qView->rootObject()->
        findChild<QObject*>("configureDbConnectionDialog");
    if (!confDbConnDialog)
        throw std::runtime_error("No configureDbConnectionDialog object found");

#ifndef TEST_BUILD
    dbBack.configureDbConnection(confDbConnDialog, /*passMan.isEncryptionReady()*/ false);
#endif
}

void ViewHandler::iconRequestedDialog()
{
#ifndef TEST_BUILD
    /// Set-up and show QFileDialog
    ///
    QFileDialog qfd;
    qfd.setFileMode(QFileDialog::ExistingFile);
    qfd.setAcceptMode(QFileDialog::AcceptOpen);
    qfd.setNameFilter("Image files (*.png *.xpm *.jpg *.svg *.ico)");

//    qfd.selectFile(QStandardPaths::writableLocation(QStandardPaths::HomeLocation));

    bool accepted = qfd.exec() == QDialog::Accepted;

    /// Set target path and filename for the accepted download
    ///
    if (accepted)
    {
        emit iconSelected("file://" + qfd.selectedFiles().first());
    }
#endif
}

void ViewHandler::updatePanelIcon(QString dbName, QString iconPath)
{
    for (int i = 0; i < panelModel.rowCount(); ++i)
    {
        QModelIndex index = panelModel.index(i, 0);
        if (index.data(Qt::DisplayRole).toString() != dbName)
            continue;

        panelModel.setData(index, iconPath, Qt::ToolTipRole);
    }
}
