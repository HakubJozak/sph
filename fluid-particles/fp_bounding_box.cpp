#include "DXUT.h"
#include "fp_bounding_box.h"

fp_BoundingBox::fp_BoundingBox(D3DXVECTOR3* Start, D3DXVECTOR3* Size) : 
        m_SizeChangedSinceLastRender(true),
        m_VertexBuffer(NULL),
        m_WireframeIndexBuffer(NULL),
        m_SolidIndexBuffer(NULL),
        m_VertexLayout(NULL) {
    m_Start = (Start != NULL) ? *Start : D3DXVECTOR3(-1, -1, -1);
    m_Size = (Size != NULL) ? *Size : D3DXVECTOR3(2, 2, 2);
    D3DXMatrixTranslation(&m_World, m_Start.x, m_Start.y, m_Start.z);
    D3DXMatrixTranslation(&m_EnvironmentWorld, -m_Size.x / 2, -m_Size.y / 2, 
            -m_Size.z / 2);
}


fp_BoundingBox::~fp_BoundingBox() {
	OnD3D10DestroyDevice();
}

HRESULT fp_BoundingBox::OnD3D10CreateDevice(
        ID3D10Device* D3DDevice,
        ID3D10EffectTechnique* RenderTechnique) {
	HRESULT hr;

    // Define the input layout
    D3D10_INPUT_ELEMENT_DESC layout[] = {
        {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D10_INPUT_PER_VERTEX_DATA, 0}  
    };
    UINT numElements = 1;

    // Create the input layout
    D3D10_PASS_DESC PassDesc;

	RenderTechnique->GetPassByIndex(1)->GetDesc(&PassDesc);
    V_RETURN(D3DDevice->CreateInputLayout(layout, numElements, PassDesc.pIAInputSignature,
            PassDesc.IAInputSignatureSize, &m_VertexLayout));

    // Create vertex buffer
    D3D10_BUFFER_DESC bd;
    bd.Usage = D3D10_USAGE_DYNAMIC;
    bd.ByteWidth = sizeof(D3DXVECTOR3) * 8;
    bd.BindFlags = D3D10_BIND_VERTEX_BUFFER;
    bd.CPUAccessFlags = D3D10_CPU_ACCESS_WRITE;
    bd.MiscFlags = 0;    
    V_RETURN(D3DDevice->CreateBuffer(&bd, NULL, &m_VertexBuffer));

    // Create wireframe index buffer
    WORD wireframeIndices[] = {
		0,1, 1,2, 2,3, 3,0,  // front
		4,5, 5,6, 6,7, 7,4,  // back
		0,4, 1,5, 2,6, 3,7   // sides
	};
    bd.Usage = D3D10_USAGE_DEFAULT;
    bd.ByteWidth = sizeof(WORD) * 24;
    bd.BindFlags = D3D10_BIND_INDEX_BUFFER;
    bd.CPUAccessFlags = 0;
    bd.MiscFlags = 0;
    D3D10_SUBRESOURCE_DATA InitData;
    InitData.pSysMem = wireframeIndices;
    V_RETURN(D3DDevice->CreateBuffer(&bd, &InitData, &m_WireframeIndexBuffer));

    // Create solid index buffer
    WORD solidIndices[] = {
		2,1,0, 0,3,2,    // front
		4,5,6, 4,6,7,    // back
		1,4,0, 4,1,5,    // bottom
		3,6,2, 6,3,7,    // top
		4,3,0, 4,7,3,    // left
		1,2,5, 6,5,2,    // right
	};

    bd.Usage = D3D10_USAGE_DEFAULT;
    bd.ByteWidth = sizeof(WORD) * 36;
    bd.BindFlags = D3D10_BIND_INDEX_BUFFER;
    bd.CPUAccessFlags = 0;
    bd.MiscFlags = 0;
    InitData.pSysMem = solidIndices;
    V_RETURN(D3DDevice->CreateBuffer(&bd, &InitData, &m_SolidIndexBuffer));

	return S_OK;
}

void fp_BoundingBox::OnD3D10FrameRenderWireframe(
        ID3D10Device* D3DDevice,
        bool DoSetup) {    
    if(m_SizeChangedSinceLastRender) {
        FillVertexBuffer(D3DDevice);
        m_SizeChangedSinceLastRender = false;
    }

	if (DoSetup) {
        SetLayoutAndVertexBuffer(D3DDevice);

		// Set primitive topology
		D3DDevice->IASetPrimitiveTopology(D3D10_PRIMITIVE_TOPOLOGY_LINELIST);

		// Set index buffer
		D3DDevice->IASetIndexBuffer(m_WireframeIndexBuffer, DXGI_FORMAT_R16_UINT, 0);
	}

	// Draw
    D3DDevice->DrawIndexed(24, 0, 0);
}

void fp_BoundingBox::OnD3D10FrameRenderSolid(ID3D10Device* D3DDevice,  bool DoSetup) {
    if(m_SizeChangedSinceLastRender) {
        FillVertexBuffer(D3DDevice);
        m_SizeChangedSinceLastRender = false;
    }
    
    if (DoSetup) {
        SetLayoutAndVertexBuffer(D3DDevice);

		// Set primitive topology
		D3DDevice->IASetPrimitiveTopology(D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		// Set index buffer
		D3DDevice->IASetIndexBuffer(m_SolidIndexBuffer, DXGI_FORMAT_R16_UINT, 0);
	}

	// Draw
    D3DDevice->DrawIndexed(36, 0, 0);
}

void fp_BoundingBox::OnD3D10DestroyDevice() {
    SAFE_RELEASE(m_SolidIndexBuffer);
	SAFE_RELEASE(m_WireframeIndexBuffer);
    SAFE_RELEASE(m_VertexBuffer);
	SAFE_RELEASE(m_VertexLayout);
}

D3DXVECTOR3 fp_BoundingBox::GetStart() {
	return m_Start;
}

D3DXVECTOR3 fp_BoundingBox::GetEnd() {
    return m_Start + m_Size;
}

D3DXVECTOR3 fp_BoundingBox::GetSize() {
	return m_Size;
}

D3DXVECTOR3 fp_BoundingBox::GetCenter() {
	return m_Start + 0.5f * m_Size;
}

D3DXMATRIX fp_BoundingBox::GetWorld() {
    return m_World;
}

D3DXMATRIX fp_BoundingBox::GetEnvironmentWorld() {
    return m_EnvironmentWorld;
}

void fp_BoundingBox::SetStart(D3DXVECTOR3* Start) {
    m_Start = *Start;
    D3DXMatrixTranslation(&m_World, Start->x, Start->y, Start->z);
}

void fp_BoundingBox::SetSize(D3DXVECTOR3* Size) {
    m_Size = *Size;
    m_SizeChangedSinceLastRender = true;
    D3DXMatrixTranslation(&m_EnvironmentWorld, -m_Size.x / 2, -m_Size.y / 2,
            -m_Size.z / 2);
}

void fp_BoundingBox::FillVertexBuffer(ID3D10Device* D3DDevice) {
    D3DXVECTOR3* vertices;
    m_VertexBuffer->Map(D3D10_MAP_WRITE_DISCARD, 0, (void**)&vertices);
    vertices[0] = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
    vertices[1] = D3DXVECTOR3(m_Size.x, 0.0f, 0.0f);
    vertices[2] = D3DXVECTOR3(m_Size.x, m_Size.y, 0.0f);
    vertices[3] = D3DXVECTOR3(0.0f, m_Size.y, 0.0f);
    vertices[4] = D3DXVECTOR3(0.0f, 0.0f, m_Size.z);
    vertices[5] = D3DXVECTOR3(m_Size.x, 0.0f, m_Size.z);
    vertices[6] = D3DXVECTOR3(m_Size);
    vertices[7] = D3DXVECTOR3(0.0f, m_Size.y, m_Size.z);
    m_VertexBuffer->Unmap();
}

void fp_BoundingBox::SetLayoutAndVertexBuffer(ID3D10Device* D3DDevice) {
	// Set the input layout
    D3DDevice->IASetInputLayout(m_VertexLayout);

    // Set vertex buffer
    UINT stride = sizeof(D3DXVECTOR3);
    UINT offset = 0;
    D3DDevice->IASetVertexBuffers(0, 1, &m_VertexBuffer, &stride, &offset);
}
