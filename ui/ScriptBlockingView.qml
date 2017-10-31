import QtQuick 2.7
import QtQuick.Controls 2.2
import "."

Rectangle
{
    id: root
    
    color: Style.background

    property string targetUrl

    signal whitelistLocal(string site, string url);
    signal whitelistGlobal(string url);
    signal removeLocal(string site, string url);
    signal removeGlobal(string url);
    
    ListModel
    {
        id: scriptBlockListModel
        objectName: "scriptBlockListModel"
    }

    Item
    {
        id: header
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.leftMargin: Style.scriptBlockingView.margin
        anchors.right: parent.right
        anchors.rightMargin: Style.scriptBlockingView.margin
        
        height: Style.scriptBlockingView.entry.height
        
        Rectangle
        {
            id: col1
            color: Style.scriptBlockingView.headerBackground
            
            anchors.left: parent.left
            anchors.right: col2.left
            anchors.rightMargin: Style.margin
            anchors.top: parent.top
            anchors.bottom: parent.bottom
            
            Text
            {
                anchors.verticalCenter: parent.verticalCenter
                anchors.left: parent.left
                anchors.leftMargin: Style.margin
                text: "Source url"
                font.bold: true
            }
        }
        Rectangle
        {
            id: col2
            color: Style.scriptBlockingView.headerBackground
            
            anchors.right: col3.left
            anchors.rightMargin: Style.margin
            anchors.top: parent.top
            anchors.bottom: parent.bottom
            
            width: Style.scriptBlockingView.colWidth
            
            Text
            {
                anchors.verticalCenter: parent.verticalCenter
                anchors.horizontalCenter: parent.horizontalCenter
                text: "Allow"
                font.bold: true
            }
        }
        Rectangle
        {
            id: col3
            color: Style.scriptBlockingView.headerBackground
            
            anchors.right: parent.right
            anchors.top: parent.top
            anchors.bottom: parent.bottom
            
            width: Style.scriptBlockingView.colWidth
            
            Text
            {
                anchors.verticalCenter: parent.verticalCenter
                anchors.horizontalCenter: parent.horizontalCenter
                text: "Global Allow"
                font.bold: true
            }
        }
    }
    
    ScrollView
    {
        id: scrollView
        clip: true
        
        anchors.top: header.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: okButton.top
        anchors.bottomMargin: Style.margin

        ListView
        {
            id: scriptBlockListView
            anchors.fill: parent
            
            model: scriptBlockListModel
            delegate: ScriptBlockListEntry
            {
                url: model.url
                allowed: model.allowed
                gallowed: model.gallowed
                myId: model.myId
                
                onAllowedChanged: {
                    if (allowed)
                        whitelistLocal(targetUrl, url)
                    else
                        removeLocal(targetUrl, url)
                }
                onGallowedChanged: {
                    if (gallowed)
                        whitelistGlobal(url)
                    else
                        removeGlobal(url)
                }
            }
        }

    }

    Button
    {
        id: okButton

        text: "Ok"
        width: Style.buttonWidth
        height: Style.buttonHeight
        
        background: Rectangle {
            color: mouseAreaButton.containsMouse ? Style.contextMenu.entry.selected : Style.contextMenu.background.color
        }

        anchors.bottom: parent.bottom
        anchors.horizontalCenter: parent.horizontalCenter
            
        onClicked: root.visible = false
        
        MouseArea
        {
            id: mouseAreaButton
            anchors.fill: parent
            hoverEnabled: true
            onClicked: okButton.clicked()
        }
    }

    function clearEntries()
    {
        scriptBlockListModel.clear()
    }
    function addEntry(entry)
    {
        entry["myId"] = scriptBlockListModel.count
        scriptBlockListModel.append(entry)
        console.log(scriptBlockListModel.count)
    }
}