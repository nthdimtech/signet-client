import QtQuick 2.9
import QtQuick.Window 2.2
import QtQuick.Controls 2.2
import "components" as Components

Column {
    anchors.horizontalCenter: parent.horizontalCenter
    Components.Banner {
        id: banner
    }
    ProgressBar {
        anchors.horizontalCenter: parent.horizontalCenter
        from: 1
        to: 100
        value: 25
    }
    Label {
        anchors.horizontalCenter: parent.horizontalCenter
        text: "Loading device data..."
    }
}

