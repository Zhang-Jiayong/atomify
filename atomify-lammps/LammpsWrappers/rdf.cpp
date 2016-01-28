#include "rdf.h"
#include "../lammpscontroller.h"
#include <compute_rdf.h>
#include <fix_ave_time.h>
RDF::RDF()
{

}

QString RDF::generatePairString()
{
    QString str;
    for(const QVariant &variant : m_atomPairs) {
        str.append(QString("%1 ").arg(variant.value<double>()));
    }
    return str;
}

int RDF::numberOfPairs()
{
    return m_atomPairs.size()/2;
}

void RDF::updateCommand()
{
    setCommand(QString("compute %1 %2 rdf %3 %4").arg(identifier()).arg(m_group).arg(m_numberOfBins).arg(generatePairString()));
}

QList<QString> RDF::enabledCommands()
{
    return {
        QString("compute %1 %2 rdf %3 %4").arg(identifier()).arg(m_group).arg(m_numberOfBins).arg(generatePairString()),
        QString("fix %1_ave all ave/time 1 1 1 c_%1 file %1.txt mode vector").arg(identifier())
    };
}

QList<QString> RDF::disableCommands()
{
    return { QString("unfix %1_ave").arg(identifier()),
             QString("uncompute %1").arg(identifier())
    };
}

QList<QString> RDF::resetCommands()
{
    return { QString("unfix %1_ave").arg(identifier()),
             QString("uncompute %1").arg(identifier()),
             QString("compute %1 %2 rdf %3 %4").arg(identifier()).arg(m_group).arg(m_numberOfBins).arg(generatePairString()),
             QString("fix %1_ave all ave/time 1 1 1 c_%1 file %1.txt mode vector ave running").arg(identifier())
    };
}

void RDF::update(LAMMPSController *lammpsController)
{
    SimulatorControl::update(lammpsController);
    QString fixAveTimeIdentifier = QString("%1_ave").arg(identifier());
    LAMMPS_NS::FixAveTime *lmp_fix = dynamic_cast<LAMMPS_NS::FixAveTime*>(lammpsController->findFixByIdentifier(fixAveTimeIdentifier));
    if(lmp_fix != nullptr) {
        if(m_dataSource)  m_dataSource->points().resize(m_numberOfBins);
        if(m_dataSource1) m_dataSource1->points().resize(m_numberOfBins);
        if(m_dataSource2) m_dataSource2->points().resize(m_numberOfBins);
        if(m_dataSource3) m_dataSource3->points().resize(m_numberOfBins);
        if(m_dataSource4) m_dataSource4->points().resize(m_numberOfBins);

        for(int i=0; i<m_numberOfBins; i++) {
            float x = lmp_fix->compute_array(i,0);
            if(numberOfPairs() >= 1 && m_dataSource) {
                m_dataSource->points()[i].setX(x);
                m_dataSource->points()[i].setY(lmp_fix->compute_array(i,1));
            }
            if(numberOfPairs() >= 2 && m_dataSource1) {
                m_dataSource1->points()[i].setX(x);
                m_dataSource1->points()[i].setY(lmp_fix->compute_array(i,3));
            }
            if(numberOfPairs() >= 3 && m_dataSource2) {
                m_dataSource2->points()[i].setX(x);
                m_dataSource2->points()[i].setY(lmp_fix->compute_array(i,5));
            }
            if(numberOfPairs() >= 4 && m_dataSource3) {
                m_dataSource3->points()[i].setX(x);
                m_dataSource3->points()[i].setY(lmp_fix->compute_array(i,7));
            }
            if(numberOfPairs() >= 5 && m_dataSource4) {
                m_dataSource4->points()[i].setX(x);
                m_dataSource4->points()[i].setY(lmp_fix->compute_array(i,9));
            }
        }
        if(m_dataSource) m_dataSource->update();
        if(m_dataSource1) m_dataSource1->update();
        if(m_dataSource2) m_dataSource2->update();
        if(m_dataSource3) m_dataSource3->update();
        if(m_dataSource4) m_dataSource4->update();

    }
}

QVariantList RDF::atomPairs() const
{
    return m_atomPairs;
}

QString RDF::group() const
{
    return m_group;
}

int RDF::numberOfBins() const
{
    return m_numberOfBins;
}

LineGraphDataSource *RDF::dataSource1() const
{
    return m_dataSource1;
}

LineGraphDataSource *RDF::dataSource2() const
{
    return m_dataSource2;
}

LineGraphDataSource *RDF::dataSource3() const
{
    return m_dataSource3;
}

LineGraphDataSource *RDF::dataSource4() const
{
    return m_dataSource4;
}

void RDF::setAtomPairs(QVariantList atomPairs)
{
    if (m_atomPairs == atomPairs)
        return;

    m_atomPairs = atomPairs;

    emit atomPairsChanged(atomPairs);
}

void RDF::setGroup(QString group)
{
    if (m_group == group)
        return;

    m_group = group;
    emit groupChanged(group);
}

void RDF::setNumberOfBins(int numberOfBins)
{
    if (m_numberOfBins == numberOfBins)
        return;

    m_numberOfBins = numberOfBins;
    emit numberOfBinsChanged(numberOfBins);
}

void RDF::setDataSource1(LineGraphDataSource *dataSource1)
{
    if (m_dataSource1 == dataSource1)
        return;

    m_dataSource1 = dataSource1;
    emit dataSource1Changed(dataSource1);
}

void RDF::setDataSource2(LineGraphDataSource *dataSource2)
{
    if (m_dataSource2 == dataSource2)
        return;

    m_dataSource2 = dataSource2;
    emit dataSource2Changed(dataSource2);
}

void RDF::setDataSource3(LineGraphDataSource *dataSource3)
{
    if (m_dataSource3 == dataSource3)
        return;

    m_dataSource3 = dataSource3;
    emit dataSource3Changed(dataSource3);
}

void RDF::setDataSource4(LineGraphDataSource *dataSource4)
{
    if (m_dataSource4 == dataSource4)
        return;

    m_dataSource4 = dataSource4;
    emit dataSource4Changed(dataSource4);
}
