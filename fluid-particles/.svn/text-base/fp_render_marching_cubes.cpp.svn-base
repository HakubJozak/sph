#include "DXUT.h"
//#include "SDKmisc.h"
#include "fp_render_marching_cubes.h"
#include "fp_util.h"

#define FP_RENDER_MARCHING_CUBES_EFFECT_FILE L"fp_render_marching_cubes.fx" 

const D3D10_INPUT_ELEMENT_DESC fp_MCVertex::Layout[] = {
    { "POSITION",  0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0,
            D3D10_INPUT_PER_VERTEX_DATA, 0 },
    { "NORMAL",    0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12,
            D3D10_INPUT_PER_VERTEX_DATA, 0 }, };

fp_CPUDensityGrid::fp_CPUDensityGrid(fp_Fluid* Fluid, float VoxelSize, float DensityGridBorder) 
        :
        m_Fluid(Fluid),
        m_LastMinX(0.0f),
        m_LastMinY(0.0f),
        m_LastMinZ(0.0f),
        m_LastMaxX(0.0f),
        m_LastMaxY(0.0f),
        m_LastMaxZ(0.0f),
        m_NumValues(0),
        m_NumValuesYZ(0),
        m_NumValuesX(0),
        m_NumValuesY(0),
        m_NumValuesZ(0),        
        m_DensityValues(),
        m_GradientDensityValues(),
        m_StampRowLengths(NULL),
        m_StampRowStartOffsets(NULL),
        m_StampRowValueStarts(NULL),
        m_StampValues(NULL),
        m_GradientStampValues(NULL),
        m_DensityGridBorder(DensityGridBorder) {
    m_VoxelSize = -1.0f; // Needed in SetSmoothingLength()
    UpdateSmoothingLength();
    SetVoxelSize(VoxelSize);
    m_DensityValues.reserve(FP_MC_INITIAL_DENSITY_GRID_CAPACITY);
    m_GradientDensityValues.reserve(FP_MC_INITIAL_DENSITY_GRID_CAPACITY);
}

void fp_CPUDensityGrid::UpdateSmoothingLength() {
    if(m_VoxelSize > 0.0f) {
        if(m_StampValues != NULL)
            DestroyStamp();
        CreateStamp();
    }
}

void fp_CPUDensityGrid::SetVoxelSize(float VoxelSize) {
    m_VoxelSize = VoxelSize;
    m_HalfVoxelSize = VoxelSize / 2.0f;
    if(m_StampValues != NULL)
        DestroyStamp();
    CreateStamp();
}

void fp_CPUDensityGrid::ConstructFromFluid() {
    float minX = m_LastMinX, minY = m_LastMinY, minZ = m_LastMinZ;
    float maxX = m_LastMaxX, maxY = m_LastMaxY, maxZ = m_LastMaxZ;

    float partMinX, partMaxX, partMinY, partMaxY, partMinZ, partMaxZ;
    m_Fluid->GetParticleMinsAndMaxs(partMinX, partMaxX, partMinY, partMaxY, partMinZ,
            partMaxZ);
    partMinX -= m_DensityGridBorder;
    partMinY -= m_DensityGridBorder;
    partMinZ -= m_DensityGridBorder;
    partMaxX += m_DensityGridBorder;
    partMaxY += m_DensityGridBorder;
    partMaxZ += m_DensityGridBorder;    
    
    if(partMinX < m_LastMinX) // Grow?
        minX = partMinX - (partMaxX - partMinX) * FP_MC_DENSITY_GRID_GROW_FACTOR;
    else if (partMinX > m_LastMinX + 0.5 * (m_LastMaxX - m_LastMinX)
            * FP_MC_DENSITY_GRID_SHRINK_BORDER) // Shrink?
        minX = partMinX - (partMaxX - partMinX) * FP_MC_DENSITY_GRID_GROW_FACTOR;

    if(partMinY < m_LastMinY) // Grow?
        minY = partMinY - (partMaxY - partMinY) * FP_MC_DENSITY_GRID_GROW_FACTOR;
    else if (partMinY > m_LastMinY + 0.5 * (m_LastMaxY - m_LastMinY)
            * FP_MC_DENSITY_GRID_SHRINK_BORDER) // Shrink?
        minY = partMinY - (partMaxY - partMinY) * FP_MC_DENSITY_GRID_GROW_FACTOR;

    if(partMinZ < m_LastMinZ) // Grow?
        minZ = partMinZ - (partMaxZ - partMinZ) * FP_MC_DENSITY_GRID_GROW_FACTOR;
    else if (partMinZ > m_LastMinZ + 0.5 * (m_LastMaxZ - m_LastMinZ)
            * FP_MC_DENSITY_GRID_SHRINK_BORDER) // Shrink?
        minZ = partMinZ - (partMaxZ - partMinZ) * FP_MC_DENSITY_GRID_GROW_FACTOR;

    if(partMaxX > m_LastMaxX) // Grow?
        maxX = partMaxX + (partMaxX - partMinX) * FP_MC_DENSITY_GRID_GROW_FACTOR;
    else if (partMaxX < m_LastMaxX - 0.5 * (m_LastMaxX - m_LastMinX)
            * FP_MC_DENSITY_GRID_SHRINK_BORDER) // Shrink?
        maxX = partMaxX + (partMaxX - partMinX) * FP_MC_DENSITY_GRID_GROW_FACTOR;

    if(partMaxY > m_LastMaxY) // Grow?
        maxY = partMaxY + (partMaxY - partMinY) * FP_MC_DENSITY_GRID_GROW_FACTOR;
    else if (partMaxY < m_LastMaxY - 0.5 * (m_LastMaxY - m_LastMinY)
            * FP_MC_DENSITY_GRID_SHRINK_BORDER) // Shrink?
        maxY = partMaxY + (partMaxY - partMinY) * FP_MC_DENSITY_GRID_GROW_FACTOR;

    if(partMaxZ > m_LastMaxZ) // Grow?
        maxZ = partMaxZ + (partMaxZ - partMinZ) * FP_MC_DENSITY_GRID_GROW_FACTOR;
    else if (partMaxZ < m_LastMaxZ - 0.5 * (m_LastMaxZ - m_LastMinZ)
            * FP_MC_DENSITY_GRID_SHRINK_BORDER) // Shrink?
        maxZ = partMaxZ + (partMaxZ - partMinZ) * FP_MC_DENSITY_GRID_GROW_FACTOR;

    if((maxX - minX) > FP_MC_MAX_DENSITY_GRID_SIDELENGTH)
        maxX = minX + FP_MC_MAX_DENSITY_GRID_SIDELENGTH;
    if((maxY - minY) > FP_MC_MAX_DENSITY_GRID_SIDELENGTH)
        maxY = minY + FP_MC_MAX_DENSITY_GRID_SIDELENGTH;
    if((maxZ - minZ) > FP_MC_MAX_DENSITY_GRID_SIDELENGTH)
        maxZ = minZ + FP_MC_MAX_DENSITY_GRID_SIDELENGTH;

    m_LastMinX = minX;
    m_LastMinY = minY;
    m_LastMinZ = minZ;
    m_LastMaxX = maxX;
    m_LastMaxY = maxY;
    m_LastMaxZ = maxZ;

    m_VolumeStart = D3DXVECTOR3(minX + m_HalfVoxelSize, minY + m_HalfVoxelSize, minZ + m_HalfVoxelSize);
    m_VolumeCellOffset = D3DXVECTOR3(m_VoxelSize, m_VoxelSize, m_VoxelSize);
    m_NumValuesX = (int)((maxX - minX + m_VoxelSize) / m_VoxelSize);
    m_NumValuesY = (int)((maxY - minY + m_VoxelSize) / m_VoxelSize);
    m_NumValuesZ = (int)((maxZ - minZ + m_VoxelSize) / m_VoxelSize);
    m_NumValuesYZ = m_NumValuesY * m_NumValuesZ;
    m_NumValues = m_NumValuesYZ * m_NumValuesX;
    m_DensityValues.clear();
    m_DensityValues.resize(m_NumValues, 0.0f);
    m_GradientDensityValues.clear();
    m_GradientDensityValues.resize(m_NumValues, D3DXVECTOR3(0.0f, 0.0f, 0.0f));
    int NumParticles = m_Fluid->m_NumParticles;
    fp_FluidParticle* Particles = m_Fluid->m_Particles;
    float* Densities = m_Fluid->GetDensities();
    float ParticleMass = m_Fluid->m_ParticleMass;
    for (int i = 0; i < NumParticles; i++) {
        D3DXVECTOR3 particlePosition = Particles[i].m_Position;
        float particleDensity = Densities[i];
        DistributeParticleWithStamp(particlePosition, ParticleMass / particleDensity, minX, minY,
                minZ);
    }
}

inline void fp_CPUDensityGrid::DistributeParticle(
        D3DXVECTOR3 ParticlePosition,
        float ParticleMassDensityQuotient,
        float MinX,
        float MinY,
        float MinZ) {    
    float smoothingLength = m_Fluid->m_SmoothingLength;
    float smoothingLengthSq = m_Fluid->m_SmoothingLengthSq;

    fp_VolumeIndex destinationStart;
    destinationStart.x = (int)((ParticlePosition.x - MinX - smoothingLength) / m_VoxelSize);
    destinationStart.y = (int)((ParticlePosition.y - MinY - smoothingLength) / m_VoxelSize);
    destinationStart.z = (int)((ParticlePosition.z - MinZ - smoothingLength) / m_VoxelSize);
    if (destinationStart.x < 0) destinationStart.x = 0;
    if (destinationStart.y < 0) destinationStart.y = 0;
    if (destinationStart.z < 0) destinationStart.z = 0;

    fp_VolumeIndex destinationEnd;
    destinationEnd.x = (int)((ParticlePosition.x - MinX + smoothingLength) / m_VoxelSize);
    destinationEnd.y = (int)((ParticlePosition.y - MinY + smoothingLength) / m_VoxelSize);
    destinationEnd.z = (int)((ParticlePosition.z - MinZ + smoothingLength) / m_VoxelSize);
    if (destinationEnd.x >= m_NumValuesX) destinationEnd.x = m_NumValuesX - 1;
    if (destinationEnd.y >= m_NumValuesY) destinationEnd.y = m_NumValuesY - 1;
    if (destinationEnd.z >= m_NumValuesZ) destinationEnd.z = m_NumValuesZ - 1;

    D3DXVECTOR3 startPos(MinX + m_HalfVoxelSize + destinationStart.x * m_VoxelSize,
            MinY + m_HalfVoxelSize + destinationStart.y * m_VoxelSize,
            MinZ + m_HalfVoxelSize + destinationStart.z * m_VoxelSize);   

    int destinationIndexXY;
    fp_VolumeIndex destination;
    D3DXVECTOR3 cellPos = startPos;
    for(destination.x = destinationStart.x; destination.x <= destinationEnd.x; 
            destination.x++) {
        cellPos.y = startPos.y;
        destinationIndexXY = destination.x * m_NumValuesYZ + destinationStart.y
                * m_NumValuesZ;
        for(destination.y = destinationStart.y; destination.y <= destinationEnd.y;
                destination.y++) {
            cellPos.z = startPos.z;            
            for(destination.z = destinationStart.z; destination.z <= destinationEnd.z;
                    destination.z++) {
                D3DXVECTOR3 particleToCell = cellPos - ParticlePosition;
                float particleToCellLenSq = D3DXVec3LengthSq(&particleToCell);
                if(particleToCellLenSq < smoothingLengthSq) {
                    int destinationIndex = destinationIndexXY + destination.z;
                    //int destinationIndex = destination.x * m_NumValuesYZ + destination.y * m_NumValuesZ
                    //        + destination.z;
                    float additionalDensityValue = ParticleMassDensityQuotient
                            * m_Fluid->WPoly6(particleToCellLenSq);
                    m_DensityValues[destinationIndex] += additionalDensityValue;
                    D3DXVECTOR3 additionalGradientDensityValue = ParticleMassDensityQuotient
                            * m_Fluid->GradientWPoly6(&particleToCell, 
                            particleToCellLenSq);
                    m_GradientDensityValues[destinationIndex] += additionalGradientDensityValue;
                }
                cellPos.z += m_VoxelSize;
            }
            destinationIndexXY += m_NumValuesZ;
            cellPos.y += m_VoxelSize;
        }
        cellPos.x += m_VoxelSize;
    }
}

inline void fp_CPUDensityGrid::DistributeParticleWithStamp(
        D3DXVECTOR3 ParticlePosition,
        float ParticleMassDensityQuotient,
        float MinX,
        float MinY,
        float MinZ) {
    fp_VolumeIndex particleVolumeIndex;
    particleVolumeIndex.x = (int)((ParticlePosition.x - MinX) / m_VoxelSize);
    particleVolumeIndex.y = (int)((ParticlePosition.y - MinY) / m_VoxelSize);
    particleVolumeIndex.z = (int)((ParticlePosition.z - MinZ) / m_VoxelSize);

    for (int stampRowIndex = 0; stampRowIndex < m_NumStampRows; stampRowIndex++) {
        fp_VolumeIndex destStart = particleVolumeIndex
                + m_StampRowStartOffsets[stampRowIndex];
        if(destStart.x < 0 || destStart.x >= m_NumValuesX
                || destStart.y < 0 || destStart.y >= m_NumValuesY)
            continue;

        int destIndexXY = m_NumValuesYZ * destStart.x
            + m_NumValuesZ * destStart.y;
        int destIndex = destStart.z < 0 ? destIndexXY : destIndexXY + destStart.z;
        int zEnd = destStart.z + m_StampRowLengths[stampRowIndex];
        if(zEnd > m_NumValuesZ)
            zEnd = m_NumValuesZ;
        int destEndIndex = destIndexXY + zEnd;

        int stampValueIndex = m_StampRowValueStarts[stampRowIndex];

        for(; destIndex < destEndIndex; destIndex++) {
            float additionalDensityValue = ParticleMassDensityQuotient 
                    * m_StampValues[stampValueIndex];
            D3DXVECTOR3 additionalGradientDensityValue = ParticleMassDensityQuotient
                    * m_GradientStampValues[stampValueIndex++];
            m_DensityValues[destIndex] += additionalDensityValue;
            m_GradientDensityValues[destIndex] += additionalGradientDensityValue;
        }
    }
}

void fp_CPUDensityGrid::CreateStamp() {
    int stampRadius = (int)ceil((m_Fluid->m_SmoothingLength - m_HalfVoxelSize) 
            / m_VoxelSize);
    int stampSideLength = 1 + 2 * stampRadius;
    int stampSideLengthSq = stampSideLength * stampSideLength;    
    
    float halfLength = stampSideLength * m_HalfVoxelSize;
    D3DXVECTOR3 mid(halfLength, halfLength, halfLength);
    
    float* cubeStampDistancesSq = new float[stampSideLength * stampSideLength
            * stampSideLength];
    D3DXVECTOR3* cubeStampR = new D3DXVECTOR3[stampSideLength * stampSideLength
            * stampSideLength];
    fp_VolumeIndex* cubeStampRowStarts = new fp_VolumeIndex[stampSideLengthSq];
    int* cubeStampRowLengths = new int[stampSideLengthSq];
    m_NumStampValues = 0;
    m_NumStampRows = 0;

    for (int stampX = 0; stampX < stampSideLength; stampX++) {
        for (int stampY = 0; stampY < stampSideLength; stampY++) {
            int stampIndexXY = stampX * stampSideLengthSq + stampY * stampSideLength;
            int cubeStampRowIndex = stampX * stampSideLength + stampY;
            cubeStampRowLengths[cubeStampRowIndex] = 0;
            for (int stampZ = 0; stampZ < stampSideLength; stampZ++) {
                D3DXVECTOR3 coord(
                        stampX * m_VoxelSize + m_HalfVoxelSize,
                        stampY * m_VoxelSize + m_HalfVoxelSize,
                        stampZ * m_VoxelSize + m_HalfVoxelSize);
                D3DXVECTOR3 midToCoord = coord - mid;
                float distSq = D3DXVec3LengthSq(&midToCoord);
                if(distSq < m_Fluid->m_SmoothingLengthSq) {
                    int stampIndex = stampIndexXY + stampZ;
                    cubeStampDistancesSq[stampIndex] = distSq;
                    cubeStampR[stampIndex] = midToCoord;
                    if(cubeStampRowLengths[cubeStampRowIndex] == 0) {
                        m_NumStampRows++;
                        fp_VolumeIndex rowStart;
                        rowStart.x = stampX;
                        rowStart.y = stampY;
                        rowStart.z = stampZ;
                        cubeStampRowStarts[cubeStampRowIndex] = rowStart;
                    }
                    m_NumStampValues++;
                    cubeStampRowLengths[cubeStampRowIndex]++;
                } else {
                    cubeStampDistancesSq[stampIndexXY + stampZ] = -1.0f;
                    cubeStampR[stampIndexXY + stampZ] = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
                }
            }
        }
    }
    m_StampRowLengths = new int[m_NumStampRows];
    m_StampRowStartOffsets = new fp_VolumeIndex[m_NumStampRows];
    m_StampRowValueStarts = new int[m_NumStampRows];
    m_StampValues = new float[m_NumStampValues];
    m_GradientStampValues = new D3DXVECTOR3[m_NumStampValues];
    int stampRowIndex = 0;
    int stampValueIndex = 0;
    for (int cubeRow = 0; cubeRow < stampSideLengthSq; cubeRow++) {
        int rowLength = cubeStampRowLengths[cubeRow];
        if(rowLength == 0)
            continue;
        m_StampRowLengths[stampRowIndex] = rowLength;
        fp_VolumeIndex cubeStampVolumeIndex = cubeStampRowStarts[cubeRow];
        fp_VolumeIndex rowStartOffset;
        rowStartOffset.x = cubeStampVolumeIndex.x - stampRadius;
        rowStartOffset.y = cubeStampVolumeIndex.y - stampRadius;
        rowStartOffset.z = cubeStampVolumeIndex.z - stampRadius;
        m_StampRowStartOffsets[stampRowIndex] = rowStartOffset;
        m_StampRowValueStarts[stampRowIndex] = stampValueIndex;
        int cubeStampIndex = cubeStampVolumeIndex.x * stampSideLengthSq
                + cubeStampVolumeIndex.y * stampSideLength
                + stampRadius - (rowLength - 1) / 2;
        int cubeStampEnd = cubeStampIndex + rowLength;
        for (; cubeStampIndex < cubeStampEnd; cubeStampIndex++) {            
            m_StampValues[stampValueIndex]
                    = m_Fluid->WPoly6(cubeStampDistancesSq[cubeStampIndex]);
            m_GradientStampValues[stampValueIndex]
                    = m_Fluid->GradientWPoly6(&cubeStampR[cubeStampIndex],
                    cubeStampDistancesSq[cubeStampIndex]);
            stampValueIndex++;
        }
        stampRowIndex++;
    }
}

void fp_CPUDensityGrid::DestroyStamp() {
    delete[] m_StampRowLengths;
    delete[] m_StampRowStartOffsets;
    delete[] m_StampRowValueStarts;
    delete[] m_StampValues;
    delete[] m_GradientStampValues;
}

fp_RenderMarchingCubes::fp_RenderMarchingCubes(
        fp_CPUDensityGrid* DensityGrid, 
        int NumLights, 
        float IsoLevel)
        :
        m_DensityGrid(DensityGrid),
        m_IsoLevel(IsoLevel),
        m_NumLights(NumLights),
        m_NumTriangles(0),
        m_NumVertices(0),
		m_VertexBuffer9(NULL),
        m_IndexBuffer9(NULL),
        m_VertexBuffer10(NULL),
        m_IndexBuffer10(NULL),
        m_Effect10(NULL),
        m_TechRenderMarchingCubes1Light(NULL),
        m_TechRenderMarchingCubes2Lights(NULL),
        m_TechRenderMarchingCubes3Lights(NULL),
        m_EffectVarLightDir(NULL),
        m_EffectVarLightDiffuse(NULL),
        m_EffectVarViewProjection(NULL),
        m_EffectVarMaterialDiffuseColor(NULL),
        m_EffectVarMaterialAmbientColor(NULL),
        m_VertexLayout(NULL) {    
    D3DCOLORVALUE diffuse  = {0.5f, 0.6f, 1.0, 1.0f};
    D3DCOLORVALUE ambient  = {0.05f, 0.05f, 0.05f, 1.0f};
    D3DCOLORVALUE emmisive = {0.0f, 0.0f, 0.0f, 1.0f};
    D3DCOLORVALUE specular = {0.5f, 0.5f, 0.5f, 1.0f};
    ZeroMemory( &m_Material9, sizeof(D3DMATERIAL9) );
    m_Material9.Diffuse = diffuse;
    m_Material9.Ambient = ambient;
    m_Material9.Emissive = emmisive;
    m_Material9.Specular = specular;
    m_Lights9 = new D3DLIGHT9[NumLights];
}

fp_RenderMarchingCubes::~fp_RenderMarchingCubes() {
	if(DXUTIsAppRenderingWithD3D9()){
		OnD3D9LostDevice(NULL);
		OnD3D9DestroyDevice(NULL);
	}
	if(DXUTIsAppRenderingWithD3D10()){
		OnD3D10ReleasingSwapChain(NULL);
		OnD3D10DestroyDevice(NULL);
	}
}

void fp_RenderMarchingCubes::ConstructMesh() {
    bool isRenderingWithD3D10 = DXUTIsAppRenderingWithD3D10();

    if(!isRenderingWithD3D10 && m_VertexBuffer9 == NULL) // TODO: catch this special case
        return; 

    m_NumVertices = 0;
    int indexIndex = 0;
    m_NumTriangles = 0;

    // Build cube
    int NumValuesX = m_DensityGrid->m_NumValuesX;
    int NumValuesY = m_DensityGrid->m_NumValuesY;
    int NumValuesZ = m_DensityGrid->m_NumValuesZ;
    int NumValuesYZ = m_DensityGrid->m_NumValuesYZ;
    int xEnd = NumValuesX - 1;
    int yEnd = NumValuesY - 1;
    int zEnd = NumValuesZ - 1;
    D3DXVECTOR3 volumeStart = m_DensityGrid->m_VolumeStart;
    float voxelSize = m_DensityGrid->m_VoxelSize;
    D3DXVECTOR3 volumeOffset = D3DXVECTOR3(voxelSize, voxelSize, voxelSize);
    std::vector<float>* densityValues = &m_DensityGrid->m_DensityValues;
    std::vector<D3DXVECTOR3>* gradientDensityValues = &m_DensityGrid->m_GradientDensityValues;

    fp_MCVertex *mcVertices;
    unsigned int* mcIndizes;

    if(isRenderingWithD3D10) {
        m_VertexBuffer10->Map(D3D10_MAP_WRITE_DISCARD, 0, (void**)&mcVertices);
        m_IndexBuffer10->Map(D3D10_MAP_WRITE_DISCARD, 0, (void**)&mcIndizes);
    } else {
        m_VertexBuffer9->Lock( 0, FP_MC_MAX_VETICES * sizeof(fp_MCVertex),
            (void**)&mcVertices, D3DLOCK_DISCARD );    
        m_IndexBuffer9->Lock(0, FP_MC_MAX_TRIANGLES * 3, (void**)&mcIndizes, D3DLOCK_DISCARD);
    }

    for(int cubeX=0; cubeX < xEnd; cubeX++) {
        for(int cubeY=0; cubeY < yEnd; cubeY++) {
            int densityGridIndex4 = cubeX * NumValuesYZ + cubeY * NumValuesZ; // x,y,z
            int densityGridIndex5 = densityGridIndex4 + NumValuesYZ; // x+1,y,z
            int densityGridIndex6 = densityGridIndex5 + NumValuesZ; // x+1,y+1,z
            int densityGridIndex7 = densityGridIndex4 + NumValuesZ; // x,y+1,z
            float densityValue0, densityValue1, densityValue2, densityValue3;
            float densityValue4 = (*densityValues)[densityGridIndex4];            
            float densityValue5 = (*densityValues)[densityGridIndex5];
            float densityValue6 = (*densityValues)[densityGridIndex6];
            float densityValue7 = (*densityValues)[densityGridIndex7];
            D3DXVECTOR3 *gradientDensityValue0, *gradientDensityValue1, 
                *gradientDensityValue2, *gradientDensityValue3;
            D3DXVECTOR3* gradientDensityValue4 = &(*gradientDensityValues)[densityGridIndex4++];
            D3DXVECTOR3* gradientDensityValue5 = &(*gradientDensityValues)[densityGridIndex5++];
            D3DXVECTOR3* gradientDensityValue6 = &(*gradientDensityValues)[densityGridIndex6++];
            D3DXVECTOR3* gradientDensityValue7 = &(*gradientDensityValues)[densityGridIndex7++];
            int vertexIndizes[12] = {-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1};
            D3DXVECTOR3 corner0(volumeStart + D3DXVECTOR3(volumeOffset.x * cubeX, 
                volumeOffset.y * cubeY, 0.0f ));
            for(int cubeZ=0; cubeZ < zEnd; cubeZ++) {                
                densityValue0 = densityValue4;
                densityValue1 = densityValue5;
                densityValue2 = densityValue6;
                densityValue3 = densityValue7;
                gradientDensityValue0 = gradientDensityValue4;
                gradientDensityValue1 = gradientDensityValue5;
                gradientDensityValue2 = gradientDensityValue6;
                gradientDensityValue3 = gradientDensityValue7;

                densityValue4 = (*densityValues)[densityGridIndex4];
                densityValue5 = (*densityValues)[densityGridIndex5];
                densityValue6 = (*densityValues)[densityGridIndex6];
                densityValue7 = (*densityValues)[densityGridIndex7];
                gradientDensityValue4 = &(*gradientDensityValues)[densityGridIndex4++];
                gradientDensityValue5 = &(*gradientDensityValues)[densityGridIndex5++];
                gradientDensityValue6 = &(*gradientDensityValues)[densityGridIndex6++];
                gradientDensityValue7 = &(*gradientDensityValues)[densityGridIndex7++];

                byte cubeType=0;
                if(densityValue0 < m_IsoLevel) cubeType |= 1;
                if(densityValue1 < m_IsoLevel) cubeType |= 2;
                if(densityValue2 < m_IsoLevel) cubeType |= 4;
                if(densityValue3 < m_IsoLevel) cubeType |= 8;
                if(densityValue4 < m_IsoLevel) cubeType |= 16;
                if(densityValue5 < m_IsoLevel) cubeType |= 32;
                if(densityValue6 < m_IsoLevel) cubeType |= 64;
                if(densityValue7 < m_IsoLevel) cubeType |= 128;

                int edgeValue = s_EdgeTable[cubeType];
                if(edgeValue == 0) {
                    corner0.z += volumeOffset.z;
                    continue;
                }

                // Vertices:                
                vertexIndizes[0] = vertexIndizes[4];
                vertexIndizes[1] = vertexIndizes[5];
                vertexIndizes[2] = vertexIndizes[6];
                vertexIndizes[3] = vertexIndizes[7];

                assert(!((edgeValue & 1) && (vertexIndizes[0] == -1)));
                assert(!((edgeValue & 2) && (vertexIndizes[1] == -1)));
                assert(!((edgeValue & 4) && (vertexIndizes[2] == -1)));
                assert(!((edgeValue & 8) && (vertexIndizes[3] == -1)));

                float s;

                if(edgeValue & 16) // Edge 4 between corner 4 and 5
                    mcVertices[vertexIndizes[4] = m_NumVertices++] = fp_MCVertex(
                    D3DXVECTOR3(corner0.x + (s = (m_IsoLevel - densityValue4)
                    / (densityValue5 - densityValue4)) * volumeOffset.x, corner0.y,
                    corner0.z + volumeOffset.z), CalcNormal(gradientDensityValue4,
                    gradientDensityValue5, s));
                if(edgeValue & 32) // Edge 5 between corner 5 and 6
                    mcVertices[vertexIndizes[5] = m_NumVertices++] = fp_MCVertex(
                    D3DXVECTOR3(corner0.x + volumeOffset.x, corner0.y + (s
                    = (m_IsoLevel - densityValue5) / (densityValue6 - densityValue5))
                    * volumeOffset.y, corner0.z + volumeOffset.z), CalcNormal(
                    gradientDensityValue5, gradientDensityValue6, s));
                if(edgeValue & 64) // Edge 6 between corner 7 and 6
                    mcVertices[vertexIndizes[6] = m_NumVertices++] = fp_MCVertex(
                    D3DXVECTOR3(corner0.x + (s = (m_IsoLevel - densityValue7)
                    / (densityValue6 - densityValue7)) * volumeOffset.x, corner0.y 
                    + volumeOffset.y, corner0.z+ volumeOffset.z), CalcNormal(
                    gradientDensityValue7, gradientDensityValue6, s));
                if(edgeValue & 128) // Edge 7 between corner 4 and 7
                    mcVertices[vertexIndizes[7] = m_NumVertices++] = fp_MCVertex(
                    D3DXVECTOR3(corner0.x, corner0.y + (s = (m_IsoLevel -
                    densityValue4) / (densityValue7 - densityValue4)) * volumeOffset.y,
                    corner0.z + volumeOffset.z), CalcNormal(gradientDensityValue4,
                    gradientDensityValue7, s));
                if(edgeValue & 256) // Edge 8 between corner 0 and 4
                    mcVertices[vertexIndizes[8] = m_NumVertices++] = fp_MCVertex(
                    D3DXVECTOR3(corner0.x, corner0.y, corner0.z + (s
                    = (m_IsoLevel - densityValue0) / (densityValue4 - densityValue0))
                    * volumeOffset.z), CalcNormal(gradientDensityValue0,
                    gradientDensityValue4, s));
                if(edgeValue & 512) // Edge 9 between corner 1 and 5
                    mcVertices[vertexIndizes[9] = m_NumVertices++] = fp_MCVertex(
                    D3DXVECTOR3(corner0.x + volumeOffset.x, corner0.y, corner0.z
                    + (s = (m_IsoLevel - densityValue1) / (densityValue5 - densityValue1))
                    * volumeOffset.z), CalcNormal(gradientDensityValue1, 
                    gradientDensityValue5, s));
                if(edgeValue & 1024) // Edge 10 between corner 2 and 6
                    mcVertices[vertexIndizes[10] = m_NumVertices++] = fp_MCVertex(
                    D3DXVECTOR3(corner0.x + volumeOffset.x, corner0.y
                    + volumeOffset.y, corner0.z + (s = (m_IsoLevel - densityValue2)
                    / (densityValue6 - densityValue2)) * volumeOffset.z), CalcNormal(
                    gradientDensityValue2, gradientDensityValue6, s));
                if(edgeValue & 2048) // Edge 11 between corner 3 and 7
                    mcVertices[vertexIndizes[11] = m_NumVertices++] = fp_MCVertex(
                    D3DXVECTOR3(corner0.x, corner0.y + volumeOffset.y, corner0.z
                    + (s = (m_IsoLevel - densityValue3) / (densityValue7 - densityValue3))
                    * volumeOffset.z), CalcNormal(gradientDensityValue3,
                    gradientDensityValue7, s));

                // Triangles:
                int* triTableEntry = s_TriTable[cubeType];
                for(int i=0; triTableEntry[i] != -1; ) {
                    mcIndizes[indexIndex++] = vertexIndizes[triTableEntry[i++]];
                    mcIndizes[indexIndex++] = vertexIndizes[triTableEntry[i++]];
                    mcIndizes[indexIndex++] = vertexIndizes[triTableEntry[i++]];
                }

                corner0.z += volumeOffset.z;
            }     
        }        
    }
    m_NumTriangles = indexIndex / 3;

    if(isRenderingWithD3D10){
        m_IndexBuffer10->Unmap();
        m_VertexBuffer10->Unmap();
    } else {
        m_IndexBuffer9->Unlock();
        m_VertexBuffer9->Unlock();
    }
}


HRESULT fp_RenderMarchingCubes::OnD3D9CreateDevice(
        IDirect3DDevice9* D3DDevice,
        const D3DSURFACE_DESC* BackBufferSurfaceDesc,
        void* UserContext ) {
    //HRESULT hr;

    return S_OK;
}

HRESULT fp_RenderMarchingCubes::OnD3D9ResetDevice(
        IDirect3DDevice9* D3DDevice,
        const D3DSURFACE_DESC* BackBufferSurfaceDesc,
        void* UserContext ) {
    D3DDevice->CreateVertexBuffer( FP_MC_MAX_VETICES * sizeof(fp_MCVertex), 
            D3DUSAGE_DYNAMIC | D3DUSAGE_WRITEONLY, 
            fp_MCVertex::FVF_Flags, D3DPOOL_DEFAULT, &m_VertexBuffer9, NULL );
    D3DDevice->CreateIndexBuffer(FP_MC_MAX_TRIANGLES * 3 * sizeof(int),
            D3DUSAGE_DYNAMIC | D3DUSAGE_WRITEONLY,
            D3DFMT_INDEX32, D3DPOOL_DEFAULT, &m_IndexBuffer9, NULL );
    return S_OK;
}

void fp_RenderMarchingCubes::OnD3D9FrameRender(IDirect3DDevice9* D3DDevice) {  
    D3DDevice->SetStreamSource(0, m_VertexBuffer9, 0, sizeof(fp_MCVertex));
    D3DDevice->SetIndices(m_IndexBuffer9);
	D3DDevice->SetFVF(fp_MCVertex::FVF_Flags);
    D3DDevice->SetMaterial(&m_Material9);
    int i;
    for (i=0; i < m_NumActiveLights; i++) {        
        D3DDevice->SetLight(i, &m_Lights9[i]);
        D3DDevice->LightEnable(i, TRUE);
    }
    D3DDevice->LightEnable(i, FALSE);
    D3DDevice->SetRenderState(D3DRS_LIGHTING, TRUE);

    D3DDevice->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, 0, m_NumVertices, 0, m_NumTriangles);

    D3DDevice->SetRenderState(D3DRS_LIGHTING, FALSE);
    D3DDevice->LightEnable(0, FALSE);
}

void fp_RenderMarchingCubes::OnD3D9DestroyDevice(void* UserContext) {

}

void fp_RenderMarchingCubes::OnD3D9LostDevice(void* UserContext) {     
    SAFE_RELEASE(m_VertexBuffer9);    
    SAFE_RELEASE(m_IndexBuffer9);
}




HRESULT fp_RenderMarchingCubes::OnD3D10CreateDevice(
        ID3D10Device* D3DDevice,
        const DXGI_SURFACE_DESC* BackBufferSurfaceDesc,
        void* UserContext ) {
    HRESULT hr;

    // Read the D3DX effect file
    m_Effect10 = fp_Util::LoadEffect(D3DDevice, FP_RENDER_MARCHING_CUBES_EFFECT_FILE);

    // Obtain technique objects
    m_TechRenderMarchingCubes1Light = m_Effect10->GetTechniqueByName(
        "RenderMarchingCubes1Light" );
    m_TechRenderMarchingCubes2Lights = m_Effect10->GetTechniqueByName(
        "RenderMarchingCubes2Lights" );
    m_TechRenderMarchingCubes3Lights = m_Effect10->GetTechniqueByName(
        "RenderMarchingCubes3Lights" );

    // Obtain effect variables
    m_EffectVarLightDir = m_Effect10->GetVariableByName( "g_LightDir" )->AsVector();
    m_EffectVarLightDiffuse = m_Effect10->GetVariableByName( "g_LightDiffuse" )
        ->AsVector();
    m_EffectVarLightAmbient = m_Effect10->GetVariableByName(
        "g_LightAmbient" )->AsVector();
    m_EffectVarViewProjection = m_Effect10->GetVariableByName(
        "g_ViewProjection" )->AsMatrix();
    m_EffectVarMaterialAmbientColor = m_Effect10->GetVariableByName(
        "g_MaterialAmbientColor")->AsVector();
    m_EffectVarMaterialDiffuseColor = m_Effect10->GetVariableByName(
        "g_MaterialDiffuseColor" )->AsVector();

    // Set effect variables as needed
    V_RETURN( m_EffectVarMaterialAmbientColor->SetFloatVector(
        (float*)&m_Material9.Ambient) );
    V_RETURN( m_EffectVarMaterialDiffuseColor->SetFloatVector(
        (float*)&m_Material9.Diffuse) );

    // Create vertex buffer
    D3D10_BUFFER_DESC bufferDesc;
    bufferDesc.Usage = D3D10_USAGE_DYNAMIC;
    bufferDesc.ByteWidth = FP_MC_MAX_VETICES * sizeof(fp_MCVertex);
    bufferDesc.BindFlags = D3D10_BIND_VERTEX_BUFFER;
    bufferDesc.CPUAccessFlags = D3D10_CPU_ACCESS_WRITE;
    bufferDesc.MiscFlags = 0;
    V_RETURN(D3DDevice->CreateBuffer(&bufferDesc, NULL, &m_VertexBuffer10));

    D3D10_PASS_DESC passDesc;
    V_RETURN( m_TechRenderMarchingCubes1Light->GetPassByIndex(0)->GetDesc(&passDesc));
    V_RETURN( D3DDevice->CreateInputLayout(fp_MCVertex::Layout, 2,
            passDesc.pIAInputSignature, passDesc.IAInputSignatureSize,
            &m_VertexLayout ) );

    // Create index buffer
    bufferDesc.ByteWidth = FP_MC_MAX_TRIANGLES * 3 * sizeof(int);
    bufferDesc.BindFlags = D3D10_BIND_INDEX_BUFFER;
    V_RETURN(D3DDevice->CreateBuffer(&bufferDesc, NULL, &m_IndexBuffer10));

    return S_OK;
}

HRESULT fp_RenderMarchingCubes::OnD3D10ResizedSwapChain(
        ID3D10Device* D3DDevice,
        IDXGISwapChain *SwapChain,
        const DXGI_SURFACE_DESC* BackBufferSurfaceDesc,
        void* UserContext ) {
    return S_OK;
}

void fp_RenderMarchingCubes::OnD3D10FrameRender(
        ID3D10Device* D3DDevice,
        const D3DXMATRIX*  ViewProjection) {  
    HRESULT hr;

    D3DXVECTOR4* lightsDir = new D3DXVECTOR4[m_NumLights];
    D3DXVECTOR4* lightsDiffuse = new D3DXVECTOR4[m_NumLights];
    D3DXVECTOR4 lightsAmbient;
    for(int i=0; i<m_NumLights; i++) {
        lightsDir[i] = -D3DXVECTOR4(m_Lights9[i].Direction, 0.0f);
        lightsDiffuse[i] = D3DXVECTOR4((float*)&m_Lights9[i].Diffuse);
        if(i < m_NumActiveLights)
            lightsAmbient += D3DXVECTOR4((float*)&m_Lights9[i].Ambient);
    }
    V( m_EffectVarLightDir->SetFloatVectorArray((float*)lightsDir, 0, m_NumLights) );
    V( m_EffectVarLightDiffuse->SetFloatVectorArray((float*)lightsDiffuse, 0, 
            m_NumLights));
    V( m_EffectVarLightAmbient->SetFloatVector((float*)&lightsAmbient));
    V( m_EffectVarViewProjection->SetMatrix((float*)ViewProjection ));
    delete[] lightsDir;
    delete[] lightsDiffuse;

    // Render the scene with this technique as defined in the .fx file
    ID3D10EffectTechnique *renderTechnique;
    switch( m_NumActiveLights ) {
    case 1: renderTechnique = m_TechRenderMarchingCubes1Light;
        break;
    case 2: renderTechnique = m_TechRenderMarchingCubes2Lights;
        break;
    case 3: renderTechnique = m_TechRenderMarchingCubes3Lights;
        break;
    default: renderTechnique = m_TechRenderMarchingCubes1Light;
        break;
    }

    //IA setup
    D3DDevice->IASetInputLayout(m_VertexLayout);
    UINT stride = sizeof(fp_MCVertex);
    UINT offset = 0;
    D3DDevice->IASetVertexBuffers(0, 1, &m_VertexBuffer10, &stride, &offset);
    D3DDevice->IASetIndexBuffer(m_IndexBuffer10, DXGI_FORMAT_R32_UINT, 0);

    //Render
    D3DDevice->IASetPrimitiveTopology(D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    D3D10_TECHNIQUE_DESC techDesc;
    renderTechnique->GetDesc( &techDesc );
    for( UINT iPass = 0; iPass < techDesc.Passes; ++iPass ) {
        renderTechnique->GetPassByIndex( iPass )->Apply(0);
        D3DDevice->DrawIndexed(m_NumTriangles * 3, 0, 0);
    }
}

void fp_RenderMarchingCubes::OnD3D10DestroyDevice( void* UserContext ) {
    SAFE_RELEASE(m_VertexBuffer10);    
    SAFE_RELEASE(m_IndexBuffer10);
    SAFE_RELEASE(m_Effect10);
    SAFE_RELEASE(m_VertexLayout);
}

void fp_RenderMarchingCubes::OnD3D10ReleasingSwapChain( void* UserContext ) {
    ;
}

inline D3DXVECTOR3 fp_RenderMarchingCubes::CalcNormal(
        const D3DXVECTOR3* gradient1, 
        const D3DXVECTOR3* gradient2, 
        float s) {
    D3DXVECTOR3 result;
    D3DXVec3Lerp(&result, gradient1, gradient2, s);
    D3DXVec3Normalize(&result, &result);
    return result;
}

int fp_RenderMarchingCubes::s_EdgeTable[256] = {
        0x0  , 0x109, 0x203, 0x30a, 0x406, 0x50f, 0x605, 0x70c,
        0x80c, 0x905, 0xa0f, 0xb06, 0xc0a, 0xd03, 0xe09, 0xf00,
        0x190, 0x99 , 0x393, 0x29a, 0x596, 0x49f, 0x795, 0x69c,
        0x99c, 0x895, 0xb9f, 0xa96, 0xd9a, 0xc93, 0xf99, 0xe90,
        0x230, 0x339, 0x33 , 0x13a, 0x636, 0x73f, 0x435, 0x53c,
        0xa3c, 0xb35, 0x83f, 0x936, 0xe3a, 0xf33, 0xc39, 0xd30,
        0x3a0, 0x2a9, 0x1a3, 0xaa , 0x7a6, 0x6af, 0x5a5, 0x4ac,
        0xbac, 0xaa5, 0x9af, 0x8a6, 0xfaa, 0xea3, 0xda9, 0xca0,
        0x460, 0x569, 0x663, 0x76a, 0x66 , 0x16f, 0x265, 0x36c,
        0xc6c, 0xd65, 0xe6f, 0xf66, 0x86a, 0x963, 0xa69, 0xb60,
        0x5f0, 0x4f9, 0x7f3, 0x6fa, 0x1f6, 0xff , 0x3f5, 0x2fc,
        0xdfc, 0xcf5, 0xfff, 0xef6, 0x9fa, 0x8f3, 0xbf9, 0xaf0,
        0x650, 0x759, 0x453, 0x55a, 0x256, 0x35f, 0x55 , 0x15c,
        0xe5c, 0xf55, 0xc5f, 0xd56, 0xa5a, 0xb53, 0x859, 0x950,
        0x7c0, 0x6c9, 0x5c3, 0x4ca, 0x3c6, 0x2cf, 0x1c5, 0xcc ,
        0xfcc, 0xec5, 0xdcf, 0xcc6, 0xbca, 0xac3, 0x9c9, 0x8c0,
        0x8c0, 0x9c9, 0xac3, 0xbca, 0xcc6, 0xdcf, 0xec5, 0xfcc,
        0xcc , 0x1c5, 0x2cf, 0x3c6, 0x4ca, 0x5c3, 0x6c9, 0x7c0,
        0x950, 0x859, 0xb53, 0xa5a, 0xd56, 0xc5f, 0xf55, 0xe5c,
        0x15c, 0x55 , 0x35f, 0x256, 0x55a, 0x453, 0x759, 0x650,
        0xaf0, 0xbf9, 0x8f3, 0x9fa, 0xef6, 0xfff, 0xcf5, 0xdfc,
        0x2fc, 0x3f5, 0xff , 0x1f6, 0x6fa, 0x7f3, 0x4f9, 0x5f0,
        0xb60, 0xa69, 0x963, 0x86a, 0xf66, 0xe6f, 0xd65, 0xc6c,
        0x36c, 0x265, 0x16f, 0x66 , 0x76a, 0x663, 0x569, 0x460,
        0xca0, 0xda9, 0xea3, 0xfaa, 0x8a6, 0x9af, 0xaa5, 0xbac,
        0x4ac, 0x5a5, 0x6af, 0x7a6, 0xaa , 0x1a3, 0x2a9, 0x3a0,
        0xd30, 0xc39, 0xf33, 0xe3a, 0x936, 0x83f, 0xb35, 0xa3c,
        0x53c, 0x435, 0x73f, 0x636, 0x13a, 0x33 , 0x339, 0x230,
        0xe90, 0xf99, 0xc93, 0xd9a, 0xa96, 0xb9f, 0x895, 0x99c,
        0x69c, 0x795, 0x49f, 0x596, 0x29a, 0x393, 0x99 , 0x190,
        0xf00, 0xe09, 0xd03, 0xc0a, 0xb06, 0xa0f, 0x905, 0x80c,
        0x70c, 0x605, 0x50f, 0x406, 0x30a, 0x203, 0x109, 0x0 };

int fp_RenderMarchingCubes::s_TriTable[256][16] = {
        {-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
        {0, 8, 3, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
        {0, 1, 9, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
        {1, 8, 3, 9, 8, 1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
        {1, 2, 10, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
        {0, 8, 3, 1, 2, 10, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
        {9, 2, 10, 0, 2, 9, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
        {2, 8, 3, 2, 10, 8, 10, 9, 8, -1, -1, -1, -1, -1, -1, -1},
        {3, 11, 2, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
        {0, 11, 2, 8, 11, 0, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
        {1, 9, 0, 2, 3, 11, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
        {1, 11, 2, 1, 9, 11, 9, 8, 11, -1, -1, -1, -1, -1, -1, -1},
        {3, 10, 1, 11, 10, 3, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
        {0, 10, 1, 0, 8, 10, 8, 11, 10, -1, -1, -1, -1, -1, -1, -1},
        {3, 9, 0, 3, 11, 9, 11, 10, 9, -1, -1, -1, -1, -1, -1, -1},
        {9, 8, 10, 10, 8, 11, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
        {4, 7, 8, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
        {4, 3, 0, 7, 3, 4, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
        {0, 1, 9, 8, 4, 7, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
        {4, 1, 9, 4, 7, 1, 7, 3, 1, -1, -1, -1, -1, -1, -1, -1},
        {1, 2, 10, 8, 4, 7, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
        {3, 4, 7, 3, 0, 4, 1, 2, 10, -1, -1, -1, -1, -1, -1, -1},
        {9, 2, 10, 9, 0, 2, 8, 4, 7, -1, -1, -1, -1, -1, -1, -1},
        {2, 10, 9, 2, 9, 7, 2, 7, 3, 7, 9, 4, -1, -1, -1, -1},
        {8, 4, 7, 3, 11, 2, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
        {11, 4, 7, 11, 2, 4, 2, 0, 4, -1, -1, -1, -1, -1, -1, -1},
        {9, 0, 1, 8, 4, 7, 2, 3, 11, -1, -1, -1, -1, -1, -1, -1},
        {4, 7, 11, 9, 4, 11, 9, 11, 2, 9, 2, 1, -1, -1, -1, -1},
        {3, 10, 1, 3, 11, 10, 7, 8, 4, -1, -1, -1, -1, -1, -1, -1},
        {1, 11, 10, 1, 4, 11, 1, 0, 4, 7, 11, 4, -1, -1, -1, -1},
        {4, 7, 8, 9, 0, 11, 9, 11, 10, 11, 0, 3, -1, -1, -1, -1},
        {4, 7, 11, 4, 11, 9, 9, 11, 10, -1, -1, -1, -1, -1, -1, -1},
        {9, 5, 4, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
        {9, 5, 4, 0, 8, 3, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
        {0, 5, 4, 1, 5, 0, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
        {8, 5, 4, 8, 3, 5, 3, 1, 5, -1, -1, -1, -1, -1, -1, -1},
        {1, 2, 10, 9, 5, 4, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
        {3, 0, 8, 1, 2, 10, 4, 9, 5, -1, -1, -1, -1, -1, -1, -1},
        {5, 2, 10, 5, 4, 2, 4, 0, 2, -1, -1, -1, -1, -1, -1, -1},
        {2, 10, 5, 3, 2, 5, 3, 5, 4, 3, 4, 8, -1, -1, -1, -1},
        {9, 5, 4, 2, 3, 11, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
        {0, 11, 2, 0, 8, 11, 4, 9, 5, -1, -1, -1, -1, -1, -1, -1},
        {0, 5, 4, 0, 1, 5, 2, 3, 11, -1, -1, -1, -1, -1, -1, -1},
        {2, 1, 5, 2, 5, 8, 2, 8, 11, 4, 8, 5, -1, -1, -1, -1},
        {10, 3, 11, 10, 1, 3, 9, 5, 4, -1, -1, -1, -1, -1, -1, -1},
        {4, 9, 5, 0, 8, 1, 8, 10, 1, 8, 11, 10, -1, -1, -1, -1},
        {5, 4, 0, 5, 0, 11, 5, 11, 10, 11, 0, 3, -1, -1, -1, -1},
        {5, 4, 8, 5, 8, 10, 10, 8, 11, -1, -1, -1, -1, -1, -1, -1},
        {9, 7, 8, 5, 7, 9, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
        {9, 3, 0, 9, 5, 3, 5, 7, 3, -1, -1, -1, -1, -1, -1, -1},
        {0, 7, 8, 0, 1, 7, 1, 5, 7, -1, -1, -1, -1, -1, -1, -1},
        {1, 5, 3, 3, 5, 7, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
        {9, 7, 8, 9, 5, 7, 10, 1, 2, -1, -1, -1, -1, -1, -1, -1},
        {10, 1, 2, 9, 5, 0, 5, 3, 0, 5, 7, 3, -1, -1, -1, -1},
        {8, 0, 2, 8, 2, 5, 8, 5, 7, 10, 5, 2, -1, -1, -1, -1},
        {2, 10, 5, 2, 5, 3, 3, 5, 7, -1, -1, -1, -1, -1, -1, -1},
        {7, 9, 5, 7, 8, 9, 3, 11, 2, -1, -1, -1, -1, -1, -1, -1},
        {9, 5, 7, 9, 7, 2, 9, 2, 0, 2, 7, 11, -1, -1, -1, -1},
        {2, 3, 11, 0, 1, 8, 1, 7, 8, 1, 5, 7, -1, -1, -1, -1},
        {11, 2, 1, 11, 1, 7, 7, 1, 5, -1, -1, -1, -1, -1, -1, -1},
        {9, 5, 8, 8, 5, 7, 10, 1, 3, 10, 3, 11, -1, -1, -1, -1},
        {5, 7, 0, 5, 0, 9, 7, 11, 0, 1, 0, 10, 11, 10, 0, -1},
        {11, 10, 0, 11, 0, 3, 10, 5, 0, 8, 0, 7, 5, 7, 0, -1},
        {11, 10, 5, 7, 11, 5, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
        {10, 6, 5, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
        {0, 8, 3, 5, 10, 6, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
        {9, 0, 1, 5, 10, 6, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
        {1, 8, 3, 1, 9, 8, 5, 10, 6, -1, -1, -1, -1, -1, -1, -1},
        {1, 6, 5, 2, 6, 1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
        {1, 6, 5, 1, 2, 6, 3, 0, 8, -1, -1, -1, -1, -1, -1, -1},
        {9, 6, 5, 9, 0, 6, 0, 2, 6, -1, -1, -1, -1, -1, -1, -1},
        {5, 9, 8, 5, 8, 2, 5, 2, 6, 3, 2, 8, -1, -1, -1, -1},
        {2, 3, 11, 10, 6, 5, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
        {11, 0, 8, 11, 2, 0, 10, 6, 5, -1, -1, -1, -1, -1, -1, -1},
        {0, 1, 9, 2, 3, 11, 5, 10, 6, -1, -1, -1, -1, -1, -1, -1},
        {5, 10, 6, 1, 9, 2, 9, 11, 2, 9, 8, 11, -1, -1, -1, -1},
        {6, 3, 11, 6, 5, 3, 5, 1, 3, -1, -1, -1, -1, -1, -1, -1},
        {0, 8, 11, 0, 11, 5, 0, 5, 1, 5, 11, 6, -1, -1, -1, -1},
        {3, 11, 6, 0, 3, 6, 0, 6, 5, 0, 5, 9, -1, -1, -1, -1},
        {6, 5, 9, 6, 9, 11, 11, 9, 8, -1, -1, -1, -1, -1, -1, -1},
        {5, 10, 6, 4, 7, 8, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
        {4, 3, 0, 4, 7, 3, 6, 5, 10, -1, -1, -1, -1, -1, -1, -1},
        {1, 9, 0, 5, 10, 6, 8, 4, 7, -1, -1, -1, -1, -1, -1, -1},
        {10, 6, 5, 1, 9, 7, 1, 7, 3, 7, 9, 4, -1, -1, -1, -1},
        {6, 1, 2, 6, 5, 1, 4, 7, 8, -1, -1, -1, -1, -1, -1, -1},
        {1, 2, 5, 5, 2, 6, 3, 0, 4, 3, 4, 7, -1, -1, -1, -1},
        {8, 4, 7, 9, 0, 5, 0, 6, 5, 0, 2, 6, -1, -1, -1, -1},
        {7, 3, 9, 7, 9, 4, 3, 2, 9, 5, 9, 6, 2, 6, 9, -1},
        {3, 11, 2, 7, 8, 4, 10, 6, 5, -1, -1, -1, -1, -1, -1, -1},
        {5, 10, 6, 4, 7, 2, 4, 2, 0, 2, 7, 11, -1, -1, -1, -1},
        {0, 1, 9, 4, 7, 8, 2, 3, 11, 5, 10, 6, -1, -1, -1, -1},
        {9, 2, 1, 9, 11, 2, 9, 4, 11, 7, 11, 4, 5, 10, 6, -1},
        {8, 4, 7, 3, 11, 5, 3, 5, 1, 5, 11, 6, -1, -1, -1, -1},
        {5, 1, 11, 5, 11, 6, 1, 0, 11, 7, 11, 4, 0, 4, 11, -1},
        {0, 5, 9, 0, 6, 5, 0, 3, 6, 11, 6, 3, 8, 4, 7, -1},
        {6, 5, 9, 6, 9, 11, 4, 7, 9, 7, 11, 9, -1, -1, -1, -1},
        {10, 4, 9, 6, 4, 10, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
        {4, 10, 6, 4, 9, 10, 0, 8, 3, -1, -1, -1, -1, -1, -1, -1},
        {10, 0, 1, 10, 6, 0, 6, 4, 0, -1, -1, -1, -1, -1, -1, -1},
        {8, 3, 1, 8, 1, 6, 8, 6, 4, 6, 1, 10, -1, -1, -1, -1},
        {1, 4, 9, 1, 2, 4, 2, 6, 4, -1, -1, -1, -1, -1, -1, -1},
        {3, 0, 8, 1, 2, 9, 2, 4, 9, 2, 6, 4, -1, -1, -1, -1},
        {0, 2, 4, 4, 2, 6, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
        {8, 3, 2, 8, 2, 4, 4, 2, 6, -1, -1, -1, -1, -1, -1, -1},
        {10, 4, 9, 10, 6, 4, 11, 2, 3, -1, -1, -1, -1, -1, -1, -1},
        {0, 8, 2, 2, 8, 11, 4, 9, 10, 4, 10, 6, -1, -1, -1, -1},
        {3, 11, 2, 0, 1, 6, 0, 6, 4, 6, 1, 10, -1, -1, -1, -1},
        {6, 4, 1, 6, 1, 10, 4, 8, 1, 2, 1, 11, 8, 11, 1, -1},
        {9, 6, 4, 9, 3, 6, 9, 1, 3, 11, 6, 3, -1, -1, -1, -1},
        {8, 11, 1, 8, 1, 0, 11, 6, 1, 9, 1, 4, 6, 4, 1, -1},
        {3, 11, 6, 3, 6, 0, 0, 6, 4, -1, -1, -1, -1, -1, -1, -1},
        {6, 4, 8, 11, 6, 8, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
        {7, 10, 6, 7, 8, 10, 8, 9, 10, -1, -1, -1, -1, -1, -1, -1},
        {0, 7, 3, 0, 10, 7, 0, 9, 10, 6, 7, 10, -1, -1, -1, -1},
        {10, 6, 7, 1, 10, 7, 1, 7, 8, 1, 8, 0, -1, -1, -1, -1},
        {10, 6, 7, 10, 7, 1, 1, 7, 3, -1, -1, -1, -1, -1, -1, -1},
        {1, 2, 6, 1, 6, 8, 1, 8, 9, 8, 6, 7, -1, -1, -1, -1},
        {2, 6, 9, 2, 9, 1, 6, 7, 9, 0, 9, 3, 7, 3, 9, -1},
        {7, 8, 0, 7, 0, 6, 6, 0, 2, -1, -1, -1, -1, -1, -1, -1},
        {7, 3, 2, 6, 7, 2, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
        {2, 3, 11, 10, 6, 8, 10, 8, 9, 8, 6, 7, -1, -1, -1, -1},
        {2, 0, 7, 2, 7, 11, 0, 9, 7, 6, 7, 10, 9, 10, 7, -1},
        {1, 8, 0, 1, 7, 8, 1, 10, 7, 6, 7, 10, 2, 3, 11, -1},
        {11, 2, 1, 11, 1, 7, 10, 6, 1, 6, 7, 1, -1, -1, -1, -1},
        {8, 9, 6, 8, 6, 7, 9, 1, 6, 11, 6, 3, 1, 3, 6, -1},
        {0, 9, 1, 11, 6, 7, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
        {7, 8, 0, 7, 0, 6, 3, 11, 0, 11, 6, 0, -1, -1, -1, -1},
        {7, 11, 6, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
        {7, 6, 11, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
        {3, 0, 8, 11, 7, 6, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
        {0, 1, 9, 11, 7, 6, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
        {8, 1, 9, 8, 3, 1, 11, 7, 6, -1, -1, -1, -1, -1, -1, -1},
        {10, 1, 2, 6, 11, 7, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
        {1, 2, 10, 3, 0, 8, 6, 11, 7, -1, -1, -1, -1, -1, -1, -1},
        {2, 9, 0, 2, 10, 9, 6, 11, 7, -1, -1, -1, -1, -1, -1, -1},
        {6, 11, 7, 2, 10, 3, 10, 8, 3, 10, 9, 8, -1, -1, -1, -1},
        {7, 2, 3, 6, 2, 7, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
        {7, 0, 8, 7, 6, 0, 6, 2, 0, -1, -1, -1, -1, -1, -1, -1},
        {2, 7, 6, 2, 3, 7, 0, 1, 9, -1, -1, -1, -1, -1, -1, -1},
        {1, 6, 2, 1, 8, 6, 1, 9, 8, 8, 7, 6, -1, -1, -1, -1},
        {10, 7, 6, 10, 1, 7, 1, 3, 7, -1, -1, -1, -1, -1, -1, -1},
        {10, 7, 6, 1, 7, 10, 1, 8, 7, 1, 0, 8, -1, -1, -1, -1},
        {0, 3, 7, 0, 7, 10, 0, 10, 9, 6, 10, 7, -1, -1, -1, -1},
        {7, 6, 10, 7, 10, 8, 8, 10, 9, -1, -1, -1, -1, -1, -1, -1},
        {6, 8, 4, 11, 8, 6, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
        {3, 6, 11, 3, 0, 6, 0, 4, 6, -1, -1, -1, -1, -1, -1, -1},
        {8, 6, 11, 8, 4, 6, 9, 0, 1, -1, -1, -1, -1, -1, -1, -1},
        {9, 4, 6, 9, 6, 3, 9, 3, 1, 11, 3, 6, -1, -1, -1, -1},
        {6, 8, 4, 6, 11, 8, 2, 10, 1, -1, -1, -1, -1, -1, -1, -1},
        {1, 2, 10, 3, 0, 11, 0, 6, 11, 0, 4, 6, -1, -1, -1, -1},
        {4, 11, 8, 4, 6, 11, 0, 2, 9, 2, 10, 9, -1, -1, -1, -1},
        {10, 9, 3, 10, 3, 2, 9, 4, 3, 11, 3, 6, 4, 6, 3, -1},
        {8, 2, 3, 8, 4, 2, 4, 6, 2, -1, -1, -1, -1, -1, -1, -1},
        {0, 4, 2, 4, 6, 2, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
        {1, 9, 0, 2, 3, 4, 2, 4, 6, 4, 3, 8, -1, -1, -1, -1},
        {1, 9, 4, 1, 4, 2, 2, 4, 6, -1, -1, -1, -1, -1, -1, -1},
        {8, 1, 3, 8, 6, 1, 8, 4, 6, 6, 10, 1, -1, -1, -1, -1},
        {10, 1, 0, 10, 0, 6, 6, 0, 4, -1, -1, -1, -1, -1, -1, -1},
        {4, 6, 3, 4, 3, 8, 6, 10, 3, 0, 3, 9, 10, 9, 3, -1},
        {10, 9, 4, 6, 10, 4, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
        {4, 9, 5, 7, 6, 11, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
        {0, 8, 3, 4, 9, 5, 11, 7, 6, -1, -1, -1, -1, -1, -1, -1},
        {5, 0, 1, 5, 4, 0, 7, 6, 11, -1, -1, -1, -1, -1, -1, -1},
        {11, 7, 6, 8, 3, 4, 3, 5, 4, 3, 1, 5, -1, -1, -1, -1},
        {9, 5, 4, 10, 1, 2, 7, 6, 11, -1, -1, -1, -1, -1, -1, -1},
        {6, 11, 7, 1, 2, 10, 0, 8, 3, 4, 9, 5, -1, -1, -1, -1},
        {7, 6, 11, 5, 4, 10, 4, 2, 10, 4, 0, 2, -1, -1, -1, -1},
        {3, 4, 8, 3, 5, 4, 3, 2, 5, 10, 5, 2, 11, 7, 6, -1},
        {7, 2, 3, 7, 6, 2, 5, 4, 9, -1, -1, -1, -1, -1, -1, -1},
        {9, 5, 4, 0, 8, 6, 0, 6, 2, 6, 8, 7, -1, -1, -1, -1},
        {3, 6, 2, 3, 7, 6, 1, 5, 0, 5, 4, 0, -1, -1, -1, -1},
        {6, 2, 8, 6, 8, 7, 2, 1, 8, 4, 8, 5, 1, 5, 8, -1},
        {9, 5, 4, 10, 1, 6, 1, 7, 6, 1, 3, 7, -1, -1, -1, -1},
        {1, 6, 10, 1, 7, 6, 1, 0, 7, 8, 7, 0, 9, 5, 4, -1},
        {4, 0, 10, 4, 10, 5, 0, 3, 10, 6, 10, 7, 3, 7, 10, -1},
        {7, 6, 10, 7, 10, 8, 5, 4, 10, 4, 8, 10, -1, -1, -1, -1},
        {6, 9, 5, 6, 11, 9, 11, 8, 9, -1, -1, -1, -1, -1, -1, -1},
        {3, 6, 11, 0, 6, 3, 0, 5, 6, 0, 9, 5, -1, -1, -1, -1},
        {0, 11, 8, 0, 5, 11, 0, 1, 5, 5, 6, 11, -1, -1, -1, -1},
        {6, 11, 3, 6, 3, 5, 5, 3, 1, -1, -1, -1, -1, -1, -1, -1},
        {1, 2, 10, 9, 5, 11, 9, 11, 8, 11, 5, 6, -1, -1, -1, -1},
        {0, 11, 3, 0, 6, 11, 0, 9, 6, 5, 6, 9, 1, 2, 10, -1},
        {11, 8, 5, 11, 5, 6, 8, 0, 5, 10, 5, 2, 0, 2, 5, -1},
        {6, 11, 3, 6, 3, 5, 2, 10, 3, 10, 5, 3, -1, -1, -1, -1},
        {5, 8, 9, 5, 2, 8, 5, 6, 2, 3, 8, 2, -1, -1, -1, -1},
        {9, 5, 6, 9, 6, 0, 0, 6, 2, -1, -1, -1, -1, -1, -1, -1},
        {1, 5, 8, 1, 8, 0, 5, 6, 8, 3, 8, 2, 6, 2, 8, -1},
        {1, 5, 6, 2, 1, 6, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
        {1, 3, 6, 1, 6, 10, 3, 8, 6, 5, 6, 9, 8, 9, 6, -1},
        {10, 1, 0, 10, 0, 6, 9, 5, 0, 5, 6, 0, -1, -1, -1, -1},
        {0, 3, 8, 5, 6, 10, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
        {10, 5, 6, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
        {11, 5, 10, 7, 5, 11, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
        {11, 5, 10, 11, 7, 5, 8, 3, 0, -1, -1, -1, -1, -1, -1, -1},
        {5, 11, 7, 5, 10, 11, 1, 9, 0, -1, -1, -1, -1, -1, -1, -1},
        {10, 7, 5, 10, 11, 7, 9, 8, 1, 8, 3, 1, -1, -1, -1, -1},
        {11, 1, 2, 11, 7, 1, 7, 5, 1, -1, -1, -1, -1, -1, -1, -1},
        {0, 8, 3, 1, 2, 7, 1, 7, 5, 7, 2, 11, -1, -1, -1, -1},
        {9, 7, 5, 9, 2, 7, 9, 0, 2, 2, 11, 7, -1, -1, -1, -1},
        {7, 5, 2, 7, 2, 11, 5, 9, 2, 3, 2, 8, 9, 8, 2, -1},
        {2, 5, 10, 2, 3, 5, 3, 7, 5, -1, -1, -1, -1, -1, -1, -1},
        {8, 2, 0, 8, 5, 2, 8, 7, 5, 10, 2, 5, -1, -1, -1, -1},
        {9, 0, 1, 5, 10, 3, 5, 3, 7, 3, 10, 2, -1, -1, -1, -1},
        {9, 8, 2, 9, 2, 1, 8, 7, 2, 10, 2, 5, 7, 5, 2, -1},
        {1, 3, 5, 3, 7, 5, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
        {0, 8, 7, 0, 7, 1, 1, 7, 5, -1, -1, -1, -1, -1, -1, -1},
        {9, 0, 3, 9, 3, 5, 5, 3, 7, -1, -1, -1, -1, -1, -1, -1},
        {9, 8, 7, 5, 9, 7, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
        {5, 8, 4, 5, 10, 8, 10, 11, 8, -1, -1, -1, -1, -1, -1, -1},
        {5, 0, 4, 5, 11, 0, 5, 10, 11, 11, 3, 0, -1, -1, -1, -1},
        {0, 1, 9, 8, 4, 10, 8, 10, 11, 10, 4, 5, -1, -1, -1, -1},
        {10, 11, 4, 10, 4, 5, 11, 3, 4, 9, 4, 1, 3, 1, 4, -1},
        {2, 5, 1, 2, 8, 5, 2, 11, 8, 4, 5, 8, -1, -1, -1, -1},
        {0, 4, 11, 0, 11, 3, 4, 5, 11, 2, 11, 1, 5, 1, 11, -1},
        {0, 2, 5, 0, 5, 9, 2, 11, 5, 4, 5, 8, 11, 8, 5, -1},
        {9, 4, 5, 2, 11, 3, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
        {2, 5, 10, 3, 5, 2, 3, 4, 5, 3, 8, 4, -1, -1, -1, -1},
        {5, 10, 2, 5, 2, 4, 4, 2, 0, -1, -1, -1, -1, -1, -1, -1},
        {3, 10, 2, 3, 5, 10, 3, 8, 5, 4, 5, 8, 0, 1, 9, -1},
        {5, 10, 2, 5, 2, 4, 1, 9, 2, 9, 4, 2, -1, -1, -1, -1},
        {8, 4, 5, 8, 5, 3, 3, 5, 1, -1, -1, -1, -1, -1, -1, -1},
        {0, 4, 5, 1, 0, 5, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
        {8, 4, 5, 8, 5, 3, 9, 0, 5, 0, 3, 5, -1, -1, -1, -1},
        {9, 4, 5, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
        {4, 11, 7, 4, 9, 11, 9, 10, 11, -1, -1, -1, -1, -1, -1, -1},
        {0, 8, 3, 4, 9, 7, 9, 11, 7, 9, 10, 11, -1, -1, -1, -1},
        {1, 10, 11, 1, 11, 4, 1, 4, 0, 7, 4, 11, -1, -1, -1, -1},
        {3, 1, 4, 3, 4, 8, 1, 10, 4, 7, 4, 11, 10, 11, 4, -1},
        {4, 11, 7, 9, 11, 4, 9, 2, 11, 9, 1, 2, -1, -1, -1, -1},
        {9, 7, 4, 9, 11, 7, 9, 1, 11, 2, 11, 1, 0, 8, 3, -1},
        {11, 7, 4, 11, 4, 2, 2, 4, 0, -1, -1, -1, -1, -1, -1, -1},
        {11, 7, 4, 11, 4, 2, 8, 3, 4, 3, 2, 4, -1, -1, -1, -1},
        {2, 9, 10, 2, 7, 9, 2, 3, 7, 7, 4, 9, -1, -1, -1, -1},
        {9, 10, 7, 9, 7, 4, 10, 2, 7, 8, 7, 0, 2, 0, 7, -1},
        {3, 7, 10, 3, 10, 2, 7, 4, 10, 1, 10, 0, 4, 0, 10, -1},
        {1, 10, 2, 8, 7, 4, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
        {4, 9, 1, 4, 1, 7, 7, 1, 3, -1, -1, -1, -1, -1, -1, -1},
        {4, 9, 1, 4, 1, 7, 0, 8, 1, 8, 7, 1, -1, -1, -1, -1},
        {4, 0, 3, 7, 4, 3, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
        {4, 8, 7, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
        {9, 10, 8, 10, 11, 8, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
        {3, 0, 9, 3, 9, 11, 11, 9, 10, -1, -1, -1, -1, -1, -1, -1},
        {0, 1, 10, 0, 10, 8, 8, 10, 11, -1, -1, -1, -1, -1, -1, -1},
        {3, 1, 10, 11, 3, 10, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
        {1, 2, 11, 1, 11, 9, 9, 11, 8, -1, -1, -1, -1, -1, -1, -1},
        {3, 0, 9, 3, 9, 11, 1, 2, 9, 2, 11, 9, -1, -1, -1, -1},
        {0, 2, 11, 8, 0, 11, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
        {3, 2, 11, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
        {2, 3, 8, 2, 8, 10, 10, 8, 9, -1, -1, -1, -1, -1, -1, -1},
        {9, 10, 2, 0, 9, 2, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
        {2, 3, 8, 2, 8, 10, 0, 1, 8, 1, 10, 8, -1, -1, -1, -1},
        {1, 10, 2, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
        {1, 3, 8, 9, 1, 8, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
        {0, 9, 1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
        {0, 3, 8, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
        {-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1}};
