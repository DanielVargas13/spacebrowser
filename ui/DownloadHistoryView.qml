import QtQuick 2.7
import QtQuick.Controls 2.2
import "."

Rectangle
{
    id: root

    signal openUrl(string url)
    signal pause(int id)
    signal resume(int id)
    signal cancel(int id)

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
                paused: model.paused
                canceled: model.canceled
                finished: model.finished

                onOpenUrl: root.openUrl(url)
                onPause:  root.pause(id)
                onResume: root.resume(id)
                onCancel: root.cancel(id)
            }
        }
    }

    function addEntry(entry)
    {
        downloadHistoryListModel.append(entry)
    }
    
    function updateProgress(id, received, total)
    {
        downloadHistoryListModel.get(id-1).received = received
        downloadHistoryListModel.get(id-1).total = total
    }
    
    function downloadFinished(id)
    {
        downloadHistoryListModel.get(id-1).finished = true
    }
    function downloadPaused(id)
    {
        downloadHistoryListModel.get(id).paused = true
    }
    function downloadResumed(id)
    {
        downloadHistoryListModel.get(id).paused = false
    }
    function downloadCanceled(id)
    {
        downloadHistoryListModel.get(id).canceled = true
    }

}