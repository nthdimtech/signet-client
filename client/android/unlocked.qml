import QtQuick 2.9
import QtQuick.Window 2.3
import QtQuick.Controls 2.3

Item {
    id: unlockedItem
    objectName: "unlockedItem"

    signal lockSignal();
    signal filterTextChangedSignal(string text);
    signal filterGroupChangedSignal(int index);
    Item {
        id: filterRow
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right
        height: childrenRect.height
        anchors.topMargin: 10
        Image {
            id: searchImg
            anchors.left: parent.left
            anchors.leftMargin: 10
            anchors.verticalCenter: filterRect.verticalCenter
            height: filterRect.height * 1.2
            width: filterRect.height * 1.2 * (sourceSize.width/sourceSize.height)
            source: "../images/search.png"
        }
        Rectangle {
            id: filterRect
            anchors.left: searchImg.right
            anchors.leftMargin: 10
            anchors.right: searchButton.left
            anchors.rightMargin: 10
            anchors.verticalCenter: searchButton.verticalCenter
            height: childrenRect.height
            border.width: 2
            TextInput {
                anchors.left: parent.left
                width: parent.width
                id: filterText
                inputMethodHints: Qt.ImhNoPredictiveText
                padding: 4
                text: ""
                focus: true
                onTextChanged: {
                    console.debug("edited:" + filterText.text)
                    filterTextChangedSignal(filterText.text)
                }
            }
        }
        Button {
            id: searchButton
            anchors.right: parent.right
            anchors.rightMargin: 10
            anchors.leftMargin: 10
            text: "Clear"
            onClicked: {
                filterText.clear()
            }
        }
    }

    Item {
        id: groupRow
        anchors.top: filterRow.bottom
        anchors.topMargin: 10
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottomMargin: 10
        implicitHeight: Math.max(groupText.height, groupComboBox.height)
        Text {
            id: groupText
            anchors.left: parent.left
            anchors.margins: 10
            anchors.verticalCenter: parent.verticalCenter
            text: "Group"
        }
        ComboBox {
            id: groupComboBox
            anchors.left: groupText.right
            anchors.right: parent.right
            anchors.margins: 10
            anchors.verticalCenter: parent.verticalCenter
            textRole: "display"

            model: groupModel
            Component {
                id: groupDelegate
                Text {
                    text: display
                    MouseArea {
                        anchors.fill: parent
                        onClicked: {
                            groupComboBox.currentIndex = index
                            groupComboBox.popup.close()
                        }
                    }
                }
            }

            delegate: groupDelegate
            onCurrentIndexChanged: {
                console.debug(currentText)
                filterGroupChangedSignal(currentIndex);
            }
            focus: true

        }
    }

    Rectangle {
        id: entryView
        border.width: 2
        anchors.top: groupRow.bottom
        anchors.bottom: lockButton.top
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.margins: 10
        clip: true
        ListView {
            id: entryListView
            anchors.fill: parent
            anchors.margins: 10
            model: entryModel
            Component {
                id: entryDelegate
                Text {
                    text: display
                    MouseArea {
                        anchors.fill: parent
                        onClicked: entryListView.currentIndex = index
                    }
                }
            }
            delegate: entryDelegate
            highlight: Rectangle {
                color: "lightsteelblue"; radius: 3
            }
            focus: true
            highlightFollowsCurrentItem: true
        }
    }

    Button {
        id: lockButton
        text: "Lock"
        anchors.margins: 10
        anchors.bottom: parent.bottom
        anchors.horizontalCenter: parent.horizontalCenter
        onClicked: {
            lockSignal();
        }
    }
}
