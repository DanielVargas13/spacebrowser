import QtQuick 2.7
import QtQuick.Controls 2.2

import "."

Item {
    PanelSelector
    {
        id: panelSelector
        objectName: "panelSelector"

        height: 50

        anchors.left: parent.left
        anchors.top: parent.top
        anchors.right: parent.right
    }

    ScrollView
    {
        id: root

        clip: true

        anchors.left: parent.left
        anchors.top: panelSelector.bottom
        anchors.bottom: parent.bottom
        anchors.right: parent.right

        signal newTabRequested()

        Item
        {
            id: tabSelectorItem

            implicitWidth: Style.tabSelector.width
            implicitHeight: tabSelector.height + newTabButton.height

            Item
            {
                id: tabSelectorWrapper
                
                anchors.left: parent.left
                anchors.top: parent.top
                anchors.right: parent.right
                height: tabSelector.getHeight()
                // children:
                TabSelector
                {
                    id: tabSelector
                    objectName: "tabSelector"

                    anchors.fill: parent
                }
            }

            NewTabButton
            {
                id: newTabButton

                anchors.top: tabSelectorWrapper.bottom
                anchors.right: parent.right
                anchors.left: parent.left
                anchors.leftMargin: Style.margin
                anchors.rightMargin: Style.margin

                onNewTabRequested:
                {
                    console.log(">>>>>>>>>>>>. " + tabSelector.height)
                    console.log(">>>>>>>>>>>>. " + tabSelector.implicitHeight)
                    console.log(">>>>>>>>>>>>. " + tabSelector.getCount())

                    root.newTabRequested()
                }
            }
        }

        function getTabSelector()
        {
            return tabSelectorWrapper.children
        }

        function createNewTab(obj, insertAfter)
        {
            tabSelector.createNewTab(obj, insertAfter)
        }

        function scrollToBottom()
        {
            if (tabSelectorItem.height - contentItem.height < 0)
                contentItem.contentY = 0
            else
                contentItem.contentY = tabSelectorItem.height - contentItem.height
        }

        function scrollToCurrent()
        {
            var itemPosition = tabSelector.getCurrentItemPosition().y
            var bottom = tabSelectorItem.height - contentItem.height
            var newPosition = itemPosition - (contentItem.height / 2)

            if (newPosition < 0)
            {
                contentItem.contentY = 0
            }
            else if (newPosition >= bottom)
                scrollToBottom()
            else
                contentItem.contentY = itemPosition - contentItem.height / 2
        }

//        function setModel(model)
//        {
//            tabSelector.setModel(model)
//        }

        function createTab()
        {// FIXME: this is better than signal, easier to decide on which tabSelector to call it
            // and will be accessible from WebViewComponent.qml
            return tabSelector.createTab()
        }
        
    }

    function setModel(model)
    {
        tabSelector.setModel(model)
    }
}
