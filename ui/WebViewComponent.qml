import QtQuick 2.7
import QtWebEngine 1.7
import QtWebChannel 1.0
import QtQuick.Controls 2.2

WebEngineView
{
    id: root

    property int myViewId
    property string dbName
    property var viewContainer
    property var tabModel

    property var targetUrl // FIXME: these two are needed to handle lazy loading of webpages
    property int passCount: 0

    signal updateTitle(int viewId, string title)
    signal updateIcon(int viewId, string iconUri)
    signal updateUrl(int viewId, string url)

    webChannel: WebChannel {
        id: webChan
    }

    anchors.fill: parent

    onTitleChanged: {
        root.updateTitle(myViewId, title)
    }

    onIconChanged: {
        var iconUri = icon.toString().replace("image://favicon/", "")
        root.updateIcon(myViewId, iconUri)
    }

    onNewViewRequested: function(request) {
        var viewId = tabModel.createTab(myViewId)
        var view = tabModel.getView(viewId)
        view.visible = false
        request.openIn(view)
    }

    onUrlChanged: {
        if (viewContainer.currentView &&
            viewContainer.currentView.myViewId == myViewId &&
            viewContainer.currentView.dbName == dbName)
        {
            viewContainer.updateAddressBar(url)
        }

        root.updateUrl(myViewId, url)
    }

    onContextMenuRequested: function(request) {
        request.accepted = true;
        contextMenu.x = request.x;
        contextMenu.y = request.y;

        console.log((request.editFlags & ContextMenuRequest.CanCopy) != 0)

        copyItem.visible = (request.editFlags & ContextMenuRequest.CanCopy) != 0
        cutItem.visible = (request.editFlags & ContextMenuRequest.CanCut) != 0
        pasteItem.visible = (request.editFlags & ContextMenuRequest.CanPaste) != 0
//        deleteItem.visible = (request.editFlags & ContextMenuRequest.CanDelete) != 0
        undoItem.visible = (request.editFlags & ContextMenuRequest.CanUndo) != 0
        redoItem.visible = (request.editFlags & ContextMenuRequest.CanRedo) != 0

        contextMenu.popup();
    }

    function setup() {
        if (url == "" && targetUrl && targetUrl != "")
            url = targetUrl
    }

    function passAvailable(passCount) {
        root.passCount = passCount
    }

    Menu {
        id: contextMenu


        MenuItem {
            text: "Back"
            enabled: root.canGoBack
            onTriggered: root.goBack()
        }
        ContextMenuEntry {
            text: "Forward"
            enabled: root.canGoForward
            onTriggered: root.goForward()
        }
        ContextMenuEntry {
            text: "Reload"
            onTriggered: root.reload()
        }
        ContextMenuEntry {
            text: "Full reload"
            onTriggered: root.reloadAndBypassCache()
        }

        MenuSeparator {
            visible: copyItem.visible || cutItem.visible || pasteItem.visible ||
                undoItem.visible || redoItem.visible
            height: visible ? implicitHeight : 0
        }

        MenuItem {
            id: copyItem
            text: "Copy"
            height: visible ? implicitHeight : 0
            onTriggered: root.triggerWebAction(WebEngineView.Copy)
        }
        MenuItem {
            id: cutItem
            text: "Cut"
            height: visible ? implicitHeight : 0
            onTriggered: root.triggerWebAction(WebEngineView.Cut)
        }
        MenuItem {
            id: pasteItem
            text: "Paste"
            height: visible ? implicitHeight : 0
            onTriggered: root.triggerWebAction(WebEngineView.Paste)
        }
/*        MenuItem { // FIXME: there seems to be no action to perform delete
            id: deleteItem
            text: "Delete"
            height: visible ? implicitHeight : 0
            onTriggered: root.triggerWebAction(WebEngineView.Delete)
            }*/
        MenuItem {
            id: undoItem
            text: "Undo"
            height: visible ? implicitHeight : 0
            onTriggered: root.triggerWebAction(WebEngineView.Undo)
        }
        MenuItem {
            id: redoItem
            text: "Redo"
            height: visible ? implicitHeight : 0
            onTriggered: root.triggerWebAction(WebEngineView.Redo)
        }

        MenuSeparator { }

        ContextMenuEntry {
            text: "Save page"
            onTriggered: root.triggerWebAction(WebEngineView.SavePage)
        }

        MenuSeparator { }

        ContextMenuEntry {
            text: "View source"
            onTriggered: root.triggerWebAction(WebEngineView.ViewSource)
        }
        ContextMenuEntry {
            text: "Inspect element"
            onTriggered: root.triggerWebAction(WebEngineView.InspectElement)
        }

    }
}
