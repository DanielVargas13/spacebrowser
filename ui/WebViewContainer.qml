import QtQuick 2.7
import QtWebEngine 1.5


Item
{
    id: root

    property WebEngineView currentView

    onCurrentViewChanged: {
        children = currentView
        updateAddressBar(currentView.url)
    }
    
    function setUrl(url)
    {
        currentView.url = url
    }
    
    function destroyView(view)
    {
        view.destroy()
    }
}