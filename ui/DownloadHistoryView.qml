import QtQuick 2.7
import QtQuick.Controls 2.2
import "."

Rectangle
{
    id: root

    signal openUrl(string url)

    color: Style.background

    ListModel
    {
        id: downloadHistoryListModel
    }

    ScrollView
    {
        id: scrollView
        clip: true

        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        anchors.bottomMargin: Style.margin
        
        ListView
        {
            id: downloadHistoryListView
            anchors.fill: parent
            
            model: downloadHistoryListModel
            header: DownloadHistoryListEntry
            {
                header: true
                url: "Source Url Path"
                path: "Downloaded File Path"
            }
            delegate: DownloadHistoryListEntry
            {
                url: model.url
                path: model.path
                received: model.received
                total: model.total
                myId: model.myId
                
                onOpenUrl: {
                    root.openUrl(url)
                }
            }
        }
    }
    
    function addEntry(entry)
    {
        downloadHistoryListModel.append(entry)
    }
    
    function updateProgress(id, received, total)
    {
        console.log(id)
        console.log(received)
        console.log(total) // FIXME: -1 is wrong, it should be properly mapped in cpp
        console.log(downloadHistoryListModel.get(id-1).received)
        downloadHistoryListModel.get(id-1).received = received
        downloadHistoryListModel.get(id-1).total = total
    }

}