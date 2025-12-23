import QtQuick 2.9
import QtQuick.Window 2.2
import QtQuick.Controls 2.9

ApplicationWindow {
    visible: true
    width: 1024
    height: 768
    color: "black"
    title: qsTr("DmvSimulator")
   
    Rectangle {
        id: trafficLight
        width: 150
        height: 150
        radius: 180
        color: "red"
        border.color: "grey"
        border.width: 2
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.top: parent.top
        anchors.topMargin: 15
        
        // Slot for C++ signal
        Connections {
            target: worker
            function onNewTrafficLightSymbol(color) {
                trafficLight.color = color;
            }
        }
    }

    Rectangle {
        id: infoDisplay
        width: 1000
        height: 500
        color: "black"
        border.color: "grey"
        border.width: 2
        radius: 10
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.top: parent.top
        anchors.topMargin: 200

        Text {
            id: infoDisplayText
            width: parent.width
            height: parent.height
            font.bold: false
            //font.wordSpacing: 1
            font.pointSize : 110
            lineHeight: 0.56 // This is the spacing between lines
            wrapMode: Text.WordWrap
            horizontalAlignment: Text.AlignLeft
            verticalAlignment : Text.AlignTop
            color: "red" 
            text: ""
    
            // Slot for C++ signal
            Connections {
                target: worker
                function onNewDisplayMessage(message) {
                    infoDisplayText.text = message;
                }
            }
        }
    }
  
}