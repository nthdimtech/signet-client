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
    Label {
        anchors.horizontalCenter: parent.horizontalCenter
        padding: 10
        text: "No device connected. Please connect device."
    }
}
