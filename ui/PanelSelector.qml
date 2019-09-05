import QtQuick.Controls 2.2
import QtQuick 2.12

ScrollView
{
    id: root
    clip: true

    signal panelSelected(string dbName)

    ListView
    {
        id: panelList

        anchors.fill: parent
        spacing: 2

        orientation: ListView.Horizontal
        layoutDirection: Qt.LeftToRight

        delegate: Rectangle
        {
            color: ListView.isCurrentItem ? Style.lightBackground : Style.background
            height: Style.panelSelector.height
            width: Style.panelSelector.entry.width

            Item
            {
                anchors.fill: parent
                Image
                {
                    anchors.top: parent.top
                    anchors.topMargin: Style.margin
                    anchors.bottom: buttonText.top
                    anchors.horizontalCenter: parent.horizontalCenter

                    fillMode: Image.PreserveAspectFit
                    source: model.toolTip
                }
                Text
                {
                    id: buttonText
                    height: parent.height / 4
                    font.pixelSize: height
                    anchors.bottom: parent.bottom
                    anchors.bottomMargin: Style.margin
                    anchors.horizontalCenter: parent.horizontalCenter
                    text: model.display
                }
            }

            MouseArea
            {
                anchors.fill: parent

                onClicked:
                {
                    if (index < model.rowCount - 1)
                        panelList.currentIndex = index
                    panelSelected(model.display)
                }
            }


        }
    }

    function setModel(model)
    {
        panelList.model = model
    }

    function setCurrentPanel(dbName)
    {
        var i;
        for (i = 0; i < panelList.model.rowCount(); ++i)
        {
            var index = panelList.model.index(i, 0)
            var data = panelList.model.data(index)

            if (data == dbName)
                panelList.currentIndex = i
        }
    }

    function getCurrentPanel()
    {
        var index = panelList.model.index(panelList.currentIndex, 0)
        var data = panelList.model.data(index)

        return data
    }
}
