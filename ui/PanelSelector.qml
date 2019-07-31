import QtQuick.Controls 2.2
import QtQuick 2.12

ScrollView
{
    id: root
    clip: true

    signal panelSelected(var panel)

    ListView
    {
        id: panelList

        anchors.fill: parent
        spacing: 2

        orientation: ListView.Horizontal
        layoutDirection: Qt.LeftToRight

        delegate: Rectangle
        {
            color: ListView.isCurrentItem ?
                Style.tabSelector.entry.selected : Style.lightBackground

            Image
            {
                source: model.icon
            }

//            onSelected:
//            {
//                panelSelected() // set tabSelector in TabSelectorPanel
//            }
        }
    }

    function setModel(model)
    {
        panelList.model = model
    }
}
