#ifndef LAMMPSTHREAD_H
#define LAMMPSTHREAD_H

#include "LammpsWrappers/atomdata.h"

#include <QThread>
#include <QMutex>
#include <QVector3D>
#include <QVector>

namespace LAMMPS_NS {
class LAMMPS;
}

namespace atomify {

class Cancelled : public std::exception { };

struct LAMMPSData {
    AtomData atomData;
    QVector<QVector3D> positions;
    int timestep = 0;
};

class LAMMPSThread : public QThread
{
    Q_OBJECT
public:
    explicit LAMMPSThread(QObject *parent = nullptr);
    LAMMPSData data();
    bool dataDirty() const;

protected:
    void run() override;
    void callback(LAMMPS_NS::LAMMPS *lmp, int mode);
private:
    mutable QMutex m_mutex;
    LAMMPSData m_data;
    bool m_dataDirty;
};

}
#endif // LAMMPSTHREAD_H
