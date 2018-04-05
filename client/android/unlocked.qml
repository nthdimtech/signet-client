import QtQuick 2.9
import QtQuick.Window 2.3
import QtQuick.Controls 2.3

Item {
    id: unlockedItem
    Item {
        id: filterRow
        anchors.top: parent.top
        height: childrenRect.height
        width: parent.width
        Image {
            id: searchImg
            height: filterText.height * 1.2
            width: filterText.height * 1.2 * (sourceSize.width/sourceSize.height)
            source: "../images/search.png"
        }
        TextEdit {
            id: filterText
            padding: 8
            anchors.left: searchImg.right
            anchors.right: parent.right
            anchors.bottom: searchImg.bottom
            text: ""
        }
    }

    Item {
        id: groupRow
        anchors.top: filterRow.bottom
        width: parent.width
        height: childrenRect.height + 10
        anchors.bottomMargin: 10
        anchors.topMargin: 10
        Text {
            id: groupText
            anchors.verticalCenter: groupCombo.verticalCenter
            anchors.left: parent.left
            anchors.margins: 10
            text: "Group"
        }
        ComboBox {
            anchors.top: parent.top
            anchors.left: groupText.right
            anchors.right: parent.right
            anchors.margins: 10
            id: groupCombo
            model: groupModel
            Component {
                id: groupDelegate
                Text { text: display }
            }
            delegate: groupDelegate
        }
    }

    Rectangle {
        id: entryView
        border.width: 2
        anchors.top: groupRow.bottom
        anchors.bottom: parent.bottom
        width: parent.width - 20
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.margins: 10
        ListView {
            anchors.fill: parent
            anchors.margins: 10
            model: entryModel
            Component {
                id: entryDelegate
                Text { text: display }
            }
            delegate: entryDelegate
        }
    }
}
