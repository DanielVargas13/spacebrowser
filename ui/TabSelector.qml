import QtQuick 2.7
import QtQuick.Controls 2.2

Item
{
    id: root
    
    signal viewSelected(int viewId)           // connected in c++ code
    signal closeTab(int viewId)               // connected in c++ code
    signal openScriptBlockingView(int viewId) // connected in c++ code
    
    height: Style.tabSelector.entry.height * tabSelectorModel.count

    ListModel
    {
        id: tabSelectorModel
    }
    
    ListView
    {
        id: tabSelectorView

        anchors.fill: parent
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
                tabSelectorView.currentIndex = index
                root.viewSelected(viewId)
            }
        }
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
            console.log("context clicked")
            console.log(mouse.button)
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
    
    function fixIndentation(viewId, _indent)
    {
        var modelId = viewId2ModelId(viewId)
        //console.log("indent: " + _indent + " viewid: " + viewId + " modelId: " + modelId)
        //console.log("length: " + tabSelectorModel.count)
        //console.log("ind: " + tabSelectorModel.get(modelId).indent + " viewId: " + tabSelectorModel.get(modelId).viewId)
        //tabSelectorModel.set(modelId, {indent: _indent})
        tabSelectorModel.get(modelId).indent = _indent // FIXME: coredumps sometimes
        //console.log("~~~ not dumped")
    }
    
    function updateTitle(viewId, title)
    {
        var id = viewId2ModelId(viewId)
        if (id >= 0)
            tabSelectorModel.get(id).title = title
    }
    
    function selectView(viewId)
    {
        var id = viewId2ModelId(viewId)
        if (id < 0)
        {
            if (tabSelectorModel.count < 1)
                return -1;
            
            id = tabSelectorModel.count-1
        }

        tabSelectorView.currentIndex = id
        return tabSelectorModel.get(id).viewId
    }
    
    function updateIcon(viewId, icon)
    {
        var id = viewId2ModelId(viewId)
        if (id >= 0)
            tabSelectorModel.get(id).icon = icon
    }
    
    function removeTabEntry(viewId)
    {
        for (var i=0; i < tabSelectorModel.count; ++i)
        {
            if (tabSelectorModel.get(i).viewId != viewId)
                continue
            
            tabSelectorModel.remove(i)
            
            /// if currently visible tab is closed, select next tab before it in the model
            if (tabSelectorView.currentIndex == i && tabSelectorModel.count > 0)
            {
                var j = 0
                if (i > 0)
                    j = i - 1

                tabSelectorView.currentIndex = j
                viewSelected(tabSelectorModel.get(j).viewId)
            }

            //dumpCurrentModel()//logging
            return
        }
    }
    
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
}
