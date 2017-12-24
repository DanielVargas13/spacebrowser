import QtQuick 2.7
import "."

Rectangle
{
    id: root
    color: Style.tabSelector.entry.background
    property string title: ""
    property string icon: ""
    property int viewId: 0

    height: Style.tabSelector.entry.height

    //anchors.top: parent.top
    anchors.left: parent.left
    anchors.right: parent.right
    anchors.leftMargin: Style.margin
    anchors.rightMargin: Style.margin

    border.width: Style.tabSelector.entry.border.width
    border.color: Style.tabSelector.entry.border.color

    signal close()
    signal selected()

    Image
    {
        id: tabIcon
        sourceSize: Qt.size(Style.tabSelector.entry.icon.width, Style.tabSelector.entry.icon.height)
        source: icon
        
        anchors.left: parent.left
        anchors.leftMargin: Style.margin
        anchors.verticalCenter: parent.verticalCenter
    }

    Text
    {
        id: tabTitle
        anchors.left: tabIcon.right
        anchors.leftMargin: Style.margin
        anchors.verticalCenter: parent.verticalCenter
        anchors.right: closeIcon.left
        anchors.rightMargin: Style.margin

        elide: Text.ElideRight
        text: title
    }

    MouseArea
    {
        anchors.fill: root
        hoverEnabled: true
        onClicked:
        {
            console.log(mouse.button)
            if (mouse.button & Qt.MiddleButton)
            {
                root.close()
                mouse.accepted = true
                console.log("middle")
            }
            else if (mouse.button == Qt.LeftButton)
            {
                root.selected()
                mouse.accepted = true
                console.log("left")
            }
        }
        onEntered:
        {
            closeIcon.stateVisible = true
        }
        onExited:
        {
            closeIcon.stateVisible = false
        }
    }

    Image
    {
        id: closeIcon
        
        property bool stateVisible: false
        visible: opacity > 0.01

        sourceSize: Qt.size(Style.tabSelector.entry.closeIcon.width, Style.tabSelector.entry.closeIcon.height)
        source: "qrc:/ui/icons/close.svg"

        anchors.verticalCenter: parent.verticalCenter
        anchors.right: parent.right
        anchors.rightMargin: Style.margin

        MouseArea
        {
            anchors.fill: closeIcon
            onClicked:
            {
                root.close()
                mouse.accepted = true
            }
        }

        states: [
            State { when: closeIcon.stateVisible;
                PropertyChanges { target: closeIcon; opacity: 1.0}
            },
            State { when: !closeIcon.stateVisible;
                PropertyChanges { target: closeIcon; opacity: 0.0}
            }
        ]
        transitions: Transition {
            NumberAnimation { property: "opacity"; duration: 250}
        }
    }
}
