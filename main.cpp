#include <BasicDownloader.h>
#include <ContentFilter.h>
#include <ViewHandler.h>
#include <PrintHandler.h>
#include <PasswordManager.h>
#include <db/Backend.h>
#include <conf/conf.h>

#include <db/Tabs2.h>

#include <QApplication>
#include <QJsonArray>
#include <QJsonObject>
#include <QLoggingCategory>
#include <QObject>
#include <QQuickItem>
#include <QQuickView>
#include <QtWebEngine>
#include <QQuickWebEngineProfile>
#include <QShortcut>
#include <QString>
#include <QStringList>
#include <QSqlDatabase>
#include <QTextEdit>

#include <memory>

Q_DECLARE_LOGGING_CATEGORY(mainLogs)
Q_LOGGING_CATEGORY(mainLogs, "main")

void setupProfileDownloadHandler(BasicDownloader& bd, QQuickWebEngineProfile* profile)
{
    QObject::connect(profile,
            SIGNAL(downloadRequested(QQuickWebEngineDownloadItem*)), &bd,
            SLOT(downloadRequested(QQuickWebEngineDownloadItem*)));
    QObject::connect(profile,
            SIGNAL(downloadFinished(QQuickWebEngineDownloadItem*)), &bd,
            SLOT(downloadFinished(QQuickWebEngineDownloadItem*)));
}

void setupDownloaderSignals(BasicDownloader& bd, const std::shared_ptr<QQuickView> view,
        QQuickWebEngineProfile* profile)
{
    /// Set-up downloader module
    ///
    QQuickItem* downloaderProgressBar = qobject_cast<QQuickItem*>(
            view->rootObject()->findChild<QObject*>("downloadProgressBar"));
    if (!downloaderProgressBar)
        throw std::runtime_error("No downloaderProgressBar object found");

    QObject::connect(&bd, SIGNAL(progressUpdated(QVariant)),
            downloaderProgressBar, SLOT(updateProgress(QVariant)));
    QQuickItem* downloadHistoryButton = qobject_cast<QQuickItem*>(
            view->rootObject()->findChild<QObject*>("downloadHistoryButton"));
    if (!downloadHistoryButton)
        throw std::runtime_error("No downloadHistoryButton object found");

    QObject::connect(&bd, &BasicDownloader::historyChanged,
            downloadHistoryButton, &QQuickItem::setVisible);
    QQuickItem* downloadHistoryView = qobject_cast<QQuickItem*>(
            view->rootObject()->findChild<QObject*>("downloadHistoryView"));
    if (!downloadHistoryView)
        throw std::runtime_error("No downloadHistoryView object found");

    QObject::connect(&bd, SIGNAL(downloadFinished(QVariant)),
            downloadHistoryView, SLOT(downloadFinished(QVariant)));
    QObject::connect(&bd, SIGNAL(newHistoryEntry(QVariant)),
            downloadHistoryView, SLOT(addEntry(QVariant)));
    QObject::connect(&bd, SIGNAL(progressUpdated(QVariant, QVariant, QVariant)),
            downloadHistoryView,
            SLOT(updateProgress(QVariant, QVariant, QVariant)));
    QObject::connect(&bd, SIGNAL(downloadPaused(QVariant)), downloadHistoryView,
            SLOT(downloadPaused(QVariant)));
    QObject::connect(&bd, SIGNAL(downloadResumed(QVariant)),
            downloadHistoryView, SLOT(downloadResumed(QVariant)));
    QObject::connect(&bd, SIGNAL(downloadCanceled(QVariant)),
            downloadHistoryView, SLOT(downloadCanceled(QVariant)));
    QObject::connect(downloadHistoryView, SIGNAL(openUrl(QString)), &bd,
            SLOT(openUrl(QString)));
    QObject::connect(downloadHistoryView, SIGNAL(pause(int)), &bd,
            SLOT(pause(int)));
    QObject::connect(downloadHistoryView, SIGNAL(resume(int)), &bd,
            SLOT(resume(int)));
    QObject::connect(downloadHistoryView, SIGNAL(cancel(int)), &bd,
            SLOT(cancel(int)));
}

void writeSettings(std::shared_ptr<QQuickView> view)
{
    QSettings settings;

    settings.setValue(conf::MainWindow::geometry, view->geometry());
}

void readSettings(std::shared_ptr<QQuickView> view)
{
    QSettings settings;

    if (settings.contains(conf::MainWindow::geometry))
        view->setGeometry(settings.value(conf::MainWindow::geometry).toRect());
}

int main(int argc, char *argv[])
{
    /// Initialize logging
    ///
    QString colorsStart = "%{if-debug}\033[34m%{endif}%{if-warning}\033[33m%{endif}"
        "%{if-critical}\033[31m%{endif}";
    qSetMessagePattern(colorsStart +
                       "[%{category}] %{function}[:%{line}]\033[0m: %{message}");

    /// Create and setup QApplication
    ///
    QApplication app(argc, argv);
    app.setOrganizationName("SpaceFoundation");
    app.setApplicationName("SpaceBrowser");
    //FIXME: test this
    //QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QtWebEngine::initialize();

    /// Create and setup content filter
    ///
    ContentFilter cf;
    QQuickWebEngineProfile* profile = QQuickWebEngineProfile::defaultProfile();
    profile->setRequestInterceptor(&cf);

    /// Create and setup QQuickView with MainWindow
    ///
    std::shared_ptr<QQuickView> view(new QQuickView);
    view->setSource(QUrl("qrc:/ui/MainWindow.qml"));
    view->setResizeMode(QQuickView::SizeRootObjectToView);
    view->show();
    readSettings(view);

    /// Setup printing handler
    ///
    PrintHandler ph;
    QObject::connect(view->rootObject(), SIGNAL(printRequest(QVariant)),
            &ph, SLOT(printRequested(QVariant)));

    /// Setup password manager
    ///
    PasswordManager passMan;
    QObject::connect(view->rootObject(), SIGNAL(loadSucceeded(QVariant)),
                     &passMan, SLOT(loadSucceeded(QVariant)));

    QObject* encKeyConfDialog = view->rootObject()->
        findChild<QObject*>("encryptionKeyConfigDialog");
    if (!encKeyConfDialog)
        throw std::runtime_error("No encryptionKeyConfigDialog object found");

    QObject::connect(encKeyConfDialog, SIGNAL(keySelected(QString)),
                     &passMan, SLOT(keySelected(QString)));

    QQuickItem* passwordManagerButton = qobject_cast<QQuickItem*>(
            view->rootObject()->findChild<QObject*>("passwordManagerButton"));
    if (!passwordManagerButton)
        throw std::runtime_error("No passwordManagerButton object found");

    QObject::connect(passwordManagerButton, SIGNAL(passwordFillRequest(QVariant)),
                     &passMan, SLOT(fillPassword(QVariant)));



    if (!passMan.isEncryptionReady())
    {
        QStringList model = passMan.keysList();
        QMetaObject::invokeMethod(view->rootObject(), "configureEncryption",
                                  Qt::ConnectionType::QueuedConnection,
                                  Q_ARG(QVariant, model));
    }

    QObject::connect(&passMan, SIGNAL(shouldBeSaved(QVariant, QVariant)),
            view->rootObject(), SLOT(shouldBeSaved(QVariant, QVariant)));
    QObject::connect(&passMan, SIGNAL(shouldBeUpdated(QVariant, QVariant)),
                view->rootObject(), SLOT(shouldBeUpdated(QVariant, QVariant)));
    QObject::connect(view->rootObject(), SIGNAL(savePasswordAccepted(QString, bool)),
            &passMan, SLOT(saveAccepted(QString, bool)));


    /// Setup View Handler
    ///

    // FIXME: refactor below to use slots/signals instead of calling vh directly
    QQuickItem* scriptBlockingView = qobject_cast<QQuickItem*>(
            view->rootObject()->findChild<QObject*>("scriptBlockingView"));
    if (!scriptBlockingView)
        throw std::runtime_error("No scriptBlockingView object found");

    QQuickItem* tabSelector = qobject_cast<QQuickItem*>(
            view->rootObject()->findChild<QObject*>("tabSelector"));

    ViewHandler vh(&cf, view);

    // FIXME: consider removing this contextProperty and communicate via signals instead
    view->engine()->rootContext()->setContextProperty("viewHandler", &vh);
    if (tabSelector)
    {
        QObject::connect(tabSelector, SIGNAL(viewSelected(int)), &vh, SLOT(viewSelected(int)));
        QObject::connect(tabSelector, SIGNAL(closeTab(int)), &vh, SLOT(closeTab(int)));
        QObject::connect(tabSelector, SIGNAL(openScriptBlockingView(int)), &vh, SLOT(openScriptBlockingView(int)));
    }

    vh.loadTabs();
    app.processEvents();
    vh.selectCurrentTab();

    QObject::connect(scriptBlockingView, SIGNAL(whitelistLocal(QString, QString)),
            &cf, SLOT(whitelistLocal(QString, QString)));
    QObject::connect(scriptBlockingView, SIGNAL(whitelistGlobal(QString)),
            &cf, SLOT(whitelistGlobal(QString)));
    QObject::connect(scriptBlockingView, SIGNAL(removeLocal(QString, QString)),
            &cf, SLOT(removeLocal(QString, QString)));
    QObject::connect(scriptBlockingView, SIGNAL(removeGlobal(QString)),
            &cf, SLOT(removeGlobal(QString)));


    /// Setup Downloader
    ///
    BasicDownloader bd;
    setupProfileDownloadHandler(bd, profile);
    setupDownloaderSignals(bd, view, profile);


    /// Initialize databases
    ///
    QSettings settings;
    unsigned int dbCount = settings.beginReadArray(conf::Databases::dbArray);
    settings.endArray();

    QObject* confDbConnDialog = view->rootObject()->
        findChild<QObject*>("configureDbConnectionDialog");
    if (!confDbConnDialog)
        throw std::runtime_error("No configureDbConnectionDialog object found");

    db::Backend dbBackend;
    QObject::connect(confDbConnDialog, SIGNAL(dbConfigured(QVariant)),
                     &dbBackend, SLOT(dbConfigured(QVariant)));

    // FIXME: call connectDatabases async and configure if connect failed
    if (!dbCount || !dbBackend.connectDatabases())
    {
        qCCritical(mainLogs) <<"no db configured or connectDatabases() failed";

        dbBackend.configureDbConnection(confDbConnDialog, passMan.isEncryptionReady());
    }

    db::Tabs2 t2;
    t2.initDatabase("localPS");

    qCDebug(mainLogs) << "Finished init, executing app";
    int status = app.exec();

    writeSettings(view);

    return status;
}
