#include "atoms.h"
#include <algorithm>
#include <atom.h>
#include <domain.h>
#include <neighbor.h>
#include <neigh_list.h>
#include "modifiers/modifiers.h"
#include "mysimulator.h"
#include "bonds.h"
#include "system.h"
using namespace LAMMPS_NS;

Atoms::Atoms(AtomifySimulator *simulator)
{
    // Colors from JMOL: http://jmol.sourceforge.net/jscolors/
    // Atomic radii from wikipedia (van der waals radius)

    m_sphereData = new SphereData(simulator);
    m_bondData = new BondData(simulator);
    m_bonds = new Bonds();
    m_atomData.neighborList.bonds = m_bonds;

    m_atomStyleTypes.insert("hydrogen", new AtomStyle(1.20, "#CCCCCC"));
    m_atomStyleTypes.insert("helium", new AtomStyle(1.40, "#D9FFFF"));
    m_atomStyleTypes.insert("lithium", new AtomStyle(1.82, "#CC80FF"));
    m_atomStyleTypes.insert("beryllium", new AtomStyle(1.53, "#C2FF00"));
    m_atomStyleTypes.insert("boron", new AtomStyle(1.92, "#FFB5B5"));
    m_atomStyleTypes.insert("carbon", new AtomStyle(1.70, "#909090"));
    m_atomStyleTypes.insert("nitrogen", new AtomStyle(1.55, "#3050F8"));
    m_atomStyleTypes.insert("oxygen", new AtomStyle(1.52, "#AA0000"));
    m_atomStyleTypes.insert("fluorine", new AtomStyle(1.35, "#90E050"));
    m_atomStyleTypes.insert("neon", new AtomStyle(1.54, "#3050F8"));
    m_atomStyleTypes.insert("sodium", new AtomStyle(2.27, "#AB5CF2"));
    m_atomStyleTypes.insert("magnesium", new AtomStyle(1.73, "#8AFF00"));
    m_atomStyleTypes.insert("aluminium", new AtomStyle(1.84, "#BFA6A6"));
    m_atomStyleTypes.insert("silicon", new AtomStyle(2.27, "#F0C8A0"));
    m_atomStyleTypes.insert("phosphorus", new AtomStyle(1.80, "#FF8000"));
    m_atomStyleTypes.insert("sulfur", new AtomStyle(1.80, "#FFFF30"));
    m_atomStyleTypes.insert("chlorine", new AtomStyle(1.75, "#1FF01F"));
    m_atomStyleTypes.insert("argon", new AtomStyle(1.88, "#80D1E3"));
    m_atomStyleTypes.insert("potassium", new AtomStyle(2.75, "#8F40D4"));
    m_atomStyleTypes.insert("calcium", new AtomStyle(2.31, "#3DFF00"));

    for(int i=0; i<50; i++) {
        m_atomStyles.push_back(m_atomStyleTypes["hydrogen"]);
        m_atomStyles.push_back(m_atomStyleTypes["helium"]);
        m_atomStyles.push_back(m_atomStyleTypes["lithium"]);
        m_atomStyles.push_back(m_atomStyleTypes["beryllium"]);
        m_atomStyles.push_back(m_atomStyleTypes["boron"]);
        m_atomStyles.push_back(m_atomStyleTypes["carbon"]);
        m_atomStyles.push_back(m_atomStyleTypes["nitrogen"]);
        m_atomStyles.push_back(m_atomStyleTypes["oxygen"]);
        m_atomStyles.push_back(m_atomStyleTypes["fluorine"]);
        m_atomStyles.push_back(m_atomStyleTypes["neon"]);
        m_atomStyles.push_back(m_atomStyleTypes["sodium"]);
        m_atomStyles.push_back(m_atomStyleTypes["magnesium"]);
        m_atomStyles.push_back(m_atomStyleTypes["aluminium"]);
        m_atomStyles.push_back(m_atomStyleTypes["silicon"]);
        m_atomStyles.push_back(m_atomStyleTypes["phosphorus"]);
        m_atomStyles.push_back(m_atomStyleTypes["sulfur"]);
        m_atomStyles.push_back(m_atomStyleTypes["chlorine"]);
        m_atomStyles.push_back(m_atomStyleTypes["argon"]);
        m_atomStyles.push_back(m_atomStyleTypes["potassium"]);
        m_atomStyles.push_back(m_atomStyleTypes["calcium"]);
    }
}

void Atoms::synchronize(LAMMPSController *lammpsController)
{
    LAMMPS *lammps = lammpsController->lammps();
    if(!lammps) { return; }

    Atom *atom = lammps->atom;
    Domain *domain = lammps->domain;
    int *types = lammps->atom->type;
    int numberOfAtoms = atom->natoms;

    if(m_atomData.positions.size() != numberOfAtoms) {
        m_atomData.positions.resize(numberOfAtoms);
    }
    if(m_atomData.deltaPositions.size() != numberOfAtoms) {
        m_atomData.deltaPositions.resize(numberOfAtoms);
        for(QVector3D &delta : m_atomData.deltaPositions) delta = QVector3D(); // Reset delta
    }
    if(m_atomData.types.size() != numberOfAtoms) {
        m_atomData.types.resize(numberOfAtoms);
    }
    if(m_atomData.bitmask.size() != numberOfAtoms) {
        m_atomData.bitmask.resize(numberOfAtoms);
    }
    if(m_atomData.visible.size() != numberOfAtoms) {
        m_atomData.visible.resize(numberOfAtoms);
    }
    if(m_atomData.colors.size() != numberOfAtoms) {
        m_atomData.colors.resize(numberOfAtoms);
        for(QVector3D &color : m_atomData.colors) color = QVector3D(0.9, 0.2, 0.1);
    }
    if(m_atomData.originalIndex.size() != numberOfAtoms) {
        m_atomData.originalIndex.resize(numberOfAtoms);
    }
    if(m_atomData.radii.size() != numberOfAtoms) {
        m_atomData.radii.resize(numberOfAtoms);
        for(float &radii : m_atomData.radii) radii = 1.0;
    }

    // lammps->

    for(int i=0; i<numberOfAtoms; i++) {
        m_atomData.types[i] = types[i];
        m_atomData.originalIndex[i] = i;

        double position[3];
        position[0] = atom->x[i][0];
        position[1] = atom->x[i][1];
        position[2] = atom->x[i][2];
        domain->remap(position); // remap into system boundaries with PBC

        m_atomData.positions[i][0] = position[0];
        m_atomData.positions[i][1] = position[1];
        m_atomData.positions[i][2] = position[2];
        m_atomData.bitmask[i] = atom->mask[i];
        m_atomData.visible[i] = true;
        m_atomData.deltaPositions[i] = QVector3D();
    }

    // if(m_bonds->enabled()) m_atomData.neighborList.synchronize(lammps);
}

void Atoms::processModifiers(System *system)
{
    m_atomDataProcessed = m_atomData;
    if(!m_atomDataProcessed.isValid()) {
        qDebug() << "Atom data is not valid before modifiers.";
        exit(1);
    }

    for(QVariant &modifier_ : m_modifiers) {
        Modifier *modifier = modifier_.value<Modifier*>();
        modifier->setSystem(system);
        modifier->apply(m_atomDataProcessed);
        if(!m_atomDataProcessed.isValid()) {
            // TODO: insert modifier name to debug message
            qDebug() << "Atom data is not valid after modifier.";
            exit(1);
        }
    }
}

void Atoms::createRenderererData(LAMMPSController *lammpsController) {
    generateSphereData(m_atomDataProcessed);
    generateBondData(m_atomDataProcessed, lammpsController);
}

float Atoms::bondScale() const
{
    return m_bondScale;
}

float Atoms::sphereScale() const
{
    return m_sphereScale;
}

QString Atoms::renderingMode() const
{
    return m_renderingMode;
}

void Atoms::synchronizeRenderer() {
    int numSpheres = m_sphereDataRaw.size() / sizeof(SphereVBOData);
    m_sphereData->setData(m_sphereDataRaw, numSpheres);

    int numBonds = m_bondsDataRaw.size() / sizeof(BondVBOData);
    m_bondData->setData(m_bondsDataRaw, numBonds);
}
void Atoms::generateSphereData(AtomData &atomData) {
    int visibleAtomCount = 0;

    for(int i = 0; i<atomData.size(); i++) {
        if(atomData.visible[i]) {
            atomData.positions[visibleAtomCount] = atomData.positions[i];
            atomData.colors[visibleAtomCount] = atomData.colors[i];
            atomData.radii[visibleAtomCount] = atomData.radii[i];
            if(m_renderingMode == "Stick") atomData.radii[visibleAtomCount] = 0.1*m_bondScale;
            else if(m_renderingMode == "Wireframe") atomData.radii[visibleAtomCount] = 0.1*m_bondScale;
            visibleAtomCount++;
        }
    }
    atomData.positions.resize(visibleAtomCount);
    atomData.colors.resize(visibleAtomCount);
    atomData.radii.resize(visibleAtomCount);

    m_sphereDataRaw.resize(visibleAtomCount * sizeof(SphereVBOData));
    SphereVBOData *vboData = reinterpret_cast<SphereVBOData *>(m_sphereDataRaw.data());
    for(int i=0; i<visibleAtomCount; i++) {
        SphereVBOData &vbo = vboData[i];
        vbo.position = atomData.positions[i] + atomData.deltaPositions[i];
        vbo.color = atomData.colors[i];
        vbo.radius = atomData.radii[i]*m_sphereScale;
    }
}

bool Atoms::dirtyData() const
{
    return m_dirtyData;
}

void Atoms::setDirtyData(bool dirtyData)
{
    m_dirtyData = dirtyData;
}

void Atoms::generateBondData(AtomData &atomData, LAMMPSController *controller) {
    bondsDataRaw.resize(0);
    if(!m_bonds->enabled()) {
        m_bondsDataRaw.clear();
        return;
    }

//    const Neighborlist &neighborList = atomData.neighborList;
//    if(neighborList.neighbors.size()==0) return;
//    long numPairs = 0;

    bool hasNeighborLists = controller->lammps()->neighbor->nlist > 0;
    if(!hasNeighborLists) {
        m_bondsDataRaw.clear();
        return;
    }

    NeighList *list = controller->lammps()->neighbor->lists[0];
    const int inum = list->inum;
    int *numneigh = list->numneigh;
    int **firstneigh = list->firstneigh;

    bondsDataRaw.reserve(atomData.positions.size());
    for(int ii=0; ii<atomData.size(); ii++) {
        if(!atomData.visible[ii]) continue;
        int i = atomData.originalIndex[ii];

        const QVector3D deltaPosition_i = atomData.deltaPositions[ii];
        const QVector3D position_i = atomData.positions[ii] + deltaPosition_i;
        const int atomType_i = atomData.types[ii];

        const QVector<float> &bondLengths = m_bonds->bondLengths()[atomType_i];
        const float sphereRadius_i = atomData.radii[ii];

        if(i >= inum) continue; // Atom i is outside the neighbor list. Will probably not happen, but let's skip in that case.

        int *jlist = firstneigh[i];
        int jnum = numneigh[i];
        for (int jj = 0; jj < jnum; jj++) {
            int j = jlist[jj];
            j &= NEIGHMASK;
            if(j >= atomData.size()) continue; // Probably a ghost atom from LAMMPS
            if(!atomData.visible[j]) continue;

            QVector3D position_j = atomData.positions[j];
            position_j += deltaPosition_i;

            const int &atomType_j = atomData.types[j];

            float dx = position_i[0] - position_j[0];
            float dy = position_i[1] - position_j[1];
            float dz = position_i[2] - position_j[2];
            float rsq = dx*dx + dy*dy + dz*dz; // Componentwise has 10% lower execution time than (position_i - position_j).lengthSquared()
            if(rsq < bondLengths[atomType_j]*bondLengths[atomType_j] ) {
                BondVBOData bond;
                bond.vertex1[0] = position_i[0];
                bond.vertex1[1] = position_i[1];
                bond.vertex1[2] = position_i[2];
                bond.vertex2[0] = position_j[0];
                bond.vertex2[1] = position_j[1];
                bond.vertex2[2] = position_j[2];
                float bondRadius = 0.1*m_bondScale;
                bond.radius1 = bondRadius;
                bond.radius2 = bondRadius;
                bond.sphereRadius1 = sphereRadius_i*m_sphereScale;
                bond.sphereRadius2 = atomData.radii[j]*m_sphereScale;
                bondsDataRaw.push_back(bond);
            }
        }
    }
    m_bondsDataRaw.resize(bondsDataRaw.size() * sizeof(BondVBOData));
    BondVBOData *posData = reinterpret_cast<BondVBOData*>(m_bondsDataRaw.data());
    // TODO can we just set the address here? Instead of copying.
    for(const BondVBOData &pos : bondsDataRaw) {
        *posData++ = pos;
    }
}

SphereData *Atoms::sphereData() const
{
    return m_sphereData;
}

QVector<AtomStyle *> &Atoms::atomStyles()
{
    return m_atomStyles;
}

void Atoms::setAtomType(int atomType, QString elementName)
{
    if(!m_atomStyleTypes.contains(elementName)) {
        qDebug() << "Warning, element " << elementName << " was not found.";
        return;
    }

    m_atomStyles[atomType] = m_atomStyleTypes[elementName];
}

void Atoms::setAtomColorAndScale(int atomType, QColor color, float size)
{
    if(atomType >= m_atomStyles.size()) return;

    m_atomStyles[atomType]->color = color;
    m_atomStyles[atomType]->radius = size;
}

void Atoms::setAtomColor(int atomType, QColor color) {
    if(atomType >= m_atomStyles.size()) return;

    m_atomStyles[atomType]->color = color;
}

BondData *Atoms::bondData() const
{
    return m_bondData;
}

Bonds *Atoms::bonds() const
{
    return m_bonds;
}

AtomData &Atoms::atomData()
{
    return m_atomData;
}

void Atoms::reset()
{
    QVector<SphereVBOData> emptySphereVBOData;
    QVector<BondVBOData> emptyBondVBOData;
    m_sphereData->setData(emptySphereVBOData);
    m_bondData->setData(emptyBondVBOData);
    m_atomData.reset();
    m_atomDataProcessed.reset();
    m_bonds->reset();
    m_atomStyles.clear();
    for(int i=0; i<50; i++) {
        m_atomStyles.push_back(m_atomStyleTypes["hydrogen"]);
        m_atomStyles.push_back(m_atomStyleTypes["helium"]);
        m_atomStyles.push_back(m_atomStyleTypes["lithium"]);
        m_atomStyles.push_back(m_atomStyleTypes["beryllium"]);
        m_atomStyles.push_back(m_atomStyleTypes["boron"]);
        m_atomStyles.push_back(m_atomStyleTypes["carbon"]);
        m_atomStyles.push_back(m_atomStyleTypes["nitrogen"]);
        m_atomStyles.push_back(m_atomStyleTypes["oxygen"]);
        m_atomStyles.push_back(m_atomStyleTypes["fluorine"]);
        m_atomStyles.push_back(m_atomStyleTypes["neon"]);
        m_atomStyles.push_back(m_atomStyleTypes["sodium"]);
        m_atomStyles.push_back(m_atomStyleTypes["magnesium"]);
        m_atomStyles.push_back(m_atomStyleTypes["aluminium"]);
        m_atomStyles.push_back(m_atomStyleTypes["silicon"]);
        m_atomStyles.push_back(m_atomStyleTypes["phosphorus"]);
        m_atomStyles.push_back(m_atomStyleTypes["sulfur"]);
        m_atomStyles.push_back(m_atomStyleTypes["chlorine"]);
        m_atomStyles.push_back(m_atomStyleTypes["argon"]);
        m_atomStyles.push_back(m_atomStyleTypes["potassium"]);
        m_atomStyles.push_back(m_atomStyleTypes["calcium"]);
    }
}

bool Atoms::sort() const
{
    return m_sort;
}

QVariantList Atoms::modifiers() const
{
    return m_modifiers;
}

void Atoms::setModifiers(QVariantList modifiers)
{
    if (m_modifiers == modifiers)
        return;

    m_modifiers = modifiers;
    emit modifiersChanged(modifiers);
}

void Atoms::setSort(bool sort)
{
    if (m_sort == sort)
        return;

    m_sort = sort;
    emit sortChanged(sort);
}

void Atoms::setBondScale(float bondScale)
{
    if (m_bondScale == bondScale)
        return;

    m_bondScale = bondScale;
    emit bondScaleChanged(bondScale);
}

void Atoms::setSphereScale(float sphereScale)
{
    if (m_sphereScale == sphereScale)
        return;

    m_sphereScale = sphereScale;
    emit sphereScaleChanged(sphereScale);
}

void Atoms::setRenderingMode(QString renderingMode)
{
    if (m_renderingMode == renderingMode)
        return;

    m_renderingMode = renderingMode;
    emit renderingModeChanged(renderingMode);
}
