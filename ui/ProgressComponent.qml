import QtQuick 2.7
import "."

Rectangle
{
    id: root

    property real progress: 0.0
    property bool stateVisible: false
    visible: opacity > 0.01
    
    width: Style.progressComponent.width
    height: Style.progressComponent.height

    border.width: Style.progressComponent.border.width
    border.color: Style.progressComponent.border.color
    radius: Style.progressComponent.radius
    
    color: Style.progressComponent.background
    opacity: Style.progressComponent.opacity

    Rectangle
    {
        anchors.left: root.left
        anchors.verticalCenter: root.verticalCenter

        width: Style.progressComponent.width * progress
        height: Style.progressComponent.height

        radius: Style.progressComponent.radius
        color: Style.progressComponent.color
        opacity: Style.progressComponent.opacity
    }

    Text
    {
        anchors.verticalCenter: root.verticalCenter
        anchors.horizontalCenter: root.horizontalCenter
        
        text: "Loading page: " + Math.round(progress * 100) + "%"
    }

    states: [
        State { when: stateVisible;
            PropertyChanges { target: root; opacity: 1.0}
        },
        State { when: !stateVisible;
            PropertyChanges { target: root; opacity: 0.0}
        }
    ]
    transitions: Transition {
        NumberAnimation { property: "opacity"; duration: 250}
    }

}