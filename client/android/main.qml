import QtQuick 2.9
import QtQuick.Window 2.2
import QtQuick.Controls 2.2

Window {
    visible: true
    title: qsTr("Signet")
    Loader {
        id: mainLoader
        objectName: "mainLoader"
        anchors.fill: parent
    }
}
