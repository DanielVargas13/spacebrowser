pragma Singleton
import QtQuick 2.7

QtObject
{
	id: general
	
    property color background: "#444444"
    property color lightBackground: "#777777"
	property color border: "#222222"
	
	property int margin: 5
		
	property QtObject mainWindow: QtObject
	{
		property int width: 1280
		property int height: 768
	}

	property QtObject addressBar: QtObject
	{
		property int height: 25
		
		property Item style: Rectangle
		{
			color: lightBackground
			border.color: general.border
	        border.width: 2
	        radius: 4
		}
	}
		
	property QtObject tabSelector: QtObject
	{
		property int width: 250
		
		property QtObject entry: QtObject
		{
			property int height: 30
			property color selected: "#999999"
			
			property QtObject icon: QtObject
			{
				property int width: 22
				property int height: 22
			}
		}
		
	}
	
}