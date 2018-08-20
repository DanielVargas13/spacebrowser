import QtQuick 2.7

Rectangle
{
    id: root

    property string source
    property string pushedSource
    property bool stateful: false
    property bool pushed: false

    height: Style.button.size
    width: Style.button.size

    visible: false
    color: Style.button.background
    radius: Style.button.radius
    border.color: Style.button.border.color
    border.width: Style.button.border.width

    Image
    {
        anchors.verticalCenter: root.verticalCenter
        anchors.horizontalCenter: root.horizontalCenter
        source: (root.stateful && root.pushed) ? root.pushedSource : root.source
        sourceSize: Qt.size(Style.button.icon.width, Style.button.icon.height)
    }
}
