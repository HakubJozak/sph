#pragma once
#ifndef FP_CPU_SPH_H
#define FP_CPU_SPH_H

#include "DXUT.h"
#pragma warning(disable:4995)
#include <vector>
#pragma warning(default:4995)

#include "fp_global.h"
#include "fp_thread.h"

typedef struct {
    D3DXVECTOR3 m_Position;
    D3DXVECTOR3 m_Velocity;
    int m_Index;
} fp_FluidParticle;

typedef struct {
    fp_FluidParticle* m_Particle1;
    fp_FluidParticle* m_Particle2;
    float m_DistanceSq;
} fp_FluidParticlePair;

typedef std::vector<fp_FluidParticle> fp_GridCell;
typedef fp_GridCell::size_type fp_GridCellSize;
typedef std::vector<fp_FluidParticlePair> fp_FluidParticlePairCache;

class fp_Grid {
public:
    std::vector<fp_GridCell*> m_Cells;
    int m_NumParticles;
    int m_NumCells;
    int m_NumCellsX;
    int m_NumCellsY;
    int m_NumCellsZ;
    int m_NumCellsYZ;
    float m_CellWidth;
    float m_MinX;
    float m_MaxX;
    float m_MinY;
    float m_MaxY;
    float m_MinZ;
    float m_MaxZ;

    fp_Grid(float CellWidth);
    // Copy constructor
    fp_Grid(const fp_Grid& Other);
    ~fp_Grid();

    void FillAndPrepare(fp_FluidParticle* Particles, int NumParticles);

private:
    inline void SetBounds(fp_FluidParticle* Particles, int NumParticles);
};

class fp_Fluid;

// Internal structure, don't use elsewhere
typedef struct {
    fp_Fluid* m_Fluid;
    int m_ThreadIdx;
    int m_NumThreads;
} fp_FluidMTHelperData;

class fp_Fluid {
    friend void fp_FluidUpdateDensitiesCachePairsMTWrapper(void*);
    friend void fp_FluidUpdateForcesMTWrapper(void*);    
    friend void fp_FluidGlassUpdateDensitiesMTWrapper(void*);
    friend void fp_FluidGlassUpdateForcesMTWrapper(void*);
    friend void fp_FluidMoveParticlesMTWrapper(void*);
    friend void fp_FluidDummyFunc(void*);

public:
    fp_FluidParticle* m_Particles;
    int m_NumParticles;
    float m_GasConstantK;
    float m_Viscosity;
    float m_SurfaceTension;
    float m_GradientColorFieldThresholdSq;
    float m_SmoothingLength;
    float m_SmoothingLengthSq;
    float m_SmoothingLengthSqInv;
    float m_SmoothingLengthPow3Inv;
    float m_VacuumDensity;
    float m_InitialDensity;
    float m_RestDensity;
    float m_InitialDensityCoefficient;
    float m_RestDensityCoefficient;
    float m_DampingCoefficient;
    float m_SearchRadius;
    float m_ParticleMass;
    float m_WPoly6Coefficient;
    float m_GradientWPoly6Coefficient;
    float m_LaplacianWPoly6Coefficient;
    float m_GradientWSpikyCoefficient;
    float m_LaplacianWViscosityCoefficient;

    float m_GlassDensity;
    float m_GlassViscosity;
    float m_GlassRadius;
    float m_GlassRadiusPlusEnforceDistance;
    float m_GlassRadiusPlusEnforceDistanceSq;
    float m_GlassFloor;
    float m_GlassFloorMinusEnforceDistance;    
    D3DXVECTOR3 m_CurrentGlassPosition;
    float m_CurrentGlassFloorY;
    float m_CurrentGlassEnforceMinY;
    D3DXVECTOR3 m_LastGlassPosition;
    D3DXVECTOR3 m_LastGlassVelocity;
    D3DXVECTOR3 m_GlassVelocityChange;
    
    D3DXVECTOR3 m_Gravity;

    fp_Fluid(
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
        D3DXVECTOR3 Gravity = FP_FLUID_DEFAULT_GRAVITY,
        float SmoothingLenght = FP_FLUID_DEFAULT_SMOOTHING_LENGTH,
        float GasConstantK = FP_FLUID_DEFAULT_GAS_CONSTANT_K,        
        float Viscosity = FP_FLUID_DEFAULT_VISCOSITY,
        float SurfaceTension = FP_FLUID_DEFAULT_SURFACE_TENSION,
        float GradientColorFieldThreshold = FP_FLUID_DEFAULT_GRADIENT_COLORFIELD_THRESHOLD,
        float ParticleMass = FP_FLUID_DEFAULT_PARTICLE_MASS,
        float InitialDensityCoefficient = FP_FLUID_DEFAULT_INITIAL_DENSITY_COEFFICIENT,
        float RestDensityCoefficient = FP_FLUID_DEFAULT_REST_DENSITY_COEFFICIENT,
        float DampingCoefficient = FP_FLUID_DEFAULT_DAMPING_COEFFICIENT,
        float GlassDensity = FP_FLUID_DEFAULT_GLASS_DENSITY,
        float GlassViscosity = FP_FLUID_DEFAULT_GLASS_VISCOSITY,
        float GlassEnforceDistance = FP_FLUID_DEFAULT_GLASS_ENFORCE_DISTANCE);
    ~fp_Fluid();
    void Update(float ElapsedTime);
    void SetGlassEnforceDistance(float GlassEnforceDistance);
    void SetSmoothingLength(float SmoothingLength);
    void SetParticleMass(float ParticleMass);
    float* GetDensities();
    void GetParticleMinsAndMaxs(
            float& MinX, 
            float& MaxX, 
            float& MinY, 
            float& MaxY, 
            float& MinZ, 
            float& MaxZ);

    inline float WPoly6(float HSq_LenRSq);
    inline D3DXVECTOR3 GradientWPoly6(const D3DXVECTOR3* R, float HSq_LenRSq);
    inline float LaplacianWPoly6(float LenRSq, float HSq_LenRSq);
    inline D3DXVECTOR3 GradientWSpiky(const D3DXVECTOR3* R, float LenR);
    inline float LaplacianWViscosity(float LenR);
private:
    // For multi threadding
    fp_WorkerThreadManager* m_WorkerThreadMgr;
    fp_FluidMTHelperData* m_MTData;
    float m_CurrentElapsedTime;
    
    fp_FluidParticlePairCache* m_PairCaches;

    fp_Grid* m_Grid;

    volatile float* m_DensitiesConcurrent;
    volatile D3DXVECTOR3* m_PressureAndViscosityForcesConcurrent;
    volatile D3DXVECTOR3* m_GradientColorFieldConcurrent;
    volatile float* m_LaplacianColorFieldConcurrent;

    float* m_Densities;
    D3DXVECTOR3* m_PressureAndViscosityForces;
    D3DXVECTOR3* m_GradientColorField;
    float* m_LaplacianColorField;

    void UpdateDensitiesCachePairsMT(int ThreadIdx);
    inline void UpdateDensitiesCachePair(
            fp_FluidParticle* p1,
            fp_FluidParticle* p2,
            int threadIdx);
    void GlassCommonUpdateMT(
            int ThreadIdx,
            void (fp_Fluid::*Func)(int, float, D3DXVECTOR3*, D3DXVECTOR3*));
    void GlassUpdateDensitiesMT(int ThreadIdx);
    void GlassUpdateDensityOnParticle(
            int ParticleIndex,
            float LenR,
            D3DXVECTOR3* R,
            D3DXVECTOR3* Velocity);
    void UpdateForcesMT(int ThreadIdx);
    inline void UpdateForcesOnPair(fp_FluidParticlePair* Pair);
    void GlassUpdateForcesMT(int ThreadIdx);
    void GlassUpdateForcesOnParticle(
            int ParticleIndex,
            float LenR,
            D3DXVECTOR3* R,
            D3DXVECTOR3* Velocity);
    void MoveParticlesMT(int ThreadIdx);
    inline void EnforceGlass(fp_FluidParticle* Particle);
};

// Inline definitions


inline float fp_Fluid::WPoly6(float HSq_LenRSq) {
	return m_WPoly6Coefficient * pow(HSq_LenRSq, 3);
}

inline D3DXVECTOR3 fp_Fluid::GradientWPoly6(const D3DXVECTOR3* R, float HSq_LenRSq) {
	return (*R) * m_GradientWPoly6Coefficient * pow(HSq_LenRSq, 2);
}

inline float fp_Fluid::LaplacianWPoly6(float LenRSq, float HSq_LenRSq) {
	   return m_LaplacianWPoly6Coefficient * HSq_LenRSq * (LenRSq - 0.75f * HSq_LenRSq);
}

inline D3DXVECTOR3 fp_Fluid::GradientWSpiky(const D3DXVECTOR3* R, float LenR) {
	return (*R) * (m_GradientWSpikyCoefficient / LenR) * pow(m_SmoothingLength - LenR, 2);
}


inline float fp_Fluid::LaplacianWViscosity(float LenR) {
	return m_LaplacianWViscosityCoefficient * (1.0f - LenR / m_SmoothingLength);
}

#endif
