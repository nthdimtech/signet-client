import QtQuick 2.9
import QtQuick.Window 2.2
import QtQuick.Controls 2.2
import "components" as Components

Item {
    id: loginComponent
    objectName: "loginComponent"
    signal loginSignal(string msg)

    Components.Banner {
        id: banner
    }

    Rectangle {
        id: controls
        anchors.top: banner.bottom
        width: childrenRect.width
        height: childrenRect.height
        anchors.horizontalCenter: parent.horizontalCenter
        Row {
            spacing: 10
            Label {
                id: passwordText
                text: "Password"
                padding: 3
                anchors.verticalCenter: parent.verticalCenter
            }
            Rectangle {
                anchors.verticalCenter: parent.verticalCenter
                width: childrenRect.width + 6
                height: childrenRect.height
                border.width: 1
                border.color: "grey"
                TextInput {
                    id: password
                    padding: 3
                    width: parent.parent.width/2
                    focus: true
                    echoMode: TextInput.Password
                    onTextChanged: {
                        badpasswordWarning.visible = false;
                    }
                }
            }
            Button {
                 anchors.verticalCenter: parent.verticalCenter
                 id: unlock
                 text: "Unlock"
                 onClicked: {
                    loginSignal(password.text)
                 }
            }
        }
    }
    Label {
        id: badpasswordWarning
        text: "Incorrect password, try again."
        objectName: "badpasswordWarning"
        anchors.top: controls.bottom
        anchors.left: password.left
        visible: false
        color: red
    }
}
