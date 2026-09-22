import QtQuick 2.12
Item {
    id: root
    width: 800
    height: 480

    // 控制繁重程度，数值越大加载越慢
    property int loadWeight: 8000

    // 批量生成子节点，增加实例化开销
    Repeater {
        model: root.loadWeight
        Rectangle {
            x: index % 40 *20
            y: Math.floor(index/40)*20
            width:18
            height:18
            color: Qt.rgba(Math.random(),Math.random(),Math.random(),0.6)
            border.color:"white"
            border.width:1
        }
    }
}
