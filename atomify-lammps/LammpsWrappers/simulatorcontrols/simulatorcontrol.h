#ifndef SIMULATORCONTROL_H
#define SIMULATORCONTROL_H
#include <Qt3DCore>
#include <QQuickItem>
#include <QVariantList>

class LAMMPSController;
class AtomifySimulator;
class SimulatorControl : public Qt3DCore::QNode
{
    Q_OBJECT
    Q_PROPERTY(bool enabled READ enabled WRITE setEnabled NOTIFY enabledChanged)
    Q_PROPERTY(QString identifier READ identifier WRITE setIdentifier NOTIFY identifierChanged)
    Q_PROPERTY(QString command READ command WRITE setCommand NOTIFY commandChanged)
    Q_PROPERTY(QVariantList dependencies READ dependencies WRITE setDependencies NOTIFY dependenciesChanged)

protected:
    QString m_command;
    QString m_identifier;
    QString m_identifierPrefix = "atomify_";
    QVariantList m_dependencies;
    bool m_enabled = true;
    bool addToLammps(LAMMPSController *lammpsController);
    virtual void updateCommand() = 0;
    virtual QList<QString> enabledCommands() = 0;
    virtual QList<QString> disableCommands() = 0;
    virtual QList<QString> resetCommands() = 0;
    virtual QString createCommandPrefix() = 0;
    void addDependency(SimulatorControl *control);
    void removeDependency(SimulatorControl *control);
public:
    explicit SimulatorControl(Qt3DCore::QNode *parent = 0);
    static int getNextId();
    ~SimulatorControl();
    bool enabled() const;
    QString identifier() const;
    QString command() const;
    QVariantList dependencies() const;
    bool dependenciesValid(LAMMPSController *lammpsController);
    virtual bool existsInLammps(LAMMPSController *lammpsController) = 0;
    virtual void update(LAMMPSController *lammpsController);
    virtual void handleCommand(QString command) { /* TODO */ }
    virtual QString fullCommand();

signals:
    void enabledChanged(bool enabled);
    void identifierChanged(QString identifier);
    void commandChanged(QString command);
    void dependenciesChanged(QVariantList dependencies);

public slots:
    void setEnabled(bool enabled);
    void setIdentifier(QString identifier);
    void setCommand(QString command);
    void setDependencies(QVariantList dependencies);
};

#endif // SIMULATORCONTROL_H
