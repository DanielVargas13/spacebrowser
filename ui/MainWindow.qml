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

    property var downloadDialog: Dialog {
        id: dialog
        title: "Download file"
        
        standardButtons: Dialog.Save | Dialog.Cancel
        modal:true
        
        x: parent.width / 2 - width / 2
        y: parent.height / 2 - height / 2
        
        implicitWidth: 320
        implicitHeight: 240

        header: Label
        {
            text: dialog.title
            
//            background: Rectangle {
//                color: "green"//Style.lightBackground
//                border.width: 1
//                border.color: Style.border
//            }
        }

        background: Rectangle {
            color: Style.lightBackground
            border.width: 1
        }

        onAccepted: console.log("Ok clicked")
        onRejected: console.log("Cancel clicked")
    }
    
    Component.onCompleted: {
        WebEngine.defaultProfile.downloadRequested.connect(downloadHandler);
    }
    
    function downloadHandler(dItem)
    {
        downloadDialog.open()
    }

    TextField
    {
        id: addressBar
        
        height: Style.addressBar.height
        background: Style.addressBar.style

        anchors.left: parent.left
        anchors.right: parent.right

        placeholderText: "https://"
        inputMethodHints: Qt.ImhUrlCharactersOnly
        
        onAccepted: {
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
        anchors.topMargin: Style.margin
        anchors.right: parent.right
        anchors.bottom: mainWindow.bottom
        width: Style.tabSelector.width
        
        onNewTabCreated: addressBar.focus = true
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

        onVisibleChanged:
        {
            tabSelectorPanel.visible = visible
        }
        
        function createNewView(newViewId, _indent, insertAfter)
        {
            var obj = {title:"Empty", icon:"", viewId:newViewId, indent:_indent}

            tabSelectorPanel.createNewTab(obj, insertAfter)

            var view = viewComp.createObject(webViewContainer);
            view.zoomFactor = 1.2 // FIXME: set in style or configuration
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
            
            Component.onDestruction: console.log("\n\n\ndestroying view: " + myViewId)
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
    Shortcut {
        sequence: "Ctrl++"
        onActivated: webViewContainer.currentView.zoomFactor += 0.1
    }
    Shortcut {
        sequence: "Ctrl+-"
        onActivated: webViewContainer.currentView.zoomFactor -= 0.1
    }
    Shortcut {
        sequence: "Ctrl+0"
        onActivated: webViewContainer.currentView.zoomFactor = 1.0
    }
    Shortcut {
        sequence: "Ctrl+t"
        onActivated: {
            var id = viewHandler.createTab()
            viewHandler.selectTab(id)
            addressBar.focus = true
        }
    }
    Shortcut {
        sequence: "Ctrl+w"
        onActivated: {
            viewHandler.closeTab(webViewContainer.currentView.myViewId)
        }
    }
    Shortcut { // test shorcut
        sequence: "Ctrl+g"
        onActivated: {
            console.log("abc");
        }
    }

    ScriptBlockingView
    {
        id: scriptBlockingView
        objectName: "scriptBlockingView"
        visible: false
        
        anchors.top: addressBar.bottom
        anchors.topMargin: Style.margin
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        
        onVisibleChanged: {
            webViewContainer.visible = !visible
        }
    }
    
}
