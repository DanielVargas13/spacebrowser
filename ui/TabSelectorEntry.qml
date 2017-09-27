import QtQuick 2.7
import "."

Rectangle
{
	id: root
	color: Style.lightBackground
	property string title: ""
	property string icon: ""
    property int viewId: 0
	
	height: Style.tabSelector.entry.height
	
	//anchors.top: parent.top
	anchors.left: parent.left
	anchors.right: parent.right
	anchors.leftMargin: Style.margin
	anchors.rightMargin: Style.margin
	
	signal close()

	Image
	{
		id: tabIcon
	    sourceSize: Qt.size(Style.tabSelector.entry.icon.width, Style.tabSelector.entry.icon.height)
	    source: icon
	    
	    anchors.left: parent.left
	    anchors.leftMargin: Style.margin
	    anchors.verticalCenter: parent.verticalCenter
	}
	
	Text
	{
	    id: tabTitle
		anchors.left: tabIcon.right
		anchors.leftMargin: Style.margin
		anchors.verticalCenter: parent.verticalCenter
		anchors.right: closeIcon.left
		anchors.rightMargin: Style.margin
		
		elide: Text.ElideRight
		text: title
	}
	
	Rectangle
	{
	    id: closeIcon
	    width: Style.tabSelector.entry.icon.width
	    height: Style.tabSelector.entry.icon.height
	    
	    color:"brown"
	    
	    anchors.verticalCenter: parent.verticalCenter
	    anchors.right: parent.right
	    anchors.rightMargin: Style.margin
	    
	    MouseArea
	    {
            anchors.fill: closeIcon
            onClicked:
            {
                close()
            } 
	    }
	}
}