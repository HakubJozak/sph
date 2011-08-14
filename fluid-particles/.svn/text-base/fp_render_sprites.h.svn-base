#pragma once
#ifndef FP_RENDER_SPRITES_H
#define FP_RENDER_SPRITES_H

#include "DXUT.h"

#include "fp_global.h"
#include "fp_cpu_sph.h"

//--------------------------------------------------------------------------------------
// Fluid particles render technique: Point Sprites
//--------------------------------------------------------------------------------------

struct fp_SpriteVertex {
    D3DXVECTOR3 m_Position;

	enum FVF {
		FVF_Flags = D3DFVF_XYZ
	};

    static const D3D10_INPUT_ELEMENT_DESC Layout[];
};

class fp_RenderSprites {
public:    
    fp_FluidParticle* m_Particles;

    fp_RenderSprites(int NumParticles, fp_FluidParticle* Particles);
    ~fp_RenderSprites();
    float GetSpriteSize() const;
    void SetSpriteSize(float SpriteSize);

    // DX9 specific
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

    // DX10 specific
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
            const D3DXMATRIX*  ViewProjection,
			const D3DXMATRIX*  InvView); 

private:
    int m_NumParticles;  
    float m_SpriteSize;

    // D3D9 resources
    LPDIRECT3DVERTEXBUFFER9 m_VertexBuffer9;
    LPDIRECT3DTEXTURE9 m_Texture9;

    // D3D10 resources
    ID3D10Buffer* m_VertexBuffer10;
    ID3D10InputLayout* m_VertexLayout;
    ID3D10ShaderResourceView * m_Texture10SRV;
    ID3D10Effect* m_Effect10;
    ID3D10EffectTechnique*  m_TechRenderSprites;
    ID3D10EffectShaderResourceVariable* m_EffectTexture;
    ID3D10EffectMatrixVariable* m_EffectVarViewProjection;
    ID3D10EffectVectorVariable* m_EffectVarSpriteCornersWorldS;
};

#endif
