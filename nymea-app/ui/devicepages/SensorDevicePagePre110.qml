import QtQuick 2.5
import QtQuick.Controls 2.1
import QtQuick.Layouts 1.1
import Nymea 1.0
import "../components"
import "../customviews"

ListView {
    anchors { fill: parent }

    property var device
    property var deviceClass

    model: ListModel {
        Component.onCompleted: {
            var supportedInterfaces = ["temperaturesensor", "humiditysensor", "pressuresensor", "moisturesensor", "lightsensor", "conductivitysensor", "noisesensor", "co2sensor"]
            for (var i = 0; i < supportedInterfaces.length; i++) {
                print("checking", root.deviceClass.name, root.deviceClass.interfaces)
                if (root.deviceClass.interfaces.indexOf(supportedInterfaces[i]) >= 0) {
                    append({name: supportedInterfaces[i]});
                }
            }
        }
    }
    delegate: SensorView {
        width: parent.width
        interfaceName: modelData
        device: root.device
        deviceClass: root.deviceClass
    }
}