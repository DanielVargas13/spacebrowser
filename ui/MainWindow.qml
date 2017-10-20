import QtQuick 2.7
import QtQuick.Controls 2.2
import QtWebEngine 1.5

//import org.qt.qwebengineview 1.0
//import QtQuick.Controls 1.4 as OldControls
//import QtQuick.Dialogs 1.2 // file dialog

import "."

//import org.qt.qtextedit 1.0

Rectangle
{
    id: mainWindow
    visible: true

    color: Style.background

    width: Style.mainWindow.width
    height: Style.mainWindow.height

    TextField
    {
        id: addressBar
        
        height: Style.addressBar.height
        background: Style.addressBar.style

        anchors.left: parent.left
        anchors.right: parent.right

        placeholderText: "https://"
        inputMethodHints: Qt.ImhUrlCharactersOnly
        
        onAccepted: { // FIXME: move logic to c++
            if (!text.toLowerCase().startsWith("https://") &&
                    !text.toLowerCase().startsWith("http://"))
            {
                text = "https://" + text
            }

            webViewContainer.setUrl(text)
        }
    }

    TabSelectorPanel
    {
        id: tabSelectorPanel
        
        anchors.top: addressBar.bottom
        anchors.right: parent.right
        anchors.bottom: mainWindow.bottom
        width: Style.tabSelector.width

    }
    
    WebViewContainer
    {
        id: webViewContainer
        objectName: "webViewContainer"

        anchors.top: addressBar.bottom
        anchors.topMargin: Style.margin
        anchors.left: parent.left
        anchors.right: tabSelectorPanel.left
        anchors.bottom: parent.bottom

        function createNewView(newViewId, _indent, insertAfter)
        {
            var obj = {title:"Empty", icon:"", viewId:newViewId, indent:_indent}

            tabSelectorPanel.createNewTab(obj, insertAfter)

            var view = viewComp.createObject(null);
            view.myViewId = newViewId

            return view
        }

        function updateTitle(viewId, title)
        {
            tabSelectorPanel.updateTitle(viewId, title)
            viewHandler.titleChanged(viewId, title)
        }
        
        function updateIcon(viewId, icon)
        {
            tabSelectorPanel.updateIcon(viewId, icon)
            viewHandler.iconChanged(viewId, icon.toString())
        }
        
        function updateAddressBar(url)
        {
            addressBar.text = url
        }

        property Component viewComp: WebViewComponent
        {
            id: webView
            viewContainer: webViewContainer
        }

    }

    Shortcut {
        sequence: "Ctrl+Tab"
        onActivated: viewHandler.nextTab();
    }
    Shortcut {
        sequence: "Ctrl+Shift+Tab"
        onActivated: viewHandler.prevTab();
    }


}