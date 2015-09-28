#include "mysimulator.h"
#include <library.h>
#include <atom.h>
#include <domain.h>
#include <update.h>
#include <modify.h>
#include <neighbor.h>
#include <neigh_list.h>

#include <core/camera.h>
#include <string>
#include <sstream>
#include <SimVis/Spheres>
#include <SimVis/Cylinders>
#include <SimVis/Points>
#include <QUrl>
#include <QString>
#include <QQmlFile>
#include <QDir>
#include <iostream>
#include <fstream>
#include <memory>
#include <QStandardPaths>
using namespace std;

MyWorker::MyWorker() {
    m_sinceStart.start();
    m_elapsed.start();
    m_lammpsController.setWorker(this);
}

void MyWorker::synchronizeSimulator(Simulator *simulator)
{
    MySimulator *mySimulator = qobject_cast<MySimulator*>(simulator);

    if(!mySimulator->m_scriptToRun.isEmpty()) {
        // We have a queued script, now run it
        m_lammpsController.reset();
        m_lammpsController.runScript(mySimulator->m_scriptToRun);
        mySimulator->m_scriptToRun.clear();
    }

    if(m_lammpsController.crashed() && !m_lammpsController.currentException().isReported()) {

        cout << "LAMMPS crashed and we picked it up :D" << endl;
        cout << "An error occured in " << m_lammpsController.currentException().file() << " on line " << m_lammpsController.currentException().line() << endl;
        cout << "Message: " << m_lammpsController.currentException().error() << endl;
        m_lammpsController.currentException().setIsReported(true);
    }

    if(mySimulator->atomStyle() != NULL) {
        // Sync new atom styles from Simulator (QML) to Worker
        m_atomStyle.setData(mySimulator->atomStyle()->data());
    }

    // Sync values from QML and simulator
    m_lammpsController.setComputes(mySimulator->computes());
    m_lammpsController.setPaused(mySimulator->paused());
    m_lammpsController.setSimulationSpeed(mySimulator->simulationSpeed());

    // Sync properties from lammps controller
    mySimulator->setSimulationTime(m_lammpsController.simulationTime());
    mySimulator->setNumberOfAtoms(m_lammpsController.numberOfAtoms());
    mySimulator->setNumberOfAtomTypes(m_lammpsController.numberOfAtomTypes());
    mySimulator->setSystemSize(m_lammpsController.systemSize());
    mySimulator->setLammpsOutput(&m_lammpsController.output);
    mySimulator->setTimePerTimestep(m_lammpsController.timePerTimestep());
    if(m_willPause) {
        m_lammpsController.setPaused(true);
        mySimulator->setPaused(true);
        m_willPause = false;
    }
}

void MyWorker::synchronizeRenderer(Renderable *renderableObject)
{
    Spheres* spheres = qobject_cast<Spheres*>(renderableObject);
#define STUFF
#ifdef STUFF
    QVector3D p1(-4,2,0);
    QVector3D p2(4,2,0);

    if(spheres) {
        QVector<QVector3D> &positions = spheres->positions();
        QVector<float> &scales = spheres->scales();
        QVector<QColor> &colors = spheres->colors();
        colors.resize(2);
        positions.resize(2);
        scales.resize(2);
        positions[0] = p1;
        positions[1] = p2;
        colors[0] = QColor("red");
        colors[1] = QColor("red");
        scales[0] = 2.0;
        scales[1] = 2.0;
        spheres->setDirty(true);
    }
    Cylinders *cylinders = qobject_cast<Cylinders*>(renderableObject);
    if(cylinders) {
        QVector<CylinderVBOData> &points = cylinders->vertices();
        points.resize(1);
        points[0].vertex1 = p1;
        points[0].vertex2 = p2;
        points[0].radius1 = 2.0;
        points[0].radius2 = 2.0;
        cylinders->setRadius(0.2);
        cylinders->setDirty(true);
    }
#else

    if(spheres) {
        LAMMPS *lammps = m_lammpsController.lammps();
        // if(!lammps || !m_lammpsController.dataDirty()) return;
        if(!lammps) return;
        m_lammpsController.setDataDirty(false);
        QVector<QVector3D> &positions = spheres->positions();
        QVector<float> &scales = spheres->scales();
        QVector<QColor> &colors = spheres->colors();
        colors.resize(lammps->atom->natoms);
        scales.resize(lammps->atom->natoms);
        positions.resize(lammps->atom->natoms);
        m_atomTypes.resize(lammps->atom->natoms);

        double position[3];

        for(unsigned int i=0; i<lammps->atom->natoms; i++) {
            position[0] = lammps->atom->x[i][0];
            position[1] = lammps->atom->x[i][1];
            position[2] = lammps->atom->x[i][2];
            lammps->domain->remap(position);

            positions[i][0] = position[0] - lammps->domain->prd_half[0];
            positions[i][1] = position[1] - lammps->domain->prd_half[1];
            positions[i][2] = position[2] - lammps->domain->prd_half[2];
            m_atomTypes[i] = lammps->atom->type[i];
        }
        m_atomStyle.setColorsAndScales(colors, scales, m_atomTypes);
        spheres->setDirty(true);
    }

    Cylinders *cylinders = qobject_cast<Cylinders*>(renderableObject);
    if(cylinders) {
        LAMMPS *lammps = m_lammpsController.lammps();
        if(!lammps) return;

        QVector<CylinderVBOData> &points = cylinders->vertices();
        points.resize(0);
        if( lammps->neighbor->nlist > 0) {
            NeighList *list = lammps->neighbor->lists[0];
            int inum = list->inum;
            int *ilist = list->ilist;
            int *numneigh = list->numneigh;
            int **firstneigh = list->firstneigh;

            double posi[3];
            double posj[3];
            for(int ii=0; ii<inum; ii++) {
                int i = ilist[ii];
                posi[0] = lammps->atom->x[i][0];
                posi[1] = lammps->atom->x[i][1];
                posi[2] = lammps->atom->x[i][2];

                lammps->domain->remap(posi);

                QVector3D qposi(posi[0], posi[1], posi[2]);
                // two-body interactions, skip half of them
                int *jlist = firstneigh[i];
                int jnum = numneigh[i];
                for (int jj = 0; jj < jnum; jj++) {
                    int j = jlist[jj];
                    posj[0] = lammps->atom->x[j][0];
                    posj[1] = lammps->atom->x[j][1];
                    posj[2] = lammps->atom->x[j][2];
                    lammps->domain->remap(posj);

                    QVector3D qposj(posj[0], posj[1], posj[2]);
                    double dr2 = (qposj-qposi).lengthSquared();
                    if(dr2 > 1.0*1.0) continue;


                    CylinderVBOData data;
                    data.vertex1[0] = posi[0] - lammps->domain->prd_half[0];;
                    data.vertex1[1] = posi[1] - lammps->domain->prd_half[1];;
                    data.vertex1[2] = posi[2] - lammps->domain->prd_half[2];;
                    data.vertex2[0] = posj[0] - lammps->domain->prd_half[0];;
                    data.vertex2[1] = posj[1] - lammps->domain->prd_half[1];;
                    data.vertex2[2] = posj[2] - lammps->domain->prd_half[2];;
                    data.radius1 = 0.23*0.5;
                    data.radius2 = 0.23*0.5;
                    points.push_back(data);
                }
            }

            cylinders->setDirty(true);
        }
    }
#endif
}

void MyWorker::work()
{
    m_lammpsController.tick();
    auto dt = m_elapsed.elapsed();
    double delta = 16 - dt;
    if(delta > 0) {
        QThread::currentThread()->msleep(delta);
    }
    m_elapsed.restart();
}
bool MyWorker::willPause() const
{
    return m_willPause;
}

void MyWorker::setWillPause(bool willPause)
{
    m_willPause = willPause;
}


MyWorker *MySimulator::createWorker()
{
    return new MyWorker();
}
QMap<QString, CPCompute *> MySimulator::computes() const
{
    return m_computes;
}

void MySimulator::setComputes(const QMap<QString, CPCompute *> &computes)
{
    m_computes = computes;
}

void MySimulator::addCompute(CPCompute *compute)
{
    m_computes[compute->identifier()] = compute;
}

LammpsOutput *MySimulator::lammpsOutput() const
{
    return m_lammpsOutput;
}

bool MySimulator::paused() const
{
    return m_paused;
}

double MySimulator::simulationTime() const
{
    return m_simulationTime;
}

AtomStyle *MySimulator::atomStyle() const
{
    return m_atomStyle;
}

int MySimulator::numberOfAtoms() const
{
    return m_numberOfAtoms;
}

int MySimulator::numberOfAtomTypes() const
{
    return m_numberOfAtomTypes;
}

QVector3D MySimulator::systemSize() const
{
    return m_systemSize;
}

double MySimulator::timePerTimestep() const
{
    return m_timePerTimestep;
}

void MySimulator::setLammpsOutput(LammpsOutput *lammpsOutput)
{
    if (m_lammpsOutput == lammpsOutput)
        return;

    m_lammpsOutput = lammpsOutput;
    emit lammpsOutputChanged(lammpsOutput);
}


int MySimulator::simulationSpeed() const
{
    return m_simulationSpeed;
}

void MySimulator::setSimulationSpeed(int arg)
{
    if (m_simulationSpeed == arg)
        return;

    m_simulationSpeed = arg;
    emit simulationSpeedChanged(arg);
}

void MySimulator::setPaused(bool paused)
{
    if (m_paused == paused)
        return;

    m_paused = paused;
    emit pausedChanged(paused);
}

void MySimulator::setSimulationTime(double simulationTime)
{
    if (m_simulationTime == simulationTime)
        return;

    m_simulationTime = simulationTime;
    emit simulationTimeChanged(simulationTime);
}

void MySimulator::setAtomStyle(AtomStyle *atomStyle)
{
    if (m_atomStyle == atomStyle)
        return;

    m_atomStyle = atomStyle;
    emit atomStyleChanged(atomStyle);
}

void MySimulator::setNumberOfAtoms(int numberOfAtoms)
{
    if (m_numberOfAtoms == numberOfAtoms)
        return;

    m_numberOfAtoms = numberOfAtoms;
    emit numberOfAtomsChanged(numberOfAtoms);
}

void MySimulator::setNumberOfAtomTypes(int numberOfAtomTypes)
{
    if (m_numberOfAtomTypes == numberOfAtomTypes)
        return;

    m_numberOfAtomTypes = numberOfAtomTypes;
    if(m_atomStyle) m_atomStyle->setMinimumSize(numberOfAtomTypes);
    emit numberOfAtomTypesChanged(numberOfAtomTypes);
}

void MySimulator::setSystemSize(QVector3D systemSize)
{
    if (m_systemSize == systemSize)
        return;

    m_systemSize = systemSize;
    emit systemSizeChanged(systemSize);
}

void MySimulator::setTimePerTimestep(double timePerTimestep)
{
    if (m_timePerTimestep == timePerTimestep)
        return;

    m_timePerTimestep = timePerTimestep;
    emit timePerTimestepChanged(timePerTimestep);
}

void MySimulator::runScript(QString script)
{
    // This is typically called from the QML thread.
    // We have to wait for synchronization before we actually load this script
    m_scriptToRun = script;
}
