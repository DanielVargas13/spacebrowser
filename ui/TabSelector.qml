import QtQuick 2.7
import QtQuick.Controls 2.2
import QtQml.Models 2.11

Item
{
    id: root

    signal viewSelected(int viewId)           // connected in c++ code
    signal closeTab(int viewId)               // connected in c++ code
    signal openScriptBlockingView(int viewId) // connected in c++ code

    height: Style.tabSelector.entry.height * visualModel.count

    DelegateModel
    {
        id: visualModel
        objectName: "visualModel"

        delegate: TabSelectorEntry
        {
            id: visualModelDelegate
            //title: visualModelDelegate.DelegateModel.model.model.getModelId(model.viewId) + " (" + model.viewId + '): ' + model.title
            title: model.title
            icon: model.icon
            viewId: model.viewId
            color: ListView.isCurrentItem ?
                Style.tabSelector.entry.selected : Style.lightBackground
            anchors.leftMargin: (model.indent+1) * Style.margin

            onClose: {
                root.closeTab(viewId)
            }

            onSelected: {
                console.log("selected: " + index + " viewId: " + viewId)
                tabSelectorView.currentIndex = index
                root.viewSelected(viewId)
            }
            Component.onCompleted: {
//                console.log("------------------------------------------_")
//                console.log(visualModelDelegate.DelegateModel.model.getModelId(viewId))

//                console.log("+++")
//                console.log(visualModelDelegate.DelegateModel.model.model.getModelId(viewId))
//                console.log("+++")
//                console.log(model)
//                for (var p in model)
//                {
//                    console.log(">> " + p)
//                    console.log(">> : " + model[p]);
//                }
//                console.log("---")
//                console.log(JSON.stringify(model))
//                console.log(model.icon)
//                console.log("+++")
            }

        }
    }

    ListView
    {
        id: tabSelectorView

        anchors.fill: parent
        model: visualModel
//        highlightFollowsCurrentItem: false
    }

    Menu {
        id: contextMenu

        background: Style.contextMenu.background
        //FIXME: background is ov erridden by style of ContextMenuEntry, no border is displayed
        //background: Style.contextMenu.background

        ContextMenuEntry {
            text: "Script blocking"
            onTriggered: {
                openScriptBlockingView(tabSelectorView.itemAt(contextMenu.x, contextMenu.y).viewId)
            }
        }
    }

    MouseArea
    {
        anchors.fill: parent
        acceptedButtons: Qt.RightButton // FIXME: add dragging here??

        onClicked: {
            console.log(root.visible)
            console.log(visualModel.count)
            console.log(visualModel.model.count)

            contextMenu.x = mouseX
            contextMenu.y = mouseY
            contextMenu.open()
            mouse.accepted = true
        }
    }

    function viewId2ModelId(viewId)
    {
        return visualModel.model.getModelId(viewId)
    }

    function createNewTab(obj, insertAfter)
    {
        if (insertAfter)
        {
            var modId = viewId2ModelId(insertAfter)
            tabSelectorModel.insert(modId+1, obj)
        }
        else
            tabSelectorModel.append(obj)
    }

    function selectView(modelId)
    {
        tabSelectorView.currentIndex = modelId
    }

    function dumpCurrentModel()
    {
        console.log(">>>>>>")
        for (var i=0; i < tabSelectorModel.count; ++i)
        {
            console.log("modelId: " + i + " viewId: " + tabSelectorModel.get(i).viewId)
        }
        console.log("<<<<<<")
    }

    function setModel(model)
    {
        visualModel.model = model
    }

    function getCurrentItemPosition()
    {
        return mapFromItem(tabSelectorView,
                           tabSelectorView.currentItem.x, tabSelectorView.currentItem.y)
    }
}
