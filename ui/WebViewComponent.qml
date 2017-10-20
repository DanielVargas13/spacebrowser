import QtWebEngine 1.5

WebEngineView
{
    id: root
    
    property int myViewId
    property var viewContainer
    
    anchors.fill: parent

    onTitleChanged: {
        viewContainer.updateTitle(myViewId, title)
    }

    onIconChanged: {
        viewContainer.updateIcon(myViewId, icon.toString())
    }

    onNewViewRequested: function(request) {
        var viewId = viewHandler.createTab(myViewId)
        request.openIn(viewHandler.getView(viewId))
    }

    onUrlChanged: {
        if (viewContainer.currentView.myViewId == myViewId)
            viewContainer.updateAddressBar(url)

        viewHandler.urlChanged(myViewId, url)
    }
}