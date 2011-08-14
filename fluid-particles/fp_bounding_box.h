#pragma once
#ifndef FP_BOUNDING_BOX_H
#define FP_BOUNDING_BOX_H

#include "DXUT.h"

class fp_BoundingBox {
public:    
	fp_BoundingBox(
            D3DXVECTOR3* Start = NULL,
            D3DXVECTOR3* Size = NULL);
	virtual ~fp_BoundingBox();

	HRESULT OnD3D10CreateDevice(
            ID3D10Device* D3DDevice, 
            ID3D10EffectTechnique* RenderTechnique);
	void OnD3D10FrameRenderWireframe(ID3D10Device* D3DDevice, bool DoSetup = true);
	void OnD3D10FrameRenderSolid(ID3D10Device* D3DDevice, bool DoSetup = true);
	void OnD3D10DestroyDevice();	
    
    D3DXVECTOR3 GetStart();
    D3DXVECTOR3 GetEnd();
	D3DXVECTOR3 GetSize();
	D3DXVECTOR3 GetCenter();
    D3DXMATRIX GetWorld();
    D3DXMATRIX GetEnvironmentWorld();
    void SetStart(D3DXVECTOR3* Start);
    void SetSize(D3DXVECTOR3* Size);


protected:
    D3DXVECTOR3 m_Start;
	D3DXVECTOR3 m_Size;
    bool m_SizeChangedSinceLastRender;
    D3DXMATRIX m_World;
    D3DXMATRIX m_EnvironmentWorld;

	ID3D10Buffer* m_VertexBuffer;
	ID3D10Buffer* m_WireframeIndexBuffer;
	ID3D10Buffer* m_SolidIndexBuffer;
	ID3D10InputLayout* m_VertexLayout;

    void FillVertexBuffer(ID3D10Device* D3DDevice);
    void SetLayoutAndVertexBuffer(ID3D10Device* D3DDevice);    
};

#endif
