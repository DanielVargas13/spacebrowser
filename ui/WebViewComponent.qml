import QtQuick 2.7
import QtWebEngine 1.5

WebEngineView
{
    id: root
    
    property int myViewId
    property var viewContainer
    
    property var targetUrl // FIXME: these two are needed to handle lazy loading of webpages
    property var targetTitle // it would be cleaner to handle this on qt level
    
    anchors.fill: parent

    onTitleChanged: {
        viewContainer.updateTitle(myViewId, title, true)
    }

    onIconChanged: {
        viewContainer.updateIcon(myViewId, icon.toString())
    }

    onNewViewRequested: function(request) {
        var viewId = viewHandler.createTab(myViewId)
        viewHandler.getView(viewId).visible = false
        request.openIn(viewHandler.getView(viewId))
    }

    onUrlChanged: {
        if (viewContainer.currentView && viewContainer.currentView.myViewId == myViewId)
            viewContainer.updateAddressBar(url)

        viewHandler.urlChanged(myViewId, url)
    }

    onTargetTitleChanged: {
        viewContainer.updateTitle(myViewId, targetTitle, false)
    }

    function setup() {
        if (url == "" && targetUrl && targetUrl != "")
            url = targetUrl
    }
}