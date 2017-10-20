#include <ViewHandler.h>

#include <QApplication>
#include <QObject>
#include <QQuickItem>
#include <QQuickView>
//#include <QWebEngineView>
#include <QtWebEngine>
#include <QShortcut>
#include <QTextEdit>

#include <memory>


int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    app.setApplicationName("Space Browser v1");

    QtWebEngine::initialize();

    std::shared_ptr<QQuickView> view(new QQuickView);

//    qmlRegisterType<QTextEdit>("org.qt.qtextedit", 1, 0, "QTextEdit");
//    qmlRegisterType<QWebEngineView>("org.qt.qwebengineview", 1, 0, "QWebEngineView");



    view->setSource(QUrl::fromLocalFile("/home/krbo/src/browser/ui/MainWindow.qml"));
    view->setResizeMode(QQuickView::SizeRootObjectToView);

    view->show();

    QQuickItem* webViewContainer = qobject_cast<QQuickItem*>(
            view->rootObject()->findChild<QObject*>("webViewContainer"));
    if (!webViewContainer)
        throw std::runtime_error("No webViewContainer object found");

    QQuickItem* tabSelector = qobject_cast<QQuickItem*>(
            view->rootObject()->findChild<QObject*>("tabSelector"));

    ViewHandler vh(webViewContainer, tabSelector);
    view->engine()->rootContext()->setContextProperty("viewHandler", &vh);
    vh.loadTabs();
//    MessageBoard msgBoard;
//    QQuickView view;


    if (tabSelector)
    {
        QObject::connect(tabSelector, SIGNAL(viewSelected(int)), &vh, SLOT(viewSelected(int)));
        QObject::connect(tabSelector, SIGNAL(closeTab(int)), &vh, SLOT(closeTab(int)));
    }

    return app.exec();
}
