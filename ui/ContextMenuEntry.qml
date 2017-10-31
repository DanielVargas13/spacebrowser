import QtQuick 2.7
import QtQuick.Controls 2.2
import "."

MenuItem
{
    id: root
    
    text: "Script blocking"
        
    background: Rectangle {
        color: mouseAreaMenu.containsMouse ? Style.contextMenu.entry.selected : Style.contextMenu.background.color
//        border.width: 1
//        border.color: Style.contextMenu.borderColor
    }
    
    MouseArea {
        id: mouseAreaMenu
        anchors.fill: parent
        hoverEnabled: true
        onClicked: root.triggered() 
    }
    
}