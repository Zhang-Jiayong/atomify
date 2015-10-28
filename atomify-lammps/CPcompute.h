#ifndef COMPUTE_H
#define COMPUTE_H

#include <QObject>
#include <QVector>
#include "lammpsoutput.h"

class AtomifySimulator; class LAMMPSController;
class CPCompute : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString identifier READ identifier WRITE setIdentifier NOTIFY identifierChanged)
    Q_PROPERTY(QString command READ command WRITE setCommand NOTIFY commandChanged)
    Q_PROPERTY(AtomifySimulator* simulator READ simulator WRITE setSimulator NOTIFY simulatorChanged)
    Q_PROPERTY(QStringList dependencies READ dependencies WRITE setDependencies NOTIFY dependenciesChanged)
    Q_PROPERTY(bool isVector READ isVector WRITE setIsVector NOTIFY isVectorChanged)
    Q_PROPERTY(double time READ time)
    Q_PROPERTY(QList<double> values READ values NOTIFY valuesChanged)
    Q_PROPERTY(double value READ firstValue NOTIFY firstValueChanged)
    Q_PROPERTY(double firstValue READ firstValue NOTIFY firstValueChanged)
    Q_PROPERTY(double secondValue READ secondValue NOTIFY secondValueChanged)
    Q_PROPERTY(double thirdValue READ thirdValue NOTIFY thirdValueChanged)
    Q_PROPERTY(double fourthValue READ fourthValue NOTIFY fourthValueChanged)
private:
    QString m_identifier;
    QString m_fixIdentifier;
    QString m_fixCommand;
    QString m_command;

    AtomifySimulator* m_simulator = nullptr;
    QStringList m_dependencies;
    QList<double> m_values;
    double m_time = 0;
    bool m_isVector = false;

public:
    explicit CPCompute(QObject *parent = 0);
    ~CPCompute();

    QString identifier() const;
    QString command() const;
    AtomifySimulator* simulator() const;
    QStringList dependencies() const;
    double firstValue() const;
    double secondValue() const;
    double thirdValue() const;
    double fourthValue() const;
    double time() const;
    QList<double> values() const;
    LammpsOutput &output();
    bool isVector() const;
    void update(LAMMPSController *lammpsController);

signals:
    void identifierChanged(QString identifier);
    void commandChanged(QString command);
    void simulatorChanged(AtomifySimulator* simulator);
    void dependenciesChanged(QStringList dependencies);
    void firstValueChanged(double value);
    void secondValueChanged(double value);
    void thirdValueChanged(double value);
    void fourthValueChanged(double value);
    void valuesChanged(QList<double> values);
    void isVectorChanged(bool isVector);

public slots:
    void setIdentifier(QString identifier);
    void setCommand(QString command);
    void setSimulator(AtomifySimulator* simulator);
    void setDependencies(QStringList dependencies);
    void setValues(QVector<double> values);
    void setIsVector(bool isVector);
};
#endif // COMPUTE_H
