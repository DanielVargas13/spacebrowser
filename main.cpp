#include <ViewHandler.h>

#include <QApplication>
#include <QObject>
#include <QQuickItem>
#include <QQuickView>
#include <QtWebEngine>
#include <QTextEdit>

#include <memory>


int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    app.setApplicationName("Space Browser v1");

    QtWebEngine::initialize();

    std::shared_ptr<QQuickView> view(new QQuickView);

//    qmlRegisterType<QTextEdit>("org.qt.qtextedit", 1, 0, "QTextEdit");




    view->setSource(QUrl::fromLocalFile("/home/krbo/src/browser/ui/MainWindow.qml"));
    view->setResizeMode(QQuickView::SizeRootObjectToView);

    view->show();

    QQuickItem* webViewContainer = qobject_cast<QQuickItem*>(
            view->rootObject()->findChild<QObject*>("webViewContainer"));
    if (!webViewContainer)
        throw std::runtime_error("No webViewContainer object found");

    ViewHandler vh(webViewContainer);

    {
        int newView = vh.createNewView();
        vh.viewSelected(newView);
    }

//    MessageBoard msgBoard;
//    QQuickView view;
    view->engine()->rootContext()->setContextProperty("viewHandler", &vh);


    QQuickItem* tabSelector = qobject_cast<QQuickItem*>(
            view->rootObject()->findChild<QObject*>("tabSelector"));
    if (tabSelector)
    {
        QObject::connect(tabSelector, SIGNAL(viewSelected(int)), &vh, SLOT(viewSelected(int)));
    }


//    QObject* webVC = view->rootObject()->findChild<QObject*>("webViewContainer");
//    if (webVC)
//    {
//        webVC->
//        QObject::connect(webVC, SIGNAL(accept(QUrl, QObject*)), &imp, SLOT(importTex(QUrl, QObject*)));
//    }
//
//
//    QObject* tip = view->rootObject()->findChild<QObject*>("texImportPreview");
//    if (tip)
//    {
//        QObject::connect(tip, SIGNAL(accept(int, QString)), &imp, SLOT(importAccepted(int, QString)));
//        QObject::connect(tip, SIGNAL(decline(int)), &imp, SLOT(importDeclined(int)));
//        QObject::connect(tip, SIGNAL(provideCollections(int, QString)), &imp, SLOT(provideCollections(int, QString)));
//    }



    return app.exec();
}
