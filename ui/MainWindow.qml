import QtQuick 2.7
import QtQuick.Controls 2.2
import QtWebEngine 1.7

import "."


Rectangle
{
    id: mainWindow
    objectName: "mainWindow"

    property bool isFullscreen: false

    signal createTab(int parent, bool select, bool scroll)
    signal closeTab(int viewId)
    signal loadSucceeded(var webView)
    signal nextTab()
    signal prevTab()
    signal printRequest(var webView)
    signal savePasswordAccepted(string url, bool accepted)
    signal showFullscreen(bool toggleOn)

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
        anchors.right: passwordManagerButton.visible ?
            passwordManagerButton.left : downloadHistoryButton.visible ?
            downloadHistoryButton.left : parent.right
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

    // FIXME: put those buttons in some (invisible) container
    BasicButton
    {
        id: passwordManagerButton
        objectName: "passwordManagerButton"

        signal passwordFillRequest(var webView)

        anchors.right: downloadHistoryButton.visible ?
            downloadHistoryButton.left : parent.right
        anchors.rightMargin: Style.margin

        source: "qrc:/ui/icons/lock.svg"
        visible: webViewContainer.currentView ?
            webViewContainer.currentView.passCount > 0 : false

        MouseArea
        {
            anchors.fill: passwordManagerButton
            onClicked: passwordManagerButton.passwordFillRequest(webViewContainer.currentView)
        }
    }

    BasicButton
    {
        id: downloadHistoryButton
        objectName: "downloadHistoryButton"

        anchors.right: parent.right
        anchors.rightMargin: Style.margin

        source: "qrc:/ui/icons/download2.svg"

        MouseArea
        {
            anchors.fill: downloadHistoryButton
            onClicked:
            {
                downloadHistoryView.visible = !downloadHistoryView.visible
            }
        }
    }

    TabSelectorPanel
    {
        id: tabSelectorPanel
        objectName: "tabSelectorPanel"

        visible: true;

        anchors.top: addressBar.bottom
        anchors.topMargin: Style.margin
        anchors.right: parent.right
        anchors.bottom: mainWindow.bottom
        width: Style.tabSelector.width

        onNewTabRequested:
        {
            mainWindow.createTab(0, true, true)
            addressBar.focus = true
        }
    }

    WebViewContainer
    {
        id: webViewContainer
        objectName: "webViewContainer"

        anchors.top: addressBar.bottom
        anchors.topMargin: Style.margin
        anchors.left: parent.left
        anchors.right: tabSelectorPanel.left
        anchors.bottom: devToolsView.visible ? devToolsView.top : parent.bottom

        onVisibleChanged:
        {
            tabSelectorPanel.visible = visible
        }

        function createViewObject(newViewId)
        {
            var view = viewComp.createObject(webViewContainer);
            view.zoomFactor = 1.2 // FIXME: set in style or configuration
            view.myViewId = newViewId

            return view;
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
                showFullscreen(request.toggleOn)
                request.accept()
                webViewContainer.currentView.parent = request.toggleOn ? mainWindow : webViewContainer
            }

            onPdfPrintingFinished: //(string filePath, bool success)
            {
                // FIXME: Possibly show some notification
            }

            onLoadingChanged: {
                if (loadRequest.status == WebEngineLoadRequest.LoadSucceededStatus) {
                    mainWindow.loadSucceeded(this)
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
            var flags = backward ? WebEngineView.FindBackward : 0
            if (caseSensitive)
                flags = flags | WebEngineView.FindCaseSensitively

            webViewContainer.currentView.findText(text, flags)
        }
    }

    WebEngineView
    {
        id: devToolsView
        visible: false
        height: 300
	inspectedView: webViewContainer.currentView

        anchors.topMargin: Style.margin
        anchors.left: parent.left
        anchors.right: tabSelectorPanel.left
        anchors.bottom: parent.bottom
    }


    Shortcut {
        sequence: "Ctrl+Tab"
        onActivated: nextTab();
    }
    Shortcut {
        sequence: "Ctrl+Shift+Tab"
        onActivated: prevTab();
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
            mainWindow.createTab(0, true, true)
            addressBar.focus = true
        }
    }
    Shortcut {
        sequence: "Ctrl+w"
        onActivated: {
            closeTab(webViewContainer.currentView.myViewId)
        }
    }
    Shortcut { // test shorcut
        sequence: "Ctrl+g"
        onActivated: {

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
            findBar.switchVisibleState()
        }
    }
    Shortcut {
        sequence: "Ctrl+o"
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
    Shortcut {
        sequence: "Ctrl+d"
        onActivated: {
            devToolsView.visible = !devToolsView.visible
        }
    }
    Shortcut {
        sequence: "Ctrl+n"
        onActivated: {
            if (findBar.visible) {
                findBar.searchForward()
            }
        }
    }
    Shortcut {
        sequence: "Ctrl+p"
        onActivated: {
            if (findBar.visible) {
                findBar.searchBackward()
            }
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
        standardButtons: Dialog.Save | Dialog.Cancel

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

    function configureEncryption(model)
    {
        encryptionKeyConfigDialogCB.model = model;
        encryptionKeyConfigDialog.open();
    }

    Dialog
    {
        id: encryptionKeyConfigDialog
        objectName: "encryptionKeyConfigDialog"
        title: "Configure encryption key"
        standardButtons: Dialog.Save | Dialog.Cancel
        closePolicy: Popup.CloseOnEscape
        modal: true

        signal keySelected(string id)

        width: parent.width / 2
        height: parent.height / 2

        x: parent.width / 2 - width / 2
        y: parent.height / 2 - height / 2

        onAccepted: keySelected(encryptionKeyConfigDialogCB.currentText)

        ComboBox {
            id: encryptionKeyConfigDialogCB

            width: encryptionKeyConfigDialog.width * 0.9
        }
    }

    DbConnectionDialog
    {
        id: configureDbConnectionDialog
        objectName: "configureDbConnectionDialog"

        width: parent.width / 4

        x: parent.width / 2 - width / 2
        y: parent.height / 2 - height / 2

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
