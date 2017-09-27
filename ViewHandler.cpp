#include <ViewHandler.h>

#include <iostream>

ViewHandler::ViewHandler(QQuickItem* _webViewContainer) : webViewContainer(_webViewContainer)
{

}

ViewHandler::~ViewHandler()
{

}

void ViewHandler::viewSelected(int viewId)
{
    std::cout << "VIEW: " << viewId << std::endl;
    webViewContainer->setProperty("currentView2", views.at(viewId));
}

int ViewHandler::createNewView()
{
    QVariant newView;
    int id = viewId;
    ++viewId;
    QMetaObject::invokeMethod(webViewContainer, "createNewView",
            Q_RETURN_ARG(QVariant, newView),
            Q_ARG(QVariant, id));

    views[id] = newView;

    return id;
}

QVariant ViewHandler::getView(int viewId)
{
    return views.at(viewId);
}
