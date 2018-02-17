import QtQuick 2.7
import QtQuick.Controls 2.2
import "."

Rectangle
{
    id: root

    signal searchRequested(string text)
    
    property bool stateVisible: false
    opacity: 0.0
    visible: opacity > 0.01
    
    height: Style.findBar.height
    color: Style.findBar.color
    radius: Style.findBar.radius
    border.color: Style.findBar.border.color
    border.width: Style.findBar.border.width

    onVisibleChanged: {
        searchText.focus = true
    }

    MouseArea
    {
        anchors.fill: root
        hoverEnabled: true
        onContainsMouseChanged:
        {
            closeIcon.stateVisible = containsMouse
        }
    }

    TextField
    {
        id: searchText
        
        height: Style.findBar.textHeight
        width: parent.width / 2
        background: Style.findBar.textStyle

        anchors.verticalCenter: parent.verticalCenter
        anchors.left: parent.left
        anchors.leftMargin: Style.margin

        selectByMouse: true
        
        onAccepted: {
            searchRequested(searchText.text)
        }
    }

    CloseIcon
    {
        id: closeIcon
        
        anchors.right: root.right
        anchors.rightMargin: Style.margin
        anchors.verticalCenter: root.verticalCenter
        
        onClose: { root.stateVisible = false }
    }

    states: [
        State { when: root.stateVisible;
            PropertyChanges { target: root; opacity: Style.findBar.opacity}
        },
        State { when: !root.stateVisible;
            PropertyChanges { target: root; opacity: 0.0}
        }
    ]
    transitions: Transition {
        NumberAnimation { property: "opacity"; duration: 250}
    }
}