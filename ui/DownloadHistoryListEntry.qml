import QtQuick 2.7
import QtQuick.Controls 2.2
import "."

Item
{
    id: root
    
    property bool header: false
    property int myId
    property string url
    property string path
    property real received
    property real total
    property color entryColor: myId%2 ? Style.lightBackground : Style.commonListView.entry.lightBackground
    property color selectedColor: Style.downloadHistoryView.openFileHighlight
    property bool paused: false
    property bool canceled: false
    property bool finished: false

    signal openUrl(string url)
    signal pause(int id)
    signal resume(int id)
    signal cancel(int id)

    anchors.left: parent.left
    anchors.leftMargin: Style.commonListView.margin
    anchors.right: parent.right
    anchors.rightMargin: Style.commonListView.margin

    height: Style.commonListView.entry.height

    Rectangle
    {
        id: urlRectangle
        color: header ? Style.commonListView.headerBackground : root.entryColor

        anchors.left: parent.left
        anchors.right: pathRectangle.left
        anchors.rightMargin: Style.margin
        anchors.top: parent.top
        anchors.bottom: parent.bottom
        
        Text
        {
            id: urlText
            anchors.left: parent.left
            anchors.leftMargin: Style.margin
            anchors.right: parent.right
            anchors.rightMargin: Style.margin
            anchors.verticalCenter: parent.verticalCenter
            
            elide: Text.ElideRight
            text: root.url
        }
    }
    
    Rectangle
    {
        id: pathRectangle
        color: header ? Style.commonListView.headerBackground :
            (pathRectangleMouse.containsMouse ? root.selectedColor : root.entryColor)

        anchors.right: progressRectangle.left
        anchors.rightMargin: Style.margin
        anchors.top: parent.top
        anchors.bottom: parent.bottom
        
        width: root.width / 3
        
        Text
        {
            id: pathText
            anchors.left: parent.left
            anchors.leftMargin: Style.margin
            anchors.right: parent.right
            anchors.rightMargin: Style.margin
            anchors.verticalCenter: parent.verticalCenter
            
            elide: Text.ElideRight
            text: root.path
        }
        
        MouseArea
        {
            id: pathRectangleMouse
            anchors.fill: parent
            hoverEnabled: true
            
            onClicked: {
                root.openUrl(root.path)
            }
        }
    }
    
    Rectangle
    {
        id: progressRectangle
        color: header ? Style.commonListView.headerBackground : Style.background

        anchors.right: parent.right
        anchors.rightMargin: Style.margin
        anchors.top: parent.top
        anchors.bottom: parent.bottom

        width: root.width / 4

        ProgressComponent
        {
            anchors.fill: parent

            componentColor: root.canceled ? Style.downloadProgressComponent.canceled : (
                    root.paused ? Style.downloadProgressComponent.paused :
                        Style.downloadProgressComponent.color)
            componentBorderColor: componentColor

            stateVisible: root.header ? false : true
            progress: received / total

            showProgress: !(root.canceled || root.finished) 
            text: root.canceled ? "Canceled" :
                root.paused ? "Paused: [" + received + " / " + total + "] " :
                    root.finished ? "Finished" : "Downloading: [" + received + " / " + total + "] "
        }

        Text
        {
            id: progressText
            anchors.left: parent.left
            anchors.leftMargin: Style.margin
            anchors.right: parent.right
            anchors.rightMargin: Style.margin
            anchors.verticalCenter: parent.verticalCenter

            visible: header

            elide: Text.ElideRight
            text: "Download Progress"
        }
    }

    Menu {
        id: contextMenu

        background: Style.contextMenu.background

        ContextMenuEntry {
            text: "Pause"
            enabled: !root.finished && !root.canceled && !root.paused
            onTriggered: {
                root.pause(root.myId)
            }
        }
        ContextMenuEntry {
            text: "Resume"
            enabled: root.paused
            onTriggered: {
                root.resume(root.myId)
            }
        }
        ContextMenuEntry {
            text: "Cancel"
            enabled: !root.finished && !root.canceled
            onTriggered: {
                root.cancel(root.myId)
            }
        }
        
        
        
//        ContextMenuEntry {
//            text: "Remove from list"
//            enabled: false
//            onTriggered: {
//                //removeFromList(downloadHistoryListView.itemAt(contextMenu.x, contextMenu.y))
//            }
//        }
//        ContextMenuEntry {
//            text: "Remove from disk"
//            enabled: false
//            onTriggered: {
//                // removeFromDisk
//            }
//        }
//        MenuSeparator { }
//
//        Menu
//        {
//            title: "Clear..."
//
//            ContextMenuEntry {
//                text: "finished"
//                enabled: false
//            }
//            ContextMenuEntry {
//                text: "failed"
//                enabled: false
//            }
//            ContextMenuEntry {
//                text: "all"
//                enabled: false
//            }
//        }
    }
    
    MouseArea
    {
        anchors.fill: root
        acceptedButtons: Qt.RightButton

        onClicked: {
            if (header) return;

            contextMenu.x = mouseX
            contextMenu.y = mouseY
            contextMenu.open()
            mouse.accepted = true
        }
    }
}
