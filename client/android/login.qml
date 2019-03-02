import QtQuick 2.9
import QtQuick.Window 2.3
import QtQuick.Controls 2.3
import "components" as Components

Item {
    id: loginComponent
    objectName: "loginComponent"
    signal loginSignal(string msg)

    function badPasswordEntered()
    {
        password.text = ""
        badpasswordWarning.visible = true
    }

    Components.Banner {
        anchors.top: parent.top
        anchors.topMargin: 10
        id: banner
    }

    Rectangle {
        id: controls
        anchors.top: banner.bottom
        anchors.topMargin: 10
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
                    focus: false
                    echoMode: TextInput.Password
                    inputMethodHints: Qt.ImhNoPredictiveText | Qt.ImhSensitiveData
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
        anchors.left: controls.left
        visible: false
    }
}
