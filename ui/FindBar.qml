import QtQuick 2.7
import QtQuick.Controls 2.2
import "."

Rectangle
{
    id: root

    signal searchRequested(string text, bool backward, bool caseSensitive)
    
    property bool stateVisible: false
    property bool caseSensitive: false
    
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
            searchRequested(searchText.text, false, caseSensitive)
        }
    }

    BasicButton
    {
        id: searchForward

        anchors.left: searchText.right
        anchors.leftMargin: Style.margin
        anchors.verticalCenter: parent.verticalCenter
        visible: true

        source: "qrc:/ui/icons/searchDown.svg"

        MouseArea
        {
            anchors.fill: searchForward
            onClicked: searchRequested(searchText.text, false, caseSensitive)
        }
    }

    BasicButton
    {
        id: searchBackward

        anchors.left: searchForward.right
        anchors.leftMargin: Style.margin
        anchors.verticalCenter: parent.verticalCenter
        visible: true

        source: "qrc:/ui/icons/searchUp.svg"

        MouseArea
        {
            anchors.fill: searchBackward
            onClicked: searchRequested(searchText.text, true, caseSensitive)
        }
    }

    BasicButton
    {
        id: caseSensitivity

        anchors.left: searchBackward.right
        anchors.leftMargin: Style.margin
        anchors.verticalCenter: parent.verticalCenter
        visible: true
        stateful: true
        pushed: caseSensitive

        source: "qrc:/ui/icons/caseInsensitive.svg"
        pushedSource: "qrc:/ui/icons/caseSensitive.svg"

        MouseArea
        {
            anchors.fill: caseSensitivity
            onClicked: caseSensitive = !caseSensitive
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

    function switchVisibleState()
    {
        if (root.stateVisible) {
            searchRequested("", false, false)
        }
        else {
            searchText.selectAll()
        }
            
        root.stateVisible = !root.stateVisible
    }
}
