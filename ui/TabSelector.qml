import QtQuick 2.7
import QtQuick.Controls 2.2
import QtQml.Models 2.11

Item
{
    id: root

    signal viewSelected(int viewId)           // connected in c++ code
    signal closeTab(int viewId)               // connected in c++ code
    signal openScriptBlockingView(int viewId) // connected in c++ code

    //height: Style.tabSelector.entry.height * tabSelectorModel.count
    height: Style.tabSelector.entry.height * visualModel.count

    // FIXME: remove this:
    ListModel
    {
        id: tabSelectorModel
    }

    DelegateModel
    {
        id: visualModel
        objectName: "visualModel"
        model: tabSelectorModel

        delegate: TabSelectorEntry
        {
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
//                console.log(model)
//                for (var p in model)
//                {
//                    console.log(">> " + p)
//                    console.log(">> : " + model[p]);
//                }
//                console.log(JSON.stringify(model))
//                console.log(model.title)
            }

        }
    }

    ListView
    {
        id: tabSelectorView

        anchors.fill: parent
        model: visualModel
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
        for (var i=0; i < tabSelectorModel.count; ++i)
        {
            if (tabSelectorModel.get(i).viewId == viewId)
                return i
        }
        return -1
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

//    function fixIndentation(viewId, _indent)
//    {
//        var modelId = viewId2ModelId(viewId)
//        tabSelectorModel.get(modelId).indent = _indent // FIXME: coredumps sometimes
//    }

    function updateTitle(viewId, title)
    {
        var id = viewId2ModelId(viewId)
        if (id >= 0)
            tabSelectorModel.get(id).title = title
    }

    function selectView(modelId)
    {
        console.log(tabSelectorView.currentIndex)
        console.log("selecting " + modelId);
        console.log("model size: " + visualModel.model.count)
        tabSelectorView.currentIndex = modelId
        console.log(tabSelectorView.currentIndex)
    }

    function updateIcon(viewId, icon)
    {
        var id = viewId2ModelId(viewId)
        if (id >= 0)
            tabSelectorModel.get(id).icon = icon
    }

/*
    function removeTabEntry(viewId)
    {
        for (var i=0; i < tabSelectorModel.count; ++i)
        {
            if (tabSelectorModel.get(i).viewId != viewId)
                continue

            tabSelectorModel.remove(i)

            /// if currently visible tab is closed, select next tab before it in the model
            if (tabSelectorView.currentIndex == i && tabSelectorModel.count > 0)
            {//FIXME: select first child, then next on the same level, then parent
                // doing it correctly will probably require proper tree model
                var j = 0
                if (i > 0)
                    j = i - 1

                tabSelectorView.currentIndex = j
                root.viewSelected(tabSelectorModel.get(j).viewId)
            }

            //dumpCurrentModel()//logging
            return
        }
    }
*/
    function getNextTab(viewId)
    {
        var id = viewId2ModelId(viewId)
        var nextId;

        if (id >= 0)
        {
            nextId = id+1
            if (tabSelectorModel.count == nextId)
                nextId = 0;

            return tabSelectorModel.get(nextId).viewId
        }
        return
    }

    function getPrevTab(viewId)
    {
        var id = viewId2ModelId(viewId)
        var prevId;

        if (id >= 0)
        {
            prevId = id-1
            if (prevId < 0)
                prevId = tabSelectorModel.count - 1;

            return tabSelectorModel.get(prevId).viewId
        }
        return
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

}
