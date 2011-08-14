#pragma once
#ifndef FP_DEPTH_PEELER_H
#define FP_DEPTH_PEELER_H

#include "DXUT.h"

#include "fp_global.h"
#include "fp_util.h"
#include "fp_cpu_sph.h"

struct fp_DepthPeelerVertex {
    D3DXVECTOR3 m_Position;
    unsigned int m_Index;

    static const D3D10_INPUT_ELEMENT_DESC Layout[];
};

class fp_DepthPeeler {
public:
	fp_DepthPeeler(int MaxDepthComplexity, int NumParticles, fp_FluidParticle* Particles);
	~fp_DepthPeeler();
	
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
            const D3DXMATRIX*  ViewProjection); 
	int MaxDepthComplexity();
	int DepthComplexity();
    ID3D10ShaderResourceView* GetAllPeelsSRV();
    ID3D10ShaderResourceView* GetPeelSRV(int PeelIndex);

private:
    int m_NumParticles;
    fp_FluidParticle* m_Particles;

	int m_MaxDepthComplexity;
	int m_DepthComplexity;

    ID3D10InputLayout* m_VertexLayout;
    ID3D10Buffer* m_VertexBuffer;

	ID3D10Effect* m_Effect;
    ID3D10EffectTechnique* m_TechPeeling;

	ID3D10EffectShaderResourceVariable* m_EffectVarLastPeelDepth;
    ID3D10EffectMatrixVariable* m_EffectVarViewProjection;

	//Occlusion Query
	bool m_FragmentsLeft;
	ID3D10Query* m_OcclusionQuery;
	static D3D10_QUERY_DESC s_QueryDesc;

    ID3D10Texture2D* m_PeelDepth;
    ID3D10ShaderResourceView* m_PeelDepthSRV[2];
    ID3D10DepthStencilView* m_PeelDepthDSV[2];

	ID3D10Texture2D* m_AllPeels; // Texture Array
    ID3D10ShaderResourceView* m_AllPeelsSRV; // Array SRV
    ID3D10ShaderResourceView** m_PeelSRV; // Array of single SRVs
    ID3D10RenderTargetView** m_PeelRTV; // Array of single RTVs
};

#endif
