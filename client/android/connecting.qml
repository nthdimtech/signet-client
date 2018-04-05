import QtQuick 2.9
import QtQuick.Window 2.2
import QtQuick.Controls 2.2
import "components" as Components

Column {
    Components.Banner {
        id: banner
    }
    Label {
        padding: 10
        anchors.horizontalCenter: parent.horizontalCenter
        text: "Trying to connect to device..."
    }
}
