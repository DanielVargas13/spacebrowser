import QtQuick 2.7
import QtQuick.Controls 2.2

import "."

Item {
    id: root

    signal openScriptBlockingView(string dbName, int viewId)
    signal newTabRequested()

    PanelSelector
    {
        id: panelSelector
        objectName: "panelSelector"

        height: Style.panelSelector.height

        anchors.left: parent.left
        anchors.leftMargin: Style.margin
        anchors.top: parent.top
        anchors.right: parent.right
    }

    ScrollView
    {
        id: scrollView

        clip: true

        anchors.left: parent.left
        anchors.top: panelSelector.bottom
        anchors.bottom: parent.bottom
        anchors.right: parent.right

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

                    onOpenScriptBlockingView:
                    {
                        root.openScriptBlockingView(dbName, viewId)
                    }
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

    function scrollToBottom()
    {
        if (tabSelectorItem.height - scrollView.contentItem.height < 0)
            scrollView.contentItem.contentY = 0
        else
            scrollView.contentItem.contentY = tabSelectorItem.height - scrollView.contentItem.height
    }

    function scrollToCurrent()
    {
        var itemPosition = tabSelector.getCurrentItemPosition().y
        var bottom = tabSelectorItem.height - scrollView.contentItem.height
        var newPosition = itemPosition - (scrollView.contentItem.height / 2)

        if (newPosition < 0)
        {
            scrollView.contentItem.contentY = 0
        }
        else if (newPosition >= bottom)
            scrollToBottom()
        else
            scrollView.contentItem.contentY = itemPosition - scrollView.contentItem.height / 2
    }
}
