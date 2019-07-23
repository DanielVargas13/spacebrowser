import QtQuick 2.7
import QtWebEngine 1.7

Item
{
    id: root

    property WebEngineView currentView

    signal titleChanged(int viewId, string title)
    signal iconChanged(int viewId, string icon)
    signal urlChanged(int viewId, string url)

    onCurrentViewChanged: {
        children = currentView
        currentView.visible = true
        currentView.setup()
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

    function setFocus()
    {
        currentView.focus = true
    }

    function updateTitle(viewId, title)
    {
        root.titleChanged(viewId, title)
    }

    function updateIcon(viewId, icon)
    {
        root.iconChanged(viewId, icon.toString())
    }

}
