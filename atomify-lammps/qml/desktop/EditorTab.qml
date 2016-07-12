import QtQuick 2.7
import QtQuick.Controls 1.5
import QtQuick.Layouts 1.2
import Atomify 1.0
import SimVis 1.0
import "../visualization"
Item {
    id: editorTabRoot
    property TextArea consoleOutput: myConsole.output
    property alias lammpsEditor: myLammpsEditor
    property AtomifySimulator simulator
    property AtomifyVisualizer visualizer

    function reportError() {
        if(simulator.lammpsError) {
            if(simulator.lammpsError.scriptFile === "") {
                consoleOutput.append(" Simulation crashed on line "+simulator.lammpsError.line)
                consoleOutput.append(" Command: '"+simulator.lammpsError.command+"'")
                consoleOutput.append(" Error: '"+simulator.lammpsError.message+"'")
            } else {
                consoleOutput.append(" Simulation crashed.")
                consoleOutput.append(" File: " + simulator.lammpsError.scriptFile + " on line " + simulator.lammpsError.line)
                consoleOutput.append(" Command: '"+simulator.lammpsError.command+"'")
                consoleOutput.append(" Error: '"+simulator.lammpsError.message+"'")
            }
        }
    }

    SplitView {
        orientation: Qt.Vertical
        anchors.fill: parent

        LammpsEditor {
            id: myLammpsEditor
            simulator: editorTabRoot.simulator
            Layout.fillHeight: true
            Layout.fillWidth: true
            Layout.minimumHeight: 200
        }

        Console {
            id: myConsole
            simulator: editorTabRoot.simulator
        }
    }

    Shortcut {
        sequence: "Escape"
        onActivated: {
            console.log("Escape pressed")
            visualizer.focus = true
//            if(myLammpsEditor.textarea.activeFocus || myConsole.textField.activeFocus) {
//                myLammpsEditor.textarea.focus = false
//                myConsole.textField.focus = false
//                visualizer.focus = true
//            } else {
//                editorTabRoot.paused = !editorTabRoot.paused
//            }
        }
    }
}
