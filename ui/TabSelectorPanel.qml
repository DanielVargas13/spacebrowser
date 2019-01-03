import QtQuick 2.7
import QtQuick.Controls 2.2

import "."

ScrollView
{
    id: root

    clip: true

    signal newTabCreated()

    Item
    {
        id: tabSelectorItem

        implicitWidth: Style.tabSelector.width
        implicitHeight: tabSelector.height + newTabButton.height

        TabSelector
        {
            id: tabSelector
            objectName: "tabSelector"

            anchors.left: parent.left
            anchors.top: parent.top
            anchors.right: parent.right
        }

        NewTabButton
        {
            id: newTabButton

            anchors.top: tabSelector.bottom
            anchors.right: parent.right
            anchors.left: parent.left
            anchors.leftMargin: Style.margin
            anchors.rightMargin: Style.margin

            onNewTabCreated: root.newTabCreated()
        }
    }

    function createNewTab(obj, insertAfter)
    {
        tabSelector.createNewTab(obj, insertAfter)
    }

    function updateTitle(viewId, title)
    {
        tabSelector.updateTitle(viewId, title)
    }

    function updateIcon(viewId, icon)
    {
        tabSelector.updateIcon(viewId, icon)
    }

    function setModel(model)
    {
        tabSelector.setModel(model)
    }
}
