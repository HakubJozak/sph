#include "DXUT.h"
#include "fp_cpu_sph.h"
#include "fp_util.h"
#include <float.h>

void fp_FluidUpdateDensitiesCachePairsMTWrapper(void* Data) {
    fp_FluidMTHelperData* mtData
            = (fp_FluidMTHelperData*) Data;
    mtData->m_Fluid->UpdateDensitiesCachePairsMT(mtData->m_ThreadIdx);
}

void fp_FluidUpdateForcesMTWrapper(void* Data) {
    fp_FluidMTHelperData* mtData
            = (fp_FluidMTHelperData*) Data;
    mtData->m_Fluid->UpdateForcesMT(mtData->m_ThreadIdx);
}

void fp_FluidGlassUpdateDensitiesMTWrapper(void* Data) {
    fp_FluidMTHelperData* mtData
        = (fp_FluidMTHelperData*) Data;
    mtData->m_Fluid->GlassUpdateDensitiesMT(mtData->m_ThreadIdx);
}

void fp_FluidGlassUpdateForcesMTWrapper(void* Data) {
    fp_FluidMTHelperData* mtData
        = (fp_FluidMTHelperData*) Data;
    mtData->m_Fluid->GlassUpdateForcesMT(mtData->m_ThreadIdx);
}


void fp_FluidMoveParticlesMTWrapper(void* Data) {
    fp_FluidMTHelperData* mtData
        = (fp_FluidMTHelperData*) Data;
    mtData->m_Fluid->MoveParticlesMT(mtData->m_ThreadIdx);
}

void fp_FluidDummyFunc(void* Data) {
    ;
}

fp_Grid::fp_Grid(float CellWidth)
        :
        m_Cells(),
        m_NumParticles(0),
        m_NumCells(0),
        m_NumCellsX(0),
        m_NumCellsY(0),
        m_NumCellsZ(0),
        m_CellWidth(CellWidth),
        m_MinX(FLT_MAX),
        m_MinY(FLT_MAX),
        m_MinZ(FLT_MAX),
        m_MaxX(FLT_MIN),
        m_MaxY(FLT_MIN),
        m_MaxZ(FLT_MIN) {
    m_Cells.reserve(FP_FLUID_INITIAL_GRID_CAPACITY);
}

fp_Grid::fp_Grid(const fp_Grid& Other)
        :
        m_NumParticles(Other.m_NumParticles),
        m_NumCells(Other.m_NumCells),
        m_NumCellsX(Other.m_NumCellsX),
        m_NumCellsY(Other.m_NumCellsY),
        m_NumCellsZ(Other.m_NumCellsZ),
        m_CellWidth(Other.m_CellWidth),
        m_MinX(Other.m_MinX),
        m_MinY(Other.m_MinY),
        m_MinZ(Other.m_MinZ),
        m_MaxX(Other.m_MaxX),
        m_MaxY(Other.m_MaxY),
        m_MaxZ(Other.m_MaxZ) {
    for (int i = 0; i < m_NumCells; i++) {
        fp_GridCell* otherCell = Other.m_Cells[i];
        if(otherCell != NULL) {
            m_Cells[i] = new fp_GridCell(*otherCell);
        }
    }
}

fp_Grid::~fp_Grid() {
    for (int i = 0; i < m_NumCells; i++) {
        if(m_Cells[i] != NULL) {
            delete m_Cells[i];
            m_Cells[i] = NULL;
        }
    }
}

void fp_Grid::FillAndPrepare(fp_FluidParticle *Particles, int NumParticles) {
    for (int i = 0; i < m_NumCells; i++) {
        if(m_Cells[i] != NULL) {
            delete m_Cells[i];
            m_Cells[i] = NULL;
        }
    }

    m_NumParticles = NumParticles;
    SetBounds(Particles, NumParticles);    
    m_NumCellsX = (int)((m_MaxX - m_MinX + m_CellWidth) / m_CellWidth);
    m_NumCellsY = (int)((m_MaxY - m_MinY + m_CellWidth) / m_CellWidth);
    m_NumCellsZ = (int)((m_MaxZ - m_MinZ + m_CellWidth) / m_CellWidth);
    m_NumCellsYZ = m_NumCellsY * m_NumCellsZ;
    m_NumCells = m_NumCellsX * m_NumCellsYZ;
    m_Cells.resize(m_NumCells, NULL);

    for (int i = 0; i < NumParticles; i++) {
        D3DXVECTOR3 pos = Particles[i].m_Position;
        int cellIndexX = (int)((pos.x - m_MinX) / m_CellWidth);
        assert(cellIndexX >= 0 && cellIndexX < m_NumCellsX);
        int cellIndexY = (int)((pos.y - m_MinY) / m_CellWidth);
        assert(cellIndexY >= 0 && cellIndexY < m_NumCellsY);
        int cellIndexZ = (int)((pos.z - m_MinZ) / m_CellWidth);
        assert(cellIndexZ >= 0 && cellIndexZ < m_NumCellsZ);
        int cellIndex = cellIndexX * m_NumCellsYZ + cellIndexY * m_NumCellsZ
                + cellIndexZ;
        fp_GridCell* cell = m_Cells[cellIndex];
        if(cell == NULL) {
            m_Cells[cellIndex] = cell = new fp_GridCell();
            cell->reserve(FP_FLUID_GRID_INITIAL_CELL_CAPACITY);
        }
        cell->push_back(Particles[i]);
    }
}

inline void fp_Grid::SetBounds(fp_FluidParticle *Particles, int NumParticles){
    m_MinX = m_MinY = m_MinZ = FLT_MAX;
    m_MaxX = m_MaxY = m_MaxZ = FLT_MIN;    
    for (int i = 0; i < NumParticles; i++)
    {
        D3DXVECTOR3 pos = Particles[i].m_Position;
        if(pos.x < m_MinX)
            m_MinX = pos.x;
        if(pos.y < m_MinY)
            m_MinY = pos.y;
        if(pos.z < m_MinZ)
            m_MinZ = pos.z;
        if(pos.x > m_MaxX)
            m_MaxX = pos.x;
        if(pos.y > m_MaxY)
            m_MaxY = pos.y;
        if(pos.z > m_MaxZ)
            m_MaxZ = pos.z;
    }
}

fp_Fluid::fp_Fluid(
        fp_WorkerThreadManager* WorkerThreadMgr,
        int NumParticlesX,
        int NumParticlesY,
        int NumParticlesZ,
        float SpacingX,
        float SpacingY,
        float SpacingZ,
        D3DXVECTOR3 Center,        
        float GlassRadius,
        float GlassFloor,        
        D3DXVECTOR3 Gravity,
        float SmoothingLenght,
        float GasConstantK,
        float Viscosity,
        float SurfaceTension,
        float GradientColorFieldThreshold,
        float ParticleMass,
        float InitialDensityCoefficient,
        float RestDensityCoefficient,
        float DampingCoefficient,
        float GlassDensity,
        float GlassViscosity,
        float GlassEnforceDistance)
        :
        m_NumParticles(NumParticlesX * NumParticlesY * NumParticlesZ),        
        m_Particles(new fp_FluidParticle[NumParticlesX * NumParticlesY * NumParticlesZ]),
        m_CurrentGlassPosition(Center),
        m_LastGlassPosition(Center),
        m_LastGlassVelocity(0.0f, 0.0f, 0.0f),
        m_GlassVelocityChange(0.0f, 0.0f, 0.0f),
        m_GlassRadius(GlassRadius),
        m_GlassFloor(GlassFloor),
        m_Gravity(Gravity),
        m_GasConstantK(GasConstantK),
        m_Viscosity(Viscosity),
        m_SurfaceTension(SurfaceTension),
        m_InitialDensityCoefficient(InitialDensityCoefficient),
        m_RestDensityCoefficient(RestDensityCoefficient),
        m_GradientColorFieldThresholdSq(GradientColorFieldThreshold
                * GradientColorFieldThreshold),
        m_DampingCoefficient(DampingCoefficient),
        m_GlassDensity(GlassDensity),
        m_GlassViscosity(GlassViscosity){
    m_WorkerThreadMgr = WorkerThreadMgr;
    int numWorkerThreads = m_WorkerThreadMgr->m_NumWorkerThreads;
    m_MTData = new fp_FluidMTHelperData[numWorkerThreads];
    m_PairCaches = new fp_FluidParticlePairCache[numWorkerThreads];
    for(int iWorker=0; iWorker < numWorkerThreads; iWorker++) {
        m_MTData[iWorker].m_Fluid = this;
        m_MTData[iWorker].m_ThreadIdx = iWorker;
        m_PairCaches[iWorker].reserve(FP_FLUID_INITIAL_PAIR_CACHE_CAPACITY);
    }

    SetGlassEnforceDistance(GlassEnforceDistance);
    m_ParticleMass = ParticleMass; // Needed in SetSmoothingLength(...)
    m_Grid = NULL;
    SetSmoothingLength(SmoothingLenght);
    SetParticleMass(ParticleMass);
    m_Grid = new fp_Grid(SmoothingLenght);
    m_PressureAndViscosityForces = new D3DXVECTOR3[m_NumParticles];    
    m_GradientColorField = new D3DXVECTOR3[m_NumParticles];    
    m_LaplacianColorField = new float[m_NumParticles];    
    m_Densities = new float[m_NumParticles];
    m_PressureAndViscosityForcesConcurrent = m_PressureAndViscosityForces;
    m_GradientColorFieldConcurrent = m_GradientColorField;
    m_LaplacianColorFieldConcurrent = m_LaplacianColorField;
    m_DensitiesConcurrent = m_Densities;
    float startX = Center.x - 0.5f * (NumParticlesX - 1) * SpacingX;
    float startY = Center.y - 0.5f * (NumParticlesY - 1) * SpacingY;
    float startZ = Center.z - 0.5f * (NumParticlesZ - 1) * SpacingZ;     

    int i = 0;
    for(int iZ = 0; iZ < NumParticlesZ; iZ++ ) {
        for (int iY = 0; iY < NumParticlesY; iY++) {
            for (int iX = 0; iX < NumParticlesX; iX++) {                
                m_Particles[i].m_Position = D3DXVECTOR3(iX * SpacingX + startX,
                        iY * SpacingY + startY, iZ * SpacingZ + startZ);
                m_Particles[i].m_Velocity = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
                m_Particles[i].m_Index = i;
                ((D3DXVECTOR3*)m_PressureAndViscosityForces)[i]
                        = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
                ((D3DXVECTOR3*)m_GradientColorField)[i] = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
                m_LaplacianColorField[i] = 0.0f;
                m_Densities[i] = m_InitialDensity;
                i++;
            }
        }
    }  
    m_Grid->FillAndPrepare(m_Particles, m_NumParticles);
}

fp_Fluid::~fp_Fluid() {
    m_WorkerThreadMgr->DoJobOnAllThreads(fp_FluidDummyFunc, NULL, 0);
    m_WorkerThreadMgr->WaitTillJobFinishedOnAllThreads();
    delete[] m_Particles;
    delete[] m_MTData;
    delete[] m_PairCaches;
    delete m_Grid;
}

void fp_Fluid::SetGlassEnforceDistance(float GlassEnforceDistance) {
    float glassEnforceDistanceSq = GlassEnforceDistance * GlassEnforceDistance;
    m_GlassRadiusPlusEnforceDistance = m_GlassRadius + GlassEnforceDistance;
    m_GlassRadiusPlusEnforceDistanceSq = m_GlassRadiusPlusEnforceDistance
        * m_GlassRadiusPlusEnforceDistance ;
    m_GlassFloorMinusEnforceDistance = m_GlassFloor - GlassEnforceDistance;
}

void fp_Fluid::SetSmoothingLength(float SmoothingLength) {
    m_SmoothingLength = SmoothingLength;
    m_SearchRadius = SmoothingLength;
    m_SmoothingLengthPow3Inv = 1.0f / pow(SmoothingLength, 3);
    m_SmoothingLengthSq = SmoothingLength * SmoothingLength;
    m_SmoothingLengthSqInv = 1.0f / m_SmoothingLengthSq;
    m_WPoly6Coefficient = 315.0f / (64.0f * D3DX_PI * pow(SmoothingLength, 9));
    m_GradientWPoly6Coefficient = -945.0f / (32.0f * D3DX_PI * pow(SmoothingLength,9));
    m_LaplacianWPoly6Coefficient = 945.0f / (8.0f * D3DX_PI * pow(SmoothingLength,9));
    m_GradientWSpikyCoefficient = -45.0f / (D3DX_PI * pow(SmoothingLength, 6));
    m_LaplacianWViscosityCoefficient = 45.0f / (D3DX_PI * pow(SmoothingLength, 5));
    m_VacuumDensity = m_ParticleMass * WPoly6(m_SmoothingLengthSq);
    m_InitialDensity = m_InitialDensityCoefficient * m_VacuumDensity;
    m_RestDensity = m_RestDensityCoefficient * m_VacuumDensity;
    if(m_Grid != NULL) {
        m_Grid->m_CellWidth = SmoothingLength;
    }
}

void fp_Fluid::SetParticleMass(float ParticleMass) {
    m_ParticleMass = ParticleMass;
    m_VacuumDensity = ParticleMass * WPoly6(m_SmoothingLengthSq);
    m_RestDensity = m_RestDensityCoefficient * m_VacuumDensity;
}

float* fp_Fluid::GetDensities() {
    return m_Densities;
}

void fp_Fluid::GetParticleMinsAndMaxs(
        float& MinX, 
        float& MaxX, 
        float& MinY, 
        float& MaxY, 
        float& MinZ, 
        float& MaxZ) {
    MinX = m_Grid->m_MinX;
    MaxX = m_Grid->m_MaxX;    
    MinY = m_Grid->m_MinY;
    MaxY = m_Grid->m_MaxY;    
    MinZ = m_Grid->m_MinZ;
    MaxZ = m_Grid->m_MaxZ;        
}

void fp_Fluid::Update(float ElapsedTime) {
    m_CurrentElapsedTime = ElapsedTime;

    // For glass collision:
    D3DXVECTOR3 glassVelocity = (m_CurrentGlassPosition - m_LastGlassPosition)
            / ElapsedTime;
    m_GlassVelocityChange = glassVelocity - m_LastGlassVelocity;
    m_LastGlassVelocity = glassVelocity;
    m_LastGlassPosition = m_CurrentGlassPosition;
    m_CurrentGlassFloorY = m_GlassFloor + m_CurrentGlassPosition.y;

    // Update densities (particle-particle)
    m_WorkerThreadMgr->DoJobOnAllThreads(fp_FluidUpdateDensitiesCachePairsMTWrapper,
            m_MTData, sizeof(fp_FluidMTHelperData));
    m_WorkerThreadMgr->WaitTillJobFinishedOnAllThreads();

    // Update densities (particle-glass)
    m_WorkerThreadMgr->DoJobOnAllThreads(
            fp_FluidGlassUpdateDensitiesMTWrapper, m_MTData,
            sizeof(fp_FluidMTHelperData));
    m_WorkerThreadMgr->WaitTillJobFinishedOnAllThreads();

    // Update pressure and viscosity forces, etc. (particle-particle)
    m_WorkerThreadMgr->DoJobOnAllThreads(fp_FluidUpdateForcesMTWrapper, m_MTData,
            sizeof(fp_FluidMTHelperData));
    m_WorkerThreadMgr->WaitTillJobFinishedOnAllThreads();    

    // Update pressure and viscosity forces, etc. (particle-glass)
    m_WorkerThreadMgr->DoJobOnAllThreads(fp_FluidGlassUpdateForcesMTWrapper, m_MTData,
            sizeof(fp_FluidMTHelperData));
    m_WorkerThreadMgr->WaitTillJobFinishedOnAllThreads();

    // Move particles and clear fields    
    m_CurrentGlassEnforceMinY = m_GlassFloorMinusEnforceDistance
            + m_CurrentGlassPosition.y;
    m_WorkerThreadMgr->DoJobOnAllThreads(fp_FluidMoveParticlesMTWrapper, m_MTData,
        sizeof(fp_FluidMTHelperData));
    m_WorkerThreadMgr->WaitTillJobFinishedOnAllThreads();

    // Update Grid
    m_Grid->FillAndPrepare(m_Particles, m_NumParticles);
}

void fp_Fluid::UpdateDensitiesCachePairsMT(int ThreadIdx) {
    int NumCellsX = m_Grid->m_NumCellsX, NumCellsY = m_Grid->m_NumCellsY, NumCellsZ
            = m_Grid->m_NumCellsZ, NumCellsYZ = NumCellsY * NumCellsZ;

    // For each cell
    for (int cellIndexX = 0; cellIndexX < NumCellsX; cellIndexX++) {
        for (int cellIndexY = 0; cellIndexY < NumCellsY; cellIndexY++) {
            for (int cellIndexZ = 0; cellIndexZ < NumCellsZ; cellIndexZ++) {                    
                int cellIndex = cellIndexX * NumCellsYZ + cellIndexY * NumCellsZ
                    + cellIndexZ;

                // For MT:
                if(cellIndex % m_WorkerThreadMgr->m_NumWorkerThreads != ThreadIdx)
                    continue;

                fp_GridCell* cell = m_Grid->m_Cells[cellIndex];
                if(cell == NULL)
                    continue;
                fp_GridCellSize cellSize = cell->size();

                // All pairs in current cell
                
                // For each particle in current cell
                for (fp_GridCellSize iCellParticle = 0; iCellParticle < cellSize;
                        iCellParticle++) {
                    fp_FluidParticle* particle = &(*cell)[iCellParticle];

                    // For each following cell neighbor
                    for(fp_GridCellSize iCellNeighbor = iCellParticle + 1;
                            iCellNeighbor < cellSize; iCellNeighbor++) {
                        fp_FluidParticle* neighborParticle = &(*cell)[iCellNeighbor];
                        UpdateDensitiesCachePair(particle, neighborParticle, ThreadIdx);
                    }
                }

                // All pairs with neighbor cells

                // For half of the neighbor cells
                int xN = 0, yN = 0, zN = 0;
                int neighborCellIndexX = cellIndexX,
                        neighborCellIndexY = cellIndexY,
                        neighborCellIndexZ = cellIndexZ;
                bool xOutside = false, yOutside = false, zOutside = false;
                while (true) {                    
                    if(zN < 1) {
                        zN++;
                        if(++neighborCellIndexZ >= NumCellsZ)
                            zOutside = true;
                        else
                            zOutside = false;
                    } else {
                        zN = -1;
                        if((neighborCellIndexZ = cellIndexZ - 1) < 0)
                            zOutside = true;
                        else
                            zOutside = false;
                        if(yN < 1) {
                            yN++;
                            if(++neighborCellIndexY >= NumCellsY)
                                yOutside = true;
                            else
                                yOutside = false;
                        } else {
                            yN = -1;
                            if((neighborCellIndexY = cellIndexY - 1) < 0)
                                yOutside = true;
                            else
                                yOutside = false;
                            if(xN < 1) {
                                xN++;
                                if(++neighborCellIndexX >= NumCellsX)
                                    xOutside = true;
                                else
                                    xOutside = false;
                            } else
                                break;
                        }
                    }

                    if(xOutside || yOutside || zOutside)
                        continue;

                    int neighborCellIndex = neighborCellIndexX * NumCellsYZ
                            + neighborCellIndexY * NumCellsZ
                            + neighborCellIndexZ;
                    fp_GridCell* neighborCell
                        = m_Grid->m_Cells[neighborCellIndex];
                    if(neighborCell == NULL)
                        continue;
                    fp_GridCellSize neighborCellSize = neighborCell->size();

                    // Pairwise process all particles from current cell with all
                    // particles from neighbor cell

                    // For each particle in current cell
                    for (fp_GridCellSize iCellParticle = 0; iCellParticle < cellSize;
                            iCellParticle++) {
                        fp_FluidParticle* particle = &(*cell)[iCellParticle];

                        // For each particle in neighbor cell
                        for (fp_GridCellSize iNeighbourCellParticle = 0;
                                iNeighbourCellParticle < neighborCellSize;
                                iNeighbourCellParticle++) {
                            fp_FluidParticle* neighborParticle =
                                    &(*neighborCell)[iNeighbourCellParticle];
                            UpdateDensitiesCachePair(particle, neighborParticle,
                                    ThreadIdx);
                        }                                                        
                    }                     
                }
            }
        }        
    }
}

inline void fp_Fluid::UpdateDensitiesCachePair(
        fp_FluidParticle* particle1,
        fp_FluidParticle* particle2,
        int threadIdx){
    D3DXVECTOR3 r = particle2->m_Position
            - particle1->m_Position;
    float distanceSq = D3DXVec3LengthSq(&r);
    assert(distanceSq > 0.0f);
    if(distanceSq < m_SmoothingLengthSq) {                                                                                    
        fp_FluidParticlePair pair;
        pair.m_Particle1 = particle1;
        pair.m_Particle2 = particle2;
        pair.m_DistanceSq = distanceSq;
        m_PairCaches[threadIdx].push_back(pair);

        int particle1Index = particle1->m_Index;
        int particle2Index = particle2->m_Index;
        float additionalDensity = m_ParticleMass * WPoly6(m_SmoothingLengthSq
                - distanceSq);
        m_DensitiesConcurrent[particle1Index] += additionalDensity;
        m_DensitiesConcurrent[particle2Index] += additionalDensity;
    }
}

void fp_Fluid::GlassCommonUpdateMT(
        int ThreadIdx,
        void (fp_Fluid::*Func)(int, float, D3DXVECTOR3*, D3DXVECTOR3*)) {
    int numParticlesPerThread = (int)ceil((double)m_NumParticles
        / m_WorkerThreadMgr->m_NumWorkerThreads);
    int startIndex = ThreadIdx * numParticlesPerThread;
    int endIndex = startIndex + numParticlesPerThread;
    if(endIndex > m_NumParticles) endIndex = m_NumParticles;
    for (int i = startIndex; i < endIndex; i++) {
        fp_FluidParticle* Particle = &m_Particles[i];
        D3DXVECTOR3 particlePosition = Particle->m_Position;
        int particleIndex = Particle->m_Index;

        // Handle Collision with floor 
        float lenR = particlePosition.y - m_CurrentGlassFloorY;
        if(lenR < FP_FLUID_GLASS_PUSHBACK_DISTANCE)
            lenR = FP_FLUID_GLASS_PUSHBACK_DISTANCE;
        if(lenR < m_SmoothingLength) {
            D3DXVECTOR3 r = D3DXVECTOR3(0.0f, lenR, 0.0f);
            (this->*Func)(particleIndex, lenR, &r, &Particle->m_Velocity);
        }

        // Handle collision with side
        D3DXVECTOR3 particleToCenter = m_CurrentGlassPosition - particlePosition;
        particleToCenter.y = 0.0f;
        float particleToCenterLen = D3DXVec3Length(&particleToCenter);
        float particleToCenterLenInv = 1.0f / particleToCenterLen;
        lenR = m_GlassRadius - particleToCenterLen;  
        if(lenR < FP_FLUID_GLASS_PUSHBACK_DISTANCE) {
            lenR = FP_FLUID_GLASS_PUSHBACK_DISTANCE;
            particleToCenterLen = m_GlassRadius - FP_FLUID_GLASS_PUSHBACK_DISTANCE;
            particleToCenter *= particleToCenterLenInv * particleToCenterLen;
            particleToCenterLenInv = 1.0f / particleToCenterLen;
        }
        if(lenR < m_SmoothingLength) {
            D3DXVECTOR3 r = particleToCenter * particleToCenterLenInv * lenR;
            (this->*Func)(particleIndex, lenR, &r,&Particle->m_Velocity);
        }
    }
}

void fp_Fluid::GlassUpdateDensitiesMT(int ThreadIdx) {
    GlassCommonUpdateMT(ThreadIdx, &fp_Fluid::GlassUpdateDensityOnParticle);
}

void fp_Fluid::GlassUpdateDensityOnParticle(
        int ParticleIndex,
        float LenR,
        D3DXVECTOR3* R,
        D3DXVECTOR3* Velocity) {
    float lenRSq = LenR * LenR;
    float hSq_lenRSq = m_SmoothingLengthSq - lenRSq;
    float wPoly6Value = WPoly6(hSq_lenRSq);
    float additionalDensity = m_ParticleMass * wPoly6Value;
    m_Densities[ParticleIndex] += additionalDensity * m_GlassDensity;
}

void fp_Fluid::UpdateForcesMT(int ThreadIdx) {
    fp_FluidParticlePairCache::iterator it = m_PairCaches[ThreadIdx].begin();
    fp_FluidParticlePairCache::iterator end = m_PairCaches[ThreadIdx].end();
    for(; it != end; it++) {
        UpdateForcesOnPair(&(*it));
    }
    m_PairCaches[ThreadIdx].clear();
}

inline void fp_Fluid::UpdateForcesOnPair(fp_FluidParticlePair* Pair){
    float dist = sqrt(Pair->m_DistanceSq);
    float hSq_lenRSq = m_SmoothingLengthSq - Pair->m_DistanceSq;
    D3DXVECTOR3 r1 = Pair->m_Particle1->m_Position - Pair->m_Particle2->m_Position;
    int particle1Index = Pair->m_Particle1->m_Index;
    int particle2Index = Pair->m_Particle2->m_Index;
    float particle1Density = m_Densities[particle1Index];
    float particle2Density = m_Densities[particle2Index];

    // Pressure forces
    float pressureAt1 = m_GasConstantK * (particle1Density - m_RestDensity);
    float pressureAt2 = m_GasConstantK * (particle2Density - m_RestDensity);
    float pressureSum = pressureAt1 + pressureAt2;
    D3DXVECTOR3 gradWSpikyValue1 = GradientWSpiky(&r1, dist);
    D3DXVECTOR3 commonPressureTerm1 = - m_ParticleMass * pressureSum / 2.0f
            * gradWSpikyValue1;
    D3DXVECTOR3 pressureForce1 = commonPressureTerm1 / particle2Density;
    D3DXVECTOR3 pressureForce2 = - commonPressureTerm1 / particle1Density;
    
    #if defined(DEBUG) || defined(_DEBUG)

    float pressureForce1LenSq = D3DXVec3LengthSq(&pressureForce1);
    D3DXVECTOR3 pressureForce1Normalized, r1Normalized;
    D3DXVec3Normalize(&pressureForce1Normalized, &pressureForce1);
    D3DXVec3Normalize(&r1Normalized, &r1);
    //float testDot = D3DXVec3Dot(&pressureForce1Normalized, &r1Normalized);
    //assert(pressureForce1LenSq < 0.1f || pressureForce1LenSq > -0.1f
    //        || testDot > 0.99f);
    assert(m_Densities[particle1Index] >= m_VacuumDensity);
    assert(pressureForce1LenSq < FP_DEBUG_MAX_FORCE_SQ);

    #endif

    // Viscosity forces
    D3DXVECTOR3 velocityDifference1 = Pair->m_Particle2->m_Velocity
            - Pair->m_Particle1->m_Velocity;
    float laplacianWViskosityValue1 = LaplacianWViscosity(dist);
    D3DXVECTOR3 commonViscosityTerm1 = m_ParticleMass * m_Viscosity * velocityDifference1
            * laplacianWViskosityValue1;
    D3DXVECTOR3 viscosityForce1 = commonViscosityTerm1 / particle2Density;
    D3DXVECTOR3 viscosityForce2 = -commonViscosityTerm1 / particle1Density;

    #if defined(DEBUG) || defined(_DEBUG)
    float viscosityForce1LenSq = D3DXVec3LengthSq(&viscosityForce1);
    assert(viscosityForce1LenSq < FP_DEBUG_MAX_FORCE_SQ);
    #endif

    // Surface tension
    D3DXVECTOR3 gradWPoly6Value1 = GradientWPoly6(&r1, hSq_lenRSq);
    D3DXVECTOR3 commonGradientColorFieldTerm1 = m_ParticleMass * gradWPoly6Value1;
    ((D3DXVECTOR3*)m_GradientColorField)[particle1Index]
            += commonGradientColorFieldTerm1 / particle2Density;
    ((D3DXVECTOR3*)m_GradientColorField)[particle2Index]
            -= commonGradientColorFieldTerm1 / particle1Density;
    float laplacianWPoly6Value1 = LaplacianWPoly6(Pair->m_DistanceSq, hSq_lenRSq);
    float commonLaplacianColorFieldTerm1 = m_ParticleMass * laplacianWPoly6Value1;
    m_LaplacianColorFieldConcurrent[particle1Index] += commonLaplacianColorFieldTerm1
            / particle2Density;
    m_LaplacianColorFieldConcurrent[particle2Index] += commonLaplacianColorFieldTerm1
            / particle1Density;

    // Total forces
    ((D3DXVECTOR3*)m_PressureAndViscosityForcesConcurrent)[particle1Index]
            += pressureForce1 + viscosityForce1;
    ((D3DXVECTOR3*)m_PressureAndViscosityForcesConcurrent)[particle2Index]
            += pressureForce2 + viscosityForce2;
}

void fp_Fluid::GlassUpdateForcesMT(int ThreadIdx) {
    GlassCommonUpdateMT(ThreadIdx, &fp_Fluid::GlassUpdateForcesOnParticle);
}

void fp_Fluid::GlassUpdateForcesOnParticle(
        int ParticleIndex,
        float LenR,
        D3DXVECTOR3* R,
        D3DXVECTOR3* Velocity) {
    float lenRSq = LenR * LenR;
    float particleDensity = m_Densities[ParticleIndex];
    
    // Pressure force
    float pressure = m_GasConstantK * (particleDensity - m_RestDensity);
    // glass should always push the particles away, so exclude negative-pressure
    // conditions
    if(pressure < FP_FLUID_GLASS_MIN_PRESSURE)
        pressure = FP_FLUID_GLASS_MIN_PRESSURE;
    D3DXVECTOR3 gradWSpikyValue = GradientWSpiky(R, LenR);
    D3DXVECTOR3 pressureTerm = - m_ParticleMass * pressure * gradWSpikyValue;
    D3DXVECTOR3 pressureForce = pressureTerm / particleDensity * m_GlassDensity;

    // Viscosity force
    D3DXVECTOR3 velocityDifference = m_LastGlassVelocity - *Velocity;
    float laplacianWViskosityValue = LaplacianWViscosity(LenR);
    D3DXVECTOR3 viscosityTerm = m_ParticleMass * m_Viscosity
            * velocityDifference * laplacianWViskosityValue;
    D3DXVECTOR3 viscosityForce = viscosityTerm / particleDensity
            * m_GlassViscosity;

    ((D3DXVECTOR3*)m_PressureAndViscosityForces)[ParticleIndex] += (pressureForce
        + viscosityForce);
}

void fp_Fluid::MoveParticlesMT(int ThreadIdx) {
    // Move particles and clear fields
    int numParticlesPerThread = (int)ceil((double)m_NumParticles
            / m_WorkerThreadMgr->m_NumWorkerThreads);
    int startIndex = ThreadIdx * numParticlesPerThread;
    int endIndex = startIndex + numParticlesPerThread;
    if(endIndex > m_NumParticles) endIndex = m_NumParticles;
    for (int i = startIndex; i < endIndex; i++) {
        D3DXVECTOR3 oldVelocity = m_Particles[i].m_Velocity;
        D3DXVECTOR3 oldVelocityContribution = oldVelocity
            * pow(m_DampingCoefficient, m_CurrentElapsedTime);
        D3DXVECTOR3 totalForce = ((D3DXVECTOR3*)m_PressureAndViscosityForces)[i];
        D3DXVECTOR3 gradColorField = ((D3DXVECTOR3*)m_GradientColorField)[i];
        float gradColorFieldLenSq = D3DXVec3LengthSq(&gradColorField);
        if(gradColorFieldLenSq >= m_GradientColorFieldThresholdSq) {
            D3DXVECTOR3 surfaceTensionForce = (-m_SurfaceTension *
                m_LaplacianColorField[i]/sqrt(gradColorFieldLenSq)) * gradColorField;
            totalForce += surfaceTensionForce;
        }
        D3DXVECTOR3 newVelocity = oldVelocityContribution 
            + (totalForce / m_Densities[i] + m_Gravity) * m_CurrentElapsedTime;        
        m_Particles[i].m_Velocity = newVelocity;
        m_Particles[i].m_Position += 0.5f * m_CurrentElapsedTime
            * (oldVelocityContribution + newVelocity);        

        EnforceGlass(&m_Particles[i]);

        ((D3DXVECTOR3*)m_PressureAndViscosityForces)[i] = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
        ((D3DXVECTOR3*)m_GradientColorField)[i] = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
        m_LaplacianColorField[i] = 0.0f;
        m_Densities[i] = m_InitialDensity;
    }
}

inline void fp_Fluid::EnforceGlass(fp_FluidParticle* Particle) {
    D3DXVECTOR3 particlePosition = Particle->m_Position;

    // Use "manual" model to enforce the particles to stay in the glass
    
    // Handle Collision with floor    
    if(particlePosition.y < m_CurrentGlassEnforceMinY) {
        // Position particle on floor
        Particle->m_Position.y = m_CurrentGlassEnforceMinY;
        // Invert it's velocity along y-Axis
        Particle->m_Velocity.y = -Particle->m_Velocity.y + m_GlassVelocityChange.y;
    }

    // Handle collision with side
    D3DXVECTOR3 particleToCenter = m_CurrentGlassPosition - particlePosition;
    particleToCenter.y = 0.0f;
    float particleToCenterLenSq = D3DXVec3LengthSq(&particleToCenter);
    if(particleToCenterLenSq > m_GlassRadiusPlusEnforceDistanceSq) {
        // Position particle on side
        float particleToCenterLen = sqrt(particleToCenterLenSq);
        float scale = (particleToCenterLen - m_GlassRadiusPlusEnforceDistance)
                / particleToCenterLen;
        Particle->m_Position += scale * particleToCenter;
        // Invert it's velocity along the sides normal
        D3DXVECTOR3 normal;
        D3DXVec3Normalize(&normal, &particleToCenter);
        // R = -2 * (N*V) * N + V
        Particle->m_Velocity += -2.0f * D3DXVec3Dot(&normal, &Particle->m_Velocity)
                * normal;
        Particle->m_Velocity.x += m_GlassVelocityChange.x * abs(normal.x);
        Particle->m_Velocity.z += m_GlassVelocityChange.z * abs(normal.z);
    }
}
