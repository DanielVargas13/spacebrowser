import QtQml 2.12
import QtQuick 2.7
import QtQuick.Controls 2.2
import QtQuick.Layouts 1.11

Dialog
{
    id: root
    title: "Configure database connection"
    standardButtons: Dialog.Save | Dialog.Cancel
    closePolicy: Popup.CloseOnEscape
    modal: true

    property var dbData
    property var drivers

    onDbDataChanged: {
        connListModel.clear()
        connListModel.append(dbData)
    }

    function configureDbConnection(dbData, drivers, encReady)
    {
        encCheckBox.enabled = encReady
        root.dbData = dbData
        root.drivers = drivers
        root.clearDialog()
        root.open()
    }

    function reconfigureDbConnection(err)
    {
        contentLayout.showError = true
        errorLabel.text = err

        root.open()
    }

    signal clearDialog()
    signal dbConfigured(var connData)

    onAccepted: {
        var id = connNameCombo.currentIndex
        var item = connListModel.get(id)

        dbConfigured({
            connName: item.connName,
            driverType: item.driverType,
            hostname: item.hostname,
            dbName: item.dbName,
            username: item.username,
            password: item.password,
            isEncrypted: item.isEncrypted,
            schemaName: item.schemaName
        })
    }

    Item {
        width: configureDbConnectionDialog.width * 0.9
        implicitHeight: contentLayout.implicitHeight
        anchors.horizontalCenter: parent.horizontalCenter

        GridLayout {
            id: contentLayout
            columns: 2
            anchors.fill: parent
            property bool showError: false

            ListModel
            {
                id: connListModel
            }
            Label {
                text: "Connection name: "
            }
            ComboBox {
                id: connNameCombo
                editable: true
                Layout.fillWidth: true
                textRole: "connName"
                model: connListModel

                onAccepted: {

                    if (connNameCombo.editText != connNameCombo.currentText)
                    {
                        connListModel.append({connName: connNameCombo.editText,
                                              driverType: driverTypeCombo.currentText,
                                              hostname: hostnameField.text,
                                              dbName: dbNameField.text,
                                              schemaName: schemaNameField.text,
                                              username: usernameField.text,
                                              password: passwordField.text,
                                              isEncrypted: encCheckBox.checked})
                        connNameCombo.currentIndex = connListModel.count - 1
                    }
                }
                onActivated: {
                    var currentItem = connListModel.get(connNameCombo.currentText)
                    var id = connNameCombo.currentIndex

                    driverTypeCombo.currentIndex = driverTypeCombo.find(
                        connListModel.get(id).driverType)

                    if (currentItem.hostname)
                        hostnameField.text = connListModel.get(id).hostname
                    else
                        hostnameField.text = ""

                    if (currentItem.dbName)
                        dbNameField.text = connListModel.get(id).dbName
                    else
                        dbNameField.text = ""

                    if (currentItem.schemaName)
                        schemaNameField.text = connListModel.get(id).schemaName
                    else
                        schemaNameField.text = ""

                    if (currentItem.username)
                        usernameField.text = connListModel.get(id).username
                    else
                        usernameField.text = ""

                    if (currentItem.password)
                        passwordField.text = connListModel.get(id).password
                    else
                        passwordField.text = ""

                    encCheckBox.checked = connListModel.get(id).isEncrypted
                }
            }

            Label {
                text: "Database driver: "
            }
            ComboBox {
                id: driverTypeCombo
                Layout.fillWidth: true
                model: configureDbConnectionDialog.drivers
                enabled: connListModel.count != 0
                onActivated: {
                    connListModel.get(connNameCombo.currentIndex).driverType
                        = driverTypeCombo.currentText
                }
                Connections
                {
                    target: root
                    onClearDialog:
                    {
                        driverTypeCombo.currentIndex = -1
                    }
                }
            }

            Label {
                text: "Hostname:"
            }
            TextField {
                id: hostnameField
                Layout.fillWidth: true
                enabled: connListModel.count != 0
                onEditingFinished: {
                    if (hostnameField.text) {
                        connListModel.get(connNameCombo.currentIndex).hostname
                            = hostnameField.text
                    }
                }
                Connections
                {
                    target: root
                    onClearDialog:
                    {
                        hostnameField.text = ""
                    }
                }
            }

            Label {
                text: "Database name:"
            }
            TextField {
                id: dbNameField
                Layout.fillWidth: true
                enabled: connListModel.count != 0
                onEditingFinished: {
                    if (dbNameField.text) {
                        connListModel.get(connNameCombo.currentIndex).dbName
                            = dbNameField.text
                    }
                }
                Connections
                {
                    target: root
                    onClearDialog:
                    {
                        dbNameField.text = ""
                    }
                }
            }

            Label {
                text: "Schema name:"
            }
            TextField {
                id: schemaNameField
                Layout.fillWidth: true
                enabled: connListModel.count != 0
                onEditingFinished: {
                    if (schemaNameField.text) {
                        connListModel.get(connNameCombo.currentIndex).schemaName
                            = schemaNameField.text
                    }
                }
                Connections
                {
                    target: root
                    onClearDialog:
                    {
                        schemaNameField.text = ""
                    }
                }
            }

            Label {
                text: "Username:"
            }
            TextField {
                id: usernameField
                Layout.fillWidth: true
                enabled: connListModel.count != 0
                onEditingFinished: {
                    if (usernameField.text) {
                        connListModel.get(connNameCombo.currentIndex).username
                            = usernameField.text
                    }
                }
                Connections
                {
                    target: root
                    onClearDialog:
                    {
                        usernameField.text = ""
                    }
                }
            }

            Label {
                text: "Password:"
            }
            TextField {
                id: passwordField
                Layout.fillWidth: true
                echoMode: TextInput.Password
                enabled: connListModel.count != 0
                onEditingFinished: {
                    if (passwordField.text) {
                        connListModel.get(connNameCombo.currentIndex).password
                            = passwordField.text
                    }
                }
                Connections
                {
                    target: root
                    onClearDialog:
                    {
                        passwordField.text = ""
                    }
                }
            }

            Label {
                text: "Encrypt password:"
            }
            CheckBox {
                id: encCheckBox
                checked: false
                onCheckStateChanged: {
                    connListModel.get(connNameCombo.currentIndex).isEncrypted
                        = checked
                }
                Connections
                {
                    target: root
                    onClearDialog:
                    {
                        if (encCheckBox.enabled)
                            checked = false
                    }
                }
            }

            Label {
                visible: contentLayout.showError
                text: "Error:"
            }
            Label {
                id: errorLabel
                visible: contentLayout.showError
                wrapMode: Text.Wrap
                Layout.fillWidth: true

                Connections
                {
                    target: root
                    onClearDialog:
                    {
                        errorLabel.text = false
                    }
                }
            }
        }
    }
}
