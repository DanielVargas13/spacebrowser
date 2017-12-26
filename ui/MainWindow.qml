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

//    property var downloadDialog: Dialog {
//        id: dialog
//        title: "Download file"
//        property var item
//        standardButtons: Dialog.Save | Dialog.Cancel
//        modal:true
//
//        x: parent.width / 2 - width / 2
//        y: parent.height / 2 - height / 2
//
//        implicitWidth: 320
//        implicitHeight: 240
//        
//        onAccepted: {
//            console.log("accepted")
//            console.log(item)
//        }
//    }
    
    property bool isFullscreen: false

    Component.onCompleted: {
        WebEngine.defaultProfile.downloadRequested.connect(downloadHandler);
    }

    function downloadHandler(dItem)
    {
        var accepted = viewHandler.downloadRequested(dItem)
        if (accepted)
            dItem.accept()
        else
            dItem.cancel()
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
        selectByMouse: true
        
        onAccepted: {
            if (!text.toLowerCase().startsWith("https://") &&
                    !text.toLowerCase().startsWith("http://"))
            {
                text = "https://" + text
            }

            webViewContainer.setUrl(text)
            webViewContainer.setFocus()
        }

        ProgressComponent
        {
            id: pageLoadProgressBar
            anchors.right: addressBar.right
            anchors.rightMargin: Style.margin
            anchors.verticalCenter: addressBar.verticalCenter
            progress: webViewContainer.currentView ? webViewContainer.currentView.loadProgress / 100.0 : 0

            stateVisible: webViewContainer.currentView ? webViewContainer.currentView.loading : false
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

        function updateTitle(viewId, title, updateModel)
        {
            tabSelectorPanel.updateTitle(viewId, title)
            if (updateModel)
                viewHandler.titleChanged(viewId, title)
        }
        
        function updateIcon(viewId, icon, updateModel)
        {
            tabSelectorPanel.updateIcon(viewId, icon)
            if (updateModel)
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
            
            //Component.onDestruction: console.log("\n\n\ndestroying view: " + myViewId)
            
            onFullScreenRequested: function(request) {
                mainWindow.isFullscreen = request.toggleOn
                viewHandler.showFullscreen(request.toggleOn)
                request.accept()
                webViewContainer.currentView.parent = request.toggleOn ? mainWindow : webViewContainer
            }
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
        sequence: StandardKey.ZoomIn//"Ctrl++"
        onActivated: webViewContainer.currentView.zoomFactor += 0.1
    }
    Shortcut {
        sequence: StandardKey.ZoomOut//"Ctrl+-"
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
    
    Shortcut {
        sequence: "Escape"
        onActivated: {
            if (mainWindow.isFullscreen)
                webViewContainer.currentView.triggerWebAction(WebEngineView.ExitFullScreen);
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

//    MouseArea  // FIXME: this works, but unfortunately prevents WebView from changing cursor shape
//    {
//        anchors.fill: webViewContainer // FIXME: it would be great to have this MouseArea in webViewContainer, but it won't get events there
//        acceptedButtons: Qt.NoButton
//        onWheel:
//        {
//            if (wheel.modifiers & Qt.ControlModifier)
//            {
//                console.log(wheel.angleDelta)
//                if (wheel.angleDelta.y > 0)
//                    webViewContainer.currentView.zoomFactor += 0.1
//                else
//                    webViewContainer.currentView.zoomFactor -= 0.1
//                wheel.accepted = true
//            }
//
//            wheel.accepted = false
//        }
//    }
}
