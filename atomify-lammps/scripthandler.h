#ifndef SCRIPTHANDLER_H
#define SCRIPTHANDLER_H

#include <QMutex>
#include <QObject>
#include <QDebug>
#include <QQueue>
#include <QString>
#include <QPair>
#include "scriptparser.h"
#include "atomstyle.h"
class LAMMPSController;
class AtomifySimulator;

struct CommandInfo {
    enum class Type {NA, File, Editor, SingleCommand, SkipLammpsTick};
    Type type;
    QString filename;
    int line = 0;
    CommandInfo() { type = Type::NA; line = -1; }

    CommandInfo(Type type) { this->type = type; }

    CommandInfo(Type type, int line, QString filename) {
        this->type = type;
        this->line = line;
        this->filename = filename;
    }
};

class ScriptHandler : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString currentCommand READ currentCommand)
    Q_PROPERTY(AtomStyle* atomStyle READ atomStyle WRITE setAtomStyle)
private:
    ScriptParser m_parser;
    QMutex m_mutex;
    QQueue<QPair<QString, CommandInfo> > m_lammpsCommandStack;
    QQueue<QPair<QString, CommandInfo> > m_queuedCommands;
    QPair<QString, CommandInfo> m_currentCommand;
    QVector<QString> m_previousSingleCommands;
    int m_currentPreviousSingleCommand = 0;
    AtomStyle* m_atomStyle = NULL;

public:
    ScriptHandler();
    QPair<QString, CommandInfo> nextCommand();
    void loadScriptFromFile(QString filename);
    QString currentCommand() const;
    AtomStyle* atomStyle() const;
    QQueue<QPair<QString, CommandInfo> > &queuedCommands();
    ScriptParser &parser() { return m_parser; }
    void parseEditorCommand(QString command, AtomifySimulator *mySimulator);
    bool parseLammpsCommand(QString command, LAMMPSController *lammpsController);

public slots:
    void runScript(QString script, CommandInfo::Type type = CommandInfo::Type::Editor, QString filename = "");
    void runCommand(QString command, bool addToPreviousCommands = false);
    void reset();
    QString previousSingleCommand();
    QString nextSingleCommand();
    QString lastSingleCommand();
    void setAtomStyle(AtomStyle* atomStyle);
    void runFile(QString filename);
};

#endif // SCRIPTHANDLER_H
