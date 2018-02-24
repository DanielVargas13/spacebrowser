pragma Singleton
import QtQuick 2.7

QtObject
{
    id: general
    
    property color background: "#444444"
    property color lightBackground: "#777777"
    property color border: "#222222"

    property int margin: 5
    property int buttonWidth: 120  // Context menu buttons
    property int buttonHeight: 40

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

    property QtObject button: QtObject
    {
        property int size: 25
        property int radius: 4
        property color background: lightBackground
        
        property QtObject border: QtObject
        {
            property color color: general.border
            property int width: 2
        }
    }

    property QtObject findBar: QtObject
    {
        property int height: 35
        property int textHeight: 25

        property real opacity: 0.7
        property color background: "transparent"
        property color color: "#D8C000" //"#81018B"
        property QtObject border: QtObject
        {
            property color color: general.border
            property int width: 2
        }
        property int radius: 8

        property Item textStyle: Rectangle
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
            property color background: general.lightBackground
            property color selected: "#999999"

            property QtObject icon: QtObject
            {
                property int width: 22
                property int height: 22
            }


            property QtObject border: QtObject
            {
                property int width: 1
                property color color: "#888888"
            }
        }
    }

    property QtObject closeIcon: QtObject
    {
        property int width: 15
        property int height: 15
    }

    property QtObject contextMenu: QtObject
    {
        property int width: 200
        
        property color borderColor: general.border
        property int borderWidth: 2
        
        property Rectangle background: Rectangle
        {
            color: general.lightBackground
            border.color: general.contextMenu.borderColor
            border.width: general.contextMenu.borderWidth
            implicitWidth: general.contextMenu.width
            implicitHeight: general.contextMenu.entry.height
        }
        
        property QtObject entry: QtObject
        {
            property int height: 25
            property color selected: general.tabSelector.entry.selected
        }
    }

    property QtObject scriptBlockingView: QtObject
    {
        property int margin: 20
        property int colWidth: 100
        property color headerBackground: "#666666"

        property QtObject entry: QtObject
        {
            property int height: 30
            property color lightBackground: "#888888"

            property QtObject button: QtObject
            {
                property int size: 20
                property color blocked: "brown"
                property color allowed: "green"
            }
        }
    }

    property QtObject progressComponent: QtObject
    {
        property int width: 200
        property int height: 21
        property real opacity: 0.6
        property color background: "transparent"
        property color color: "#A3FF3C"
        property int radius: 8

        property QtObject border: QtObject
        {
            property int width: 1
            property color color: progressComponent.color
        }
    }

    property QtObject downloadProgressComponent: QtObject
    {
        property color color: "#49D2FF"

        property QtObject border: QtObject
        {
            property color color: downloadProgressComponent.color
        }
    }
}
