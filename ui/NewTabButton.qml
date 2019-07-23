import QtQuick 2.7
import "."

Rectangle
{
    id:root

    color: Style.lightBackground
    height: Style.tabSelector.entry.height

    signal newTabRequested()

    Text
    {
        text: "+";
        anchors.horizontalCenter: root.horizontalCenter
        anchors.verticalCenter: root.verticalCenter
    }

    MouseArea
    {
        anchors.fill: root
        onClicked:
        {
            newTabRequested()
        }
    }
}
