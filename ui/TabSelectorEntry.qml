import QtQuick 2.7
import QtQml.Models 2.11
import "."

//Rectangle
//{
MouseArea {
    id: root

    property bool held: false

    //    anchors { left: parent.left; right: parent.right }
    height: content.height
    anchors.left: parent.left
    anchors.right: parent.right
    anchors.leftMargin: Style.margin
    anchors.rightMargin: Style.margin

    drag.target: held ? content : undefined
    drag.axis: Drag.YAxis

    onPressAndHold: held = true
    onReleased: held = false

    acceptedButtons: Qt.MiddleButton | Qt.LeftButton
    hoverEnabled: true
    onClicked:
    {
        if (mouse.button & Qt.MiddleButton)
        {
            root.close()
            mouse.accepted = true
        }
        else if (mouse.button == Qt.LeftButton)
        {
            root.selected()
            mouse.accepted = true
        }
    }
    onContainsMouseChanged:
    {
        closeIcon.stateVisible = containsMouse
    }


    signal close()
    signal selected()

    property alias title: content.title
    property alias icon: content.icon
    property alias viewId: content.viewId
    property alias color: content.color
    property alias border: content.border

    Component.onCompleted: {
//        console.log(root.title)
//        console.log(root.visible)
//        console.log(content.height)
/*
        console.log(root.height)
        console.log("parents:")
        console.log(root.parent.height)
        console.log(root.parent.parent.height)
        console.log(root.parent.parent.parent.height)
        console.log(root.parent.parent.parent.parent.height)
        console.log(root.parent.parent.parent.parent.parent.height)
        console.log(root.parent.parent.parent.parent.parent.parent.height)
        console.log(root.parent.parent.parent.parent.parent.parent.parent.height)
        console.log(root.parent.parent.parent.parent.parent.parent.parent.parent.height)
        */

//        var globalCoordinares = root.mapToItem(root.parent.parent.parent.parent.parent.parent.parent.parent, 0, 0)
//        console.log("x: " + globalCoordinares.x + " y: " + globalCoordinares.y)
//        console.log("w: " + root.width + " h: " + root.height)
    }

    Rectangle {
        id: content
        color: Style.tabSelector.entry.background
        property string title: ""
        property string icon: ""
        property int viewId: 0

        height: Style.tabSelector.entry.height

        anchors.left: parent.left
        anchors.right: parent.right
        anchors.verticalCenter: parent.verticalCenter

        border.width: Style.tabSelector.entry.border.width
        border.color: Style.tabSelector.entry.border.color

        Drag.active: root.held
        Drag.source: root
        Drag.hotSpot.x: width / 2
        Drag.hotSpot.y: height / 2

        states: State {
            when: root.held
            ParentChange { target: content; parent: root.parent.parent }
            AnchorChanges {
                target: content
                anchors.verticalCenter: undefined
            }
        }

        Image
        {
            id: tabIcon
            sourceSize: Qt.size(Style.tabSelector.entry.icon.width, Style.tabSelector.entry.icon.height)
            source: content.icon

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
            text: content.title
        }

        CloseIcon
        {
            id: closeIcon

            anchors.verticalCenter: parent.verticalCenter
            anchors.right: parent.right
            anchors.rightMargin: Style.margin

            onClose: root.close()
        }
    }
    DropArea {
        anchors { fill: root; margins: 1 }

        onEntered: {
//            root.drawMarker(visualModel.items.get(
//                drag.source.DelegateModel.itemsIndex - 1), drag.y, height)

//            console.log("xoot:" + root.DelegateModel.itemsIndex)
//            console.log("xurr:" + drag.source.DelegateModel.itemsIndex)
            visualModel.items.move(
                drag.source.DelegateModel.itemsIndex,
                root.DelegateModel.itemsIndex)
            // draw box to differentiate if will be inserted as root or sibling
        }
        onPositionChanged: {
//            console.log("root:" + root.DelegateModel.itemsIndex)
//            console.log("curr:" + drag.source.DelegateModel.itemsIndex)
//            root.drawMarker(visualModel.items.get(
//                drag.source.DelegateModel.itemsIndex - 1), drag.y, height)
        }
        onExited: {
//            clearMarker(drag.source)
            // hide marker
        }
        onDropped: {

            // hide marker
// change position in real model, change update parent
        }
    }

    function drawMarker(entry, y, height)
    {


        if (y < height * 0.20) {
            console.log(entry)
            entry.border.width = Style.tabSelector.entry.target.width
            entry.border.color = Style.tabSelector.entry.target.color
            console.log("child")
        }
        else if (y < height * 0.70)
            console.log("sibling")
        else
            console.log("root")

    }

    function clearMarker(entry)
    {
        entry.border.width = Style.tabSelector.entry.border.width
        entry.border.color = Style.tabSelector.entry.border.color
    }
}
