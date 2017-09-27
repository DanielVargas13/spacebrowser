import QtQuick 2.7

import QtQuick.Controls 2.2

import QtWebEngine 1.5

//import QtQuick.Controls 1.4 as OldControls

//import QtQuick.Dialogs 1.2 // file dialog

import "."

//import org.qt.qtextedit 1.0

Rectangle
{
    id: mainWindow
    visible: true

    color: Style.background

    width: Style.mainWindow.width
    height: Style.mainWindow.height

    TextField
    {
    	id: addressBar
    	
    	height: Style.addressBar.height
    	background: Style.addressBar.style
    	
    	anchors.left: parent.left
    	anchors.right: parent.right
    	
        placeholderText: qsTr("https://")
        inputMethodHints: Qt.ImhUrlCharactersOnly
        
        onAccepted: { // FIXME: move logic to c++
        	if (!text.toLowerCase().startsWith("https://") &&
        			!text.toLowerCase().startsWith("http://"))
        	{
        		text = "https://" + text        		
        	}

        	webViewContainer.setUrl(text)
        }
    }

	Item
	{
		id: tabSelector
		objectName: "tabSelector"
		
		width: Style.tabSelector.width
		
		anchors.top: addressBar.bottom
		anchors.right: parent.right
		anchors.topMargin: Style.margin
		anchors.bottom: parent.bottom
		
		signal viewSelected(int viewId)
		
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
		        color: (tabSelectorView.currentIndex == viewId) ?
		                Style.tabSelector.entry.selected : Style.lightBackground
		                
                onClose: {
                    console.log("close")
                }
		    }
		    
		    MouseArea
		    {
		        id: tabSelectorViewMouseArea
		        anchors.fill: parent
		        propagateComposedEvents: true
	            onClicked:
	            {
	                tabSelectorView.currentIndex = tabSelectorView.indexAt(mouseX, mouseY)
	                var vid = tabSelectorModel.get(tabSelectorView.currentIndex).viewId
	                console.log("vid: " + vid)
	                tabSelector.viewSelected(vid)
//	                webViewContainer.currentView = tabSelectorView.currentIndex
	                mouse.accepted = false
	            }
	        }
		}
		

	}

	Item
	{
		id: webViewContainer
		objectName: "webViewContainer"
		
		width: parent.width - tabSelector.width
		
		anchors.top: addressBar.bottom
		anchors.topMargin: Style.margin
		anchors.left: parent.left
		anchors.right: tabSelector.left
		anchors.bottom: parent.bottom
		
		property list<WebEngineView> views
		//property int currentView: 0
		property WebEngineView currentView2
		
//		Component.onCompleted: {
//		    var view = createNewView()
//		    children.push(view)
//		    currentView = 0
//		}
		
		onCurrentView2Changed: {
		    children = currentView2
		    //currentView2 = views[currentView]
		    console.log("changed")
		}
		
		function createNewView(newViewId)
		{
		    var idx = views.length // FIXME: set proper id
		    tabSelectorModel.append({title:"Empty", icon:"", viewId:newViewId})

	        var view = viewComp.createObject();
		    view.myIndex = idx // FIXME: remove this property
            views.push(view)

            return view
		}
		
		
		function setUrl(url)
		{
		    //webViewContainer.views[currentView].url = url
		    currentView2.url = url
		}
		
		property Component viewComp: WebEngineView
		{
			id: webView
			anchors.fill: parent
			
			property int myIndex: 0
			
			onTitleChanged: {
			    tabSelectorModel.get(myIndex).title = title
			}
			onIconChanged: {
				tabSelectorModel.get(myIndex).icon = icon.toString()
			}
		    onNewViewRequested: function(request) {
		        var viewId = viewHandler.createNewView()
		        request.openIn(viewHandler.getView(viewId));
            }

		}
		
	}

}