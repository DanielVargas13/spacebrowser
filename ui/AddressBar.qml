import QtQuick 2.7
import QtQuick.Controls 2.2
import "."

TextField
{
    id: root
    
    property real loadProgress: 0;
    property bool loadProgressVisible: false;
    
    ProgressComponent
    {
        id: pageLoadProgressBar
        anchors.right: root.right
        anchors.rightMargin: Style.margin
        anchors.verticalCenter: root.verticalCenter

        progress: loadProgress
        text: "Loading page: "

        stateVisible: loadProgressVisible;
    }

    ProgressComponent
    {
        id: downloadProgressBar
        objectName: "downloadProgressBar"
        anchors.right: pageLoadProgressBar.stateVisible ? pageLoadProgressBar.left : root.right
        anchors.rightMargin: Style.margin
        anchors.verticalCenter: root.verticalCenter

        componentBorderColor: Style.downloadProgressComponent.border.color
        componentColor: Style.downloadProgressComponent.color

        progress: 1.0
        text: "Downloading: "
        stateVisible: progress != 1.0
        
        function updateProgress(total)
        {
            downloadProgressBar.progress = total
        }
    }

}