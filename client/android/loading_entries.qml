import QtQuick 2.9
import QtQuick.Window 2.2
import QtQuick.Controls 2.2
import "components" as Components

Column {
    padding: 10
    spacing: 10
    Components.Banner {
        id: banner
    }
    ProgressBar {
        id: loadingProgress
        objectName: "loadingProgress"
        anchors.horizontalCenter: parent.horizontalCenter
        from: 0
        to: 100
        value: 0
    }
    Label {
        anchors.horizontalCenter: parent.horizontalCenter
        text: "Loading device data..."
    }
}

