import QtQuick 2.7
import QtQuick.Controls 2.2
import QtWebEngine 1.5

import "."


Rectangle
{
    id: mainWindow
    objectName: "mainWindow"

    property bool isFullscreen: false

    signal printRequest(var webView)
    signal loadSucceeded(var webView)
    signal savePasswordAccepted(string url, bool accepted)

    visible: true
    color: Style.background

    width: Style.mainWindow.width
    height: Style.mainWindow.height


    AddressBar
    {
        id: addressBar
        
        height: Style.addressBar.height
        background: Style.addressBar.style

        anchors.left: parent.left
        anchors.leftMargin: Style.margin
        anchors.right: downloadHistoryButton.visible ? downloadHistoryButton.left : parent.right
        anchors.rightMargin: Style.margin

        placeholderText: "https://"
        inputMethodHints: Qt.ImhUrlCharactersOnly
        selectByMouse: true

        loadProgress: webViewContainer.currentView ? webViewContainer.currentView.loadProgress / 100.0 : 0
        loadProgressVisible: webViewContainer.currentView ? webViewContainer.currentView.loading : false

        onAccepted: {
            if (text.startsWith("/"))
            {
                text = text.substring(1)
                text = "https://www.google.com/search?q=" + encodeURIComponent(text)
            } else if (text.indexOf("://") == -1)
            {
                text = "https://" + text
            }

            webViewContainer.setUrl(text)
            webViewContainer.setFocus()
        }
    }
    
    Rectangle // FIXME: export as basic button
    {
        id: downloadHistoryButton
        objectName: "downloadHistoryButton"

        height: Style.button.size
        width: Style.button.size

        color: Style.button.background
        radius: Style.button.radius
        border.color: Style.button.border.color
        border.width: Style.button.border.width

        anchors.right: parent.right
        anchors.rightMargin: Style.margin

        visible: false
        
        MouseArea
        {
            anchors.fill: downloadHistoryButton
            onClicked:
            {
                downloadHistoryView.visible = !downloadHistoryView.visible
            }
        }

        Image
        {
            anchors.verticalCenter: parent.verticalCenter
            anchors.horizontalCenter: parent.horizontalCenter
            source: "qrc:/ui/icons/download2.svg"
            sourceSize: Qt.size(Style.button.icon.width, Style.button.icon.height)
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

            onFullScreenRequested: function(request) {
                mainWindow.isFullscreen = request.toggleOn
                viewHandler.showFullscreen(request.toggleOn)
                request.accept()
                webViewContainer.currentView.parent = request.toggleOn ? mainWindow : webViewContainer
            }

            onPdfPrintingFinished: //(string filePath, bool success)
            {
                // FIXME: Possibly show some notification
            }
                
            onLoadingChanged: {
                if (loadRequest.status == WebEngineLoadRequest.LoadSucceededStatus) {
                    loadSucceeded(this)
                }
            }
        }
    }

    FindBar
    {
        id: findBar

        anchors.left: webViewContainer.left
        anchors.leftMargin: Style.margin * 2
        anchors.right: webViewContainer.right
        anchors.rightMargin: Style.margin * 2
        anchors.bottom: webViewContainer.bottom
        anchors.bottomMargin: Style.margin * 2

        onSearchRequested: {
            webViewContainer.currentView.findText(text)
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
    Shortcut {
        sequence: "Ctrl+r"
        onActivated: {
            if (webViewContainer.currentView)
                webViewContainer.currentView.reload()
        }
    }
    Shortcut {
        sequence: "Ctrl+f"
        onActivated: {
            findBar.stateVisible = !findBar.stateVisible
        }
    }
    Shortcut {
        sequence: "Ctrl+p"
        onActivated: {
            mainWindow.printRequest(webViewContainer.currentView)
        }
    }
    Shortcut {
        sequence: "Ctrl+s"
        onActivated: {
            webViewContainer.currentView.triggerWebAction(WebEngineView.SavePage)
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
        anchors.bottomMargin: Style.margin

        onVisibleChanged: {
            webViewContainer.visible = !visible
        }
    }

    DownloadHistoryView
    {
        id: downloadHistoryView
        objectName: "downloadHistoryView"
        visible: false

        anchors.top: addressBar.bottom
        anchors.topMargin: Style.margin
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        anchors.bottomMargin: Style.margin
        
        onVisibleChanged: {
            webViewContainer.visible = !visible
        }
    }
    
    function shouldBeSaved(url, login)
    {
        savePasswordDialog.url = url
        savePasswordDialog.login = login
        savePasswordDialog.open()
    }
    
    function shouldBeUpdated(url, login)
    {
        savePasswordDialog.url = url
        savePasswordDialog.login = login
        savePasswordDialog.open()
    }
    
    Dialog
    {
        id: savePasswordDialog
        title: update ? "Update password?" : "Save password?"
        standardButtons: Dialog.Save | Dialog.Discard

        property string url
        property string login
        property bool update: false
        
        onAccepted: mainWindow.savePasswordAccepted(url, true)
        onRejected: mainWindow.savePasswordAccepted(url, false)
        
        Label {
            text: update ? ("Do you want to update password for site \"" +
                    savePasswordDialog.url + "\" for user \"" +
                    savePasswordDialog.login + "\"?") :
                        ("Do you want to save password for site \"" +
                                savePasswordDialog.url + "\" for user \"" +
                                savePasswordDialog.login + "\"?");
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
