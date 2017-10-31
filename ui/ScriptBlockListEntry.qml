import QtQuick 2.7
import "."

Item
{
    id: root
    
    property string url
    property bool allowed
    property bool gallowed
    property int myId
    
    anchors.left: parent.left
    anchors.leftMargin: Style.scriptBlockingView.margin
    anchors.right: parent.right
    anchors.rightMargin: Style.scriptBlockingView.margin
    
    height: Style.scriptBlockingView.entry.height
    
    Rectangle
    {
        color: myId%2 ? Style.lightBackground : Style.scriptBlockingView.entry.lightBackground
        
        anchors.left: parent.left
        anchors.right: localWhitelist.left
        anchors.rightMargin: Style.margin
        anchors.top: parent.top
        anchors.bottom: parent.bottom
        
        Text
        {
            id: tabTitle
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
        id: localWhitelist
        anchors.right: globalWhitelist.left
        anchors.rightMargin: Style.margin
        anchors.top: parent.top
        anchors.bottom: parent.bottom

        width: Style.scriptBlockingView.colWidth

        color: myId%2 ? Style.lightBackground : Style.scriptBlockingView.entry.lightBackground

        Rectangle
        {
            anchors.verticalCenter: parent.verticalCenter
            anchors.horizontalCenter: parent.horizontalCenter
            
            width: Style.scriptBlockingView.entry.button.size
            height: Style.scriptBlockingView.entry.button.size
            
            color: root.allowed ? Style.scriptBlockingView.entry.button.allowed :
                Style.scriptBlockingView.entry.button.blocked
                
            MouseArea
            {
                anchors.fill: parent
                onClicked: {
                    root.allowed = !root.allowed 
                }
            }
        }
    }
    
    Rectangle
    {
        id: globalWhitelist
        
        anchors.right: parent.right
        anchors.top: parent.top
        anchors.bottom: parent.bottom

        width: Style.scriptBlockingView.colWidth

        color: myId%2 ? Style.lightBackground : Style.scriptBlockingView.entry.lightBackground

        Rectangle
        {
            anchors.verticalCenter: parent.verticalCenter
            anchors.horizontalCenter: parent.horizontalCenter
            
            width: Style.scriptBlockingView.entry.button.size
            height: Style.scriptBlockingView.entry.button.size
            
            color: root.gallowed ? Style.scriptBlockingView.entry.button.allowed :
                Style.scriptBlockingView.entry.button.blocked
                
            MouseArea
            {
                anchors.fill: parent
                onClicked: {
                    root.gallowed = !root.gallowed 
                }
            }
        }
    }
}