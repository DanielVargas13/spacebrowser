import QtQuick 2.7
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

    signal openUrl(string url)

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
                console.log("CLICKED")
                root.openUrl(pathText.text)
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

            componentBorderColor: Style.downloadProgressComponent.border.color
            componentColor: Style.downloadProgressComponent.color

            stateVisible: root.header ? false : true
            progress: received / total

            text: progress == total ? "Finished " : "Downloading: [" + received + " / " + total + "] "
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
}