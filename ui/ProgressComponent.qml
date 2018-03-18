import QtQuick 2.7
import "."

Rectangle
{
    id: root

    property real progress: 0.0
    property bool stateVisible: false
    property string text: ""
    property color componentColor: Style.progressComponent.color
    property color componentBorderColor: Style.progressComponent.border.color
    visible: opacity > 0.01
    
    width: Style.progressComponent.width
    height: Style.progressComponent.height

    border.width: Style.progressComponent.border.width
    border.color: root.componentBorderColor
    radius: Style.progressComponent.radius
    
    color: Style.progressComponent.background
    opacity: Style.progressComponent.opacity

    Rectangle
    {
        anchors.left: root.left
        anchors.verticalCenter: root.verticalCenter

        width: root.width * progress
        height: root.height

        radius: Style.progressComponent.radius
        color: root.componentColor
        opacity: Style.progressComponent.opacity
    }

    Text
    {
        anchors.verticalCenter: root.verticalCenter
        anchors.horizontalCenter: root.horizontalCenter
        
        text: root.text + Math.round(progress * 100) + "%"
    }

    states: [
        State { when: root.stateVisible;
            PropertyChanges { target: root; opacity: 1.0}
        },
        State { when: !root.stateVisible;
            PropertyChanges { target: root; opacity: 0.0}
        }
    ]
    transitions: Transition {
        NumberAnimation { property: "opacity"; duration: 250}
    }

}