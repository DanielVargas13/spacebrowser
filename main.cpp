#include <ViewHandler.h>
#include <ContentFilter.h>

#include <QApplication>
#include <QObject>
#include <QQuickItem>
#include <QQuickView>
//#include <QWebEngineView>
#include <QtWebEngine>
#include <QQuickWebEngineProfile>
#include <QShortcut>
#include <QTextEdit>

#include <memory>


int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    app.setApplicationName("Space Browser v1");

    QtWebEngine::initialize();

    ContentFilter cf;
    QQuickWebEngineProfile::defaultProfile()->setRequestInterceptor(&cf);

    std::shared_ptr<QQuickView> view(new QQuickView);

//    qmlRegisterType<QTextEdit>("org.qt.qtextedit", 1, 0, "QTextEdit");
//    qmlRegisterType<QWebEngineView>("org.qt.qwebengineview", 1, 0, "QWebEngineView");



    view->setSource(QUrl("qrc:/ui/MainWindow.qml"));
    view->setResizeMode(QQuickView::SizeRootObjectToView);

    view->show();

    QQuickItem* webViewContainer = qobject_cast<QQuickItem*>(
            view->rootObject()->findChild<QObject*>("webViewContainer"));
    if (!webViewContainer)
        throw std::runtime_error("No webViewContainer object found");

    QQuickItem* scriptBlockingView = qobject_cast<QQuickItem*>(
            view->rootObject()->findChild<QObject*>("scriptBlockingView"));
    if (!scriptBlockingView)
        throw std::runtime_error("No scriptBlockingView object found");

    QQuickItem* tabSelector = qobject_cast<QQuickItem*>(
            view->rootObject()->findChild<QObject*>("tabSelector"));

    ViewHandler vh(webViewContainer, tabSelector, scriptBlockingView, cf);
    view->engine()->rootContext()->setContextProperty("viewHandler", &vh);
    vh.loadTabs();
//    MessageBoard msgBoard;
//    QQuickView view;


    if (tabSelector)
    {
        QObject::connect(tabSelector, SIGNAL(viewSelected(int)), &vh, SLOT(viewSelected(int)));
        QObject::connect(tabSelector, SIGNAL(closeTab(int)), &vh, SLOT(closeTab(int)));
        QObject::connect(tabSelector, SIGNAL(openScriptBlockingView(int)), &vh, SLOT(openScriptBlockingView(int)));
    }

    QObject::connect(scriptBlockingView, SIGNAL(whitelistLocal(QString, QString)),
            &cf, SLOT(whitelistLocal(QString, QString)));
    QObject::connect(scriptBlockingView, SIGNAL(whitelistGlobal(QString)),
            &cf, SLOT(whitelistGlobal(QString)));
    QObject::connect(scriptBlockingView, SIGNAL(removeLocal(QString, QString)),
            &cf, SLOT(removeLocal(QString, QString)));
    QObject::connect(scriptBlockingView, SIGNAL(removeGlobal(QString)),
            &cf, SLOT(removeGlobal(QString)));

    return app.exec();
}
