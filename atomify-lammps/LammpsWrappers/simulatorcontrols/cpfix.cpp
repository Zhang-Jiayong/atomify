#include "cpfix.h"
#include "lammpscontroller.h"
#include "../system.h"
#include "../../dataproviders/data2d.h"
#include <style_compute.h>
#include <iostream>
using namespace std;

CPFix::CPFix(Qt3DCore::QNode *parent) : SimulatorControl(parent)
{
}

CPFix::~CPFix() { }

bool CPFix::copyData(FixAveChunk *fix, LAMMPSController *lammpsController) {
    enum{BIN1D,BIN2D,BIN3D,BINSPHERE,BINCYLINDER,
         TYPE,MOLECULE,COMPUTE,FIX,VARIABLE};

    if(!fix) return false;
    int dim;
    ComputeChunkAtom *chunk = static_cast<ComputeChunkAtom*>(fix->extract("cchunk", dim));
    int *nvalues = static_cast<int*>(fix->extract("nvalues", dim));
    int *nchunk = static_cast<int*>(fix->extract("nchunk", dim));
    int *colextra = static_cast<int*>(fix->extract("colextra", dim));
    if(!nvalues || !nchunk || !chunk || !colextra) {
        qDebug() << "Warning, could not get values from FixAveChunk::extract.";
        return true;
    }

    if(chunk->which == BIN2D) {
        setInteractive(true);

        if(m_dataRaw.size() != *nvalues) {
            m_dataRaw.clear();
            m_data.clear();
            for(int i=0; i<*nvalues; i++) {
                Data2D *data = new Data2D(this);
                m_dataRaw.push_back(data);
                m_data.push_back(QVariant::fromValue(data));
            }
        }
        if(fix->nextvalid() == lammpsController->system()->timesteps()+1) {
            QStringList labels = {"x", "y", "z"};

            int x = chunk->dim[0];
            int z = chunk->dim[1];
            int y = 3 - x-z;

            QSize size(chunk->nlayers[0], chunk->nlayers[1]);

            float xMin = lammpsController->system()->origin()[x];
            float xMax = lammpsController->system()->origin()[x] + lammpsController->system()->size()[x];
            float zMin = lammpsController->system()->origin()[z];
            float zMax = lammpsController->system()->origin()[z] + lammpsController->system()->size()[z];

            QString xLabel = labels[x];
            QString yLabel = labels[y];
            QString zLabel = labels[z];

            float minValues[*nvalues];
            float maxValues[*nvalues];

            for(int i=0; i<*nvalues; i++) {
                m_dataRaw[i]->setXMin(xMin);
                m_dataRaw[i]->setXMax(xMax);
                m_dataRaw[i]->setZMin(zMin);
                m_dataRaw[i]->setZMax(zMax);
                m_dataRaw[i]->setXLabel(xLabel);
                m_dataRaw[i]->setYLabel(yLabel);
                m_dataRaw[i]->setZLabel(zLabel);
                m_dataRaw[i]->setSize(size);
                minValues[i] = 1e9;
                maxValues[i] = -1e9;
            }

            for(int i=0; i<*nchunk; i++) {
                float x = fix->compute_array(i,0);
                float z = fix->compute_array(i,1);

                for(int j=0; j<*nvalues; j++) {
                    int valueIndex = *colextra+1+j;
                    float y = fix->compute_array(i,valueIndex);
                    minValues[j] = std::min(minValues[j], y);
                    maxValues[j] = std::max(maxValues[j], y);
                    m_dataRaw[j]->setValue(x,y,z);
                }
            }

            for(int i=0; i<*nvalues; i++) {
                m_dataRaw[i]->setYMin(minValues[i]);
                m_dataRaw[i]->setYMax(maxValues[i]);
                m_dataRaw[i]->update();
            }
        }
    }
}

void CPFix::copyData(LAMMPSController *lammpsController)
{
    // if(lammpsController->system()->timesteps() % m_frequency != 0) return;
    if(lastUpdate != -1 && (lammpsController->system()->timesteps()-lastUpdate) < m_frequency) return;
    // if(lammpsController->system()->timesteps() % m_frequency != 0) return;
    Fix *lmp_fix = lammpsController->findFixByIdentifier(identifier());
    if(lmp_fix == nullptr) return;
    if(copyData(dynamic_cast<FixAveChunk*>(lmp_fix), lammpsController)) return;
}

bool CPFix::interactive() const
{
    return m_interactive;
}

QVariantList CPFix::data() const
{
    return m_data;
}

QVariant CPFix::model() const
{
    return m_model;
}

void CPFix::update(LAMMPSController *lammpsController)
{
    SimulatorControl::update(lammpsController);
}

QList<QString> CPFix::enabledCommands()
{
    return { fullCommand() };
}

QList<QString> CPFix::disableCommands()
{
    return {QString("unfix %1 ").arg(identifier())};
}

void CPFix::updateCommand()
{

}

bool CPFix::existsInLammps(LAMMPSController *lammpsController)
{
    return lammpsController->fixExists(identifier());
}

QString CPFix::group() const
{
    return m_group;
}

int CPFix::frequency() const
{
    return m_frequency;
}

void CPFix::setGroup(QString group)
{
    if (m_group == group)
        return;

    m_group = group;
    emit groupChanged(group);
}

void CPFix::setFrequency(int frequency)
{
    if (m_frequency == frequency)
        return;

    m_frequency = frequency;
    emit frequencyChanged(frequency);
}

void CPFix::setInteractive(bool interactive)
{
    if (m_interactive == interactive)
        return;

    m_interactive = interactive;
    emit interactiveChanged(interactive);
}

void CPFix::setData(QVariantList data)
{
    if (m_data == data)
        return;

    m_data = data;
    emit dataChanged(data);
}

void CPFix::setModel(QVariant model)
{
    if (m_model == model)
        return;

    m_model = model;
    emit modelChanged(model);
}

QList<QString> CPFix::resetCommands()
{
    return { QString("unfix %1").arg(identifier()), fullCommand() };
}

QString CPFix::createCommandPrefix()
{
    return QString("fix %1 %2 ").arg(identifier()).arg(group());
}
