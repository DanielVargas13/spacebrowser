import QtQuick 2.7
import "."

Image
{
    id: root

    signal close()

    property bool stateVisible: false
    opacity: 0.0
    visible: opacity > 0.01

    sourceSize: Qt.size(Style.closeIcon.width, Style.closeIcon.height)
    source: "qrc:/ui/icons/close.svg"

    MouseArea
    {
        anchors.fill: root
        onClicked:
        {
            root.close()
            mouse.accepted = true
        }
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