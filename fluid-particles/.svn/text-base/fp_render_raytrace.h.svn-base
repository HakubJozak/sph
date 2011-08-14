#pragma once
#ifndef FP_RENDER_RAYTRACE_H
#define FP_RENDER_RAYTRACE_H

#include "DXUT.h"

#include "fp_global.h"
#include "fp_util.h"
#include "fp_cpu_sph.h"
#include "fp_bounding_box.h"

//--------------------------------------------------------------------------------------
// Fluid particles render technique: Isosurface via GPU raytrace
//--------------------------------------------------------------------------------------

struct fp_SplatParticleVertex {
    D3DXVECTOR4 m_PositionAndDensity;
    static const D3D10_INPUT_ELEMENT_DESC Layout[];
};

class fp_RenderRaytrace {
public:      
    fp_Fluid* m_Fluid;
    fp_FluidParticle* m_Particles;
    std::vector<std::wstring> m_CubeMapNames;
    int m_CurrentCubeMap;

    fp_RenderRaytrace(
            fp_Fluid* Fluid,
            float VoxelSize,
            float IsoLevel = FP_RAYTRACE_DEFAULT_ISO_LEVEL,
            float StepScale = FP_RAYTRACE_DEFAULT_STEP_SCALE,
            const fp_VolumeIndex& VolumeDimensions
                    = FP_RAYTRACE_VOLUME_TEXTURE_DIMENSIONS);
    ~fp_RenderRaytrace();

    void SetFluid(fp_Fluid* Fluid);
    void SetIsoLevel(float IsoLevel);
    void SetStepScale(float StepScale);
    void SetVoxelSize(float VoxelSize);
    void SetRefractionRatio(float RefractionRatio);
    D3DXVECTOR3 GetVolumeSize();
    fp_VolumeIndex GetVolumeTextureSize();
    
    // Defines the lower, left, front corner of the volume
    void SetVolumeStartPos(D3DXVECTOR3* VolumeStartPos);
    
    // D3D9 specific
    HRESULT OnD3D9CreateDevice(
            IDirect3DDevice9* D3DDevice,
            const D3DSURFACE_DESC* BackBufferSurfaceDesc,
            void* UserContext );
    HRESULT OnD3D9ResetDevice(
            IDirect3DDevice9* D3DDevice,
            const D3DSURFACE_DESC* BackBufferSurfaceDesc,
            void* UserContext );
    void    OnD3D9FrameRender(IDirect3DDevice9* D3DDevice);
    void    OnD3D9LostDevice( void* UserContext );
    void    OnD3D9DestroyDevice( void* UserContext );

    // D3D10 specific
    HRESULT OnD3D10CreateDevice(
            ID3D10Device* D3DDevice,
            const DXGI_SURFACE_DESC* BackBufferSurfaceDesc,
            void* UserContext );
    HRESULT OnD3D10ResizedSwapChain(
            ID3D10Device* D3DDevice,
            IDXGISwapChain *SwapChain,
            const DXGI_SURFACE_DESC* BackBufferSurfaceDesc,
            void* UserContext );
    void    OnD3D10ReleasingSwapChain( void* UserContext );
    void    OnD3D10DestroyDevice( void* UserContext );
    void    OnD3D10FrameRender(
            ID3D10Device* D3DDevice,
            const D3DXMATRIX*  View,
            const D3DXMATRIX*  Projection,
            const D3DXMATRIX*  ViewProjection,
            const D3DXMATRIX*  InvView,
            bool UpdateVis);

private:
    ID3D10Texture3D *m_VolumeTexture;
    ID3D10RenderTargetView *m_VolumeRTV;
    ID3D10ShaderResourceView *m_VolumeSRV;    

    ID3D10Texture1D *m_WValsMulParticleMassTexture;
    ID3D10ShaderResourceView *m_WValsMulParticleMassSRV;
    int m_WValsMulParticleMassLength;

    std::vector<ID3D10ShaderResourceView*> m_EnvironmentMapSRV;

    fp_RenderTarget2D* m_ExitPoint;

    ID3D10InputLayout* m_SplatParticleVertexLayout;
    ID3D10Buffer* m_SplatParticleVertexBuffer;    

    ID3D10Effect* m_Effect;

    ID3D10EffectTechnique* m_TechRenderRaytrace;

    ID3D10EffectMatrixVariable* m_EffectVarWorldToNDS;
    ID3D10EffectVectorVariable* m_EffectVarCornersPos;
    ID3D10EffectScalarVariable* m_EffectVarHalfParticleVoxelDiameter;
    ID3D10EffectScalarVariable* m_EffectVarParticleVoxelRadius;
    ID3D10EffectVectorVariable* m_EffectVarVolumeDimensions;
    ID3D10EffectShaderResourceVariable* m_EffectVarWValsMulParticleMass;

    ID3D10EffectScalarVariable* m_EffectVarIsoLevel;
    ID3D10EffectShaderResourceVariable* m_EffectVarDensityGrid;
    ID3D10EffectVectorVariable* m_EffectVarTexDelta;
    ID3D10EffectMatrixVariable* m_EffectVarWorld;
    ID3D10EffectMatrixVariable* m_EffectVarWorldView;
    ID3D10EffectMatrixVariable* m_EffectVarWorldViewProjection;
    ID3D10EffectMatrixVariable* m_EffectVarInvView;
    ID3D10EffectShaderResourceVariable* m_EffectVarExitPoint;
    ID3D10EffectVectorVariable* m_EffectVarBBoxStart;
    ID3D10EffectVectorVariable* m_EffectVarBBoxSize;
    ID3D10EffectScalarVariable* m_EffectVarStepSize;
    ID3D10EffectVectorVariable* m_EffectVarVolumeSizeRatio;

    ID3D10EffectShaderResourceVariable* m_EffectVarEnvironmentMap;
    ID3D10EffectScalarVariable* m_EffectVarRefractionRatio;
    ID3D10EffectScalarVariable* m_EffectVarRefractionRatioSq;
    ID3D10EffectScalarVariable* m_EffectVarR0;
    ID3D10EffectScalarVariable* m_EffectVarOneMinusR0;
    ID3D10EffectScalarVariable* m_EffectVarRefractionRatio_2;
    ID3D10EffectScalarVariable* m_EffectVarRefractionRatioSq_2;
    ID3D10EffectScalarVariable* m_EffectVarR0_2;
    ID3D10EffectScalarVariable* m_EffectVarOneMinusR0_2;

    fp_VolumeIndex m_VolumeDimensions;
    
    int m_NumParticles;
    float m_VoxelSize;
    int m_ParticleVoxelDiameter;
    int m_ParticleVoxelRadius;
    bool m_NeedPerPixelStepSize;
    float m_IsoLevel;
    float m_StepScale;

    fp_BoundingBox m_BBox;
    fp_BoundingBox m_EnvironmentBox;

    HRESULT CreateVolumeTexture(ID3D10Device* pd3dDevice);
    void FillVolumeTexture(ID3D10Device* D3DDevice); 
    void DestroyVolumeTexture();

    void RenderEnvironment(
            ID3D10Device* D3DDevice,
            const D3DXMATRIX* View,
            const D3DXMATRIX* Projection);

    void RenderVolume(
            ID3D10Device* D3DDevice,
            const D3DXMATRIX* View,
            const D3DXMATRIX* ViewProjection,
            const D3DXMATRIX*  InvView); 
};

#endif
