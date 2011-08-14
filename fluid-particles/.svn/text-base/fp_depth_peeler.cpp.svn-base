#include "DXUT.h"
#include "fp_depth_peeler.h"

#define FP_DEPTH_PEELER_EFFECT_FILE L"fp_depth_peeler.fx"

const D3D10_INPUT_ELEMENT_DESC fp_DepthPeelerVertex::Layout[] = {
        {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D10_INPUT_PER_VERTEX_DATA,0},
        {"INDEX", 0, DXGI_FORMAT_R32_UINT, 0, 12, D3D10_INPUT_PER_VERTEX_DATA, 0} };

D3D10_QUERY_DESC fp_DepthPeeler::s_QueryDesc = {D3D10_QUERY_OCCLUSION_PREDICATE, 0};

fp_DepthPeeler::fp_DepthPeeler(
        int MaxDepthComplexity,
        int NumParticles,
        fp_FluidParticle* Particles) :
    m_MaxDepthComplexity(MaxDepthComplexity),
    m_NumParticles(NumParticles),
    m_Particles(Particles),
    m_AllPeels(NULL),
    m_AllPeelsSRV(NULL),
    m_Effect(NULL),
    m_EffectVarLastPeelDepth(NULL),
    m_EffectVarViewProjection(NULL),
    m_FragmentsLeft(false),
    m_PeelDepth(NULL),
    m_OcclusionQuery(NULL),
    m_TechPeeling(NULL) {

    m_PeelSRV = new ID3D10ShaderResourceView*[MaxDepthComplexity];
    m_PeelRTV = new ID3D10RenderTargetView*[MaxDepthComplexity];
}

fp_DepthPeeler::~fp_DepthPeeler() {
    OnD3D10DestroyDevice(NULL);
    delete[] m_PeelSRV;
    delete[] m_PeelRTV;
}

HRESULT fp_DepthPeeler::OnD3D10CreateDevice(
        ID3D10Device* D3DDevice,
        const DXGI_SURFACE_DESC* BackBufferSurfaceDesc,
        void* UserContext ) {
	HRESULT hr;

    // Read the D3DX effect file
    m_Effect = fp_Util::LoadEffect(D3DDevice, FP_DEPTH_PEELER_EFFECT_FILE);

    // Obtain technique objects
	m_TechPeeling = m_Effect->GetTechniqueByName("DepthPeeling");	

    // Obtain effect variables
    m_EffectVarLastPeelDepth = m_Effect->GetVariableByName("g_LastPeelDepth")
            ->AsShaderResource();
    m_EffectVarViewProjection = m_Effect->GetVariableByName("g_ViewProjection")
            ->AsMatrix();

    bool allValid = m_Effect->IsValid() != 0 != 0;
    allValid |= m_TechPeeling->IsValid() != 0;
    allValid |= m_EffectVarLastPeelDepth->IsValid() != 0;
    allValid |= m_EffectVarViewProjection->IsValid() != 0;
    if(!allValid)
        return E_FAIL;

    // Create vertex buffer
    D3D10_PASS_DESC passDesc;
    V_RETURN(m_TechPeeling->GetPassByIndex(0)->GetDesc(&passDesc));
    V_RETURN(D3DDevice->CreateInputLayout(fp_DepthPeelerVertex::Layout, 2,
            passDesc.pIAInputSignature, passDesc.IAInputSignatureSize, &m_VertexLayout));
    
    D3D10_BUFFER_DESC bufferDesc;
    bufferDesc.Usage = D3D10_USAGE_DYNAMIC;
    bufferDesc.ByteWidth = m_NumParticles * sizeof(fp_DepthPeelerVertex);
    bufferDesc.BindFlags = D3D10_BIND_VERTEX_BUFFER;
    bufferDesc.CPUAccessFlags = D3D10_CPU_ACCESS_WRITE;
    bufferDesc.MiscFlags = 0;
    V_RETURN(D3DDevice->CreateBuffer(&bufferDesc, NULL, &m_VertexBuffer));

    // Create occlution query
    D3DDevice->CreateQuery(&s_QueryDesc, &m_OcclusionQuery);

	return S_OK;
}

HRESULT fp_DepthPeeler::OnD3D10ResizedSwapChain(
        ID3D10Device* D3DDevice,
        IDXGISwapChain *SwapChain,
        const DXGI_SURFACE_DESC* BackBufferSurfaceDesc,
        void* UserContext ) {
    HRESULT hr;

	D3D10_TEXTURE2D_DESC texDesc;
    texDesc.Width = BackBufferSurfaceDesc->Width;
    texDesc.Height = BackBufferSurfaceDesc->Height;
    texDesc.MipLevels = 1;
    texDesc.ArraySize = 2;
    texDesc.Format = DXGI_FORMAT_R32_TYPELESS; 
    texDesc.SampleDesc.Count = 1;
    texDesc.SampleDesc.Quality = 0;
    texDesc.Usage = D3D10_USAGE_DEFAULT; 
    texDesc.BindFlags = D3D10_BIND_DEPTH_STENCIL | D3D10_BIND_SHADER_RESOURCE;
    texDesc.CPUAccessFlags = 0;
    texDesc.MiscFlags = 0;    

    D3D10_SHADER_RESOURCE_VIEW_DESC srvDesc;
	srvDesc.Format = DXGI_FORMAT_R32_FLOAT;
    srvDesc.ViewDimension = D3D10_SRV_DIMENSION_TEXTURE2DARRAY;
    srvDesc.Texture2DArray.ArraySize = 1;
    srvDesc.Texture2DArray.FirstArraySlice = 0;
    srvDesc.Texture2DArray.MipLevels = 1;
    srvDesc.Texture2DArray.MostDetailedMip = 0;

	D3D10_DEPTH_STENCIL_VIEW_DESC dsvDesc;
	dsvDesc.Format = DXGI_FORMAT_D32_FLOAT;
    dsvDesc.ViewDimension = D3D10_DSV_DIMENSION_TEXTURE2DARRAY;
    dsvDesc.Texture2DArray.ArraySize = 1;
    dsvDesc.Texture2DArray.FirstArraySlice = 0;
    dsvDesc.Texture2DArray.MipSlice = 0;

    // PeelDepth
    V_RETURN(D3DDevice->CreateTexture2D(&texDesc, NULL, &m_PeelDepth));
    V_RETURN(D3DDevice->CreateShaderResourceView(m_PeelDepth, &srvDesc,
            &m_PeelDepthSRV[0]));
    V_RETURN(D3DDevice->CreateDepthStencilView(m_PeelDepth, &dsvDesc,
            &m_PeelDepthDSV[0]));
    srvDesc.Texture2DArray.FirstArraySlice = 1;
    dsvDesc.Texture2DArray.FirstArraySlice = 1;
    V_RETURN(D3DDevice->CreateShaderResourceView(m_PeelDepth, &srvDesc,
            &m_PeelDepthSRV[1]));
    V_RETURN(D3DDevice->CreateDepthStencilView(m_PeelDepth, &dsvDesc,
            &m_PeelDepthDSV[1]));

    texDesc.ArraySize = m_MaxDepthComplexity;
    texDesc.Format = DXGI_FORMAT_R32_UINT;
    texDesc.BindFlags = D3D10_BIND_SHADER_RESOURCE | D3D10_BIND_RENDER_TARGET;

    srvDesc.Format = DXGI_FORMAT_R32_UINT;
    srvDesc.Texture2DArray.ArraySize = m_MaxDepthComplexity;
    srvDesc.Texture2DArray.FirstArraySlice = 0;

    // all peels
    V_RETURN(D3DDevice->CreateTexture2D(&texDesc, NULL, &m_AllPeels));
    V_RETURN(D3DDevice->CreateShaderResourceView(m_AllPeels, &srvDesc,
            &m_AllPeelsSRV));

    srvDesc.Texture2DArray.ArraySize = 1;

    D3D10_RENDER_TARGET_VIEW_DESC rtvDesc;
    rtvDesc.Format = DXGI_FORMAT_R32_UINT;
    rtvDesc.ViewDimension = D3D10_RTV_DIMENSION_TEXTURE2DARRAY;
    rtvDesc.Texture2DArray.ArraySize = 1;
    rtvDesc.Texture2DArray.MipSlice = 0;

    // single peels
    for(int i=0; i < m_MaxDepthComplexity; i++) {
        srvDesc.Texture2DArray.FirstArraySlice = i;
        rtvDesc.Texture2DArray.FirstArraySlice = i;

        V_RETURN(D3DDevice->CreateShaderResourceView(m_AllPeels, &srvDesc,
            &m_PeelSRV[i]));
        V_RETURN(D3DDevice->CreateRenderTargetView(m_AllPeels, &rtvDesc,
            &m_PeelRTV[i]));
    }
}

void fp_DepthPeeler::OnD3D10ReleasingSwapChain( void* UserContext ) {
    for(int i=0; i < m_MaxDepthComplexity; i++) {
        SAFE_RELEASE(m_PeelSRV[i]);
        SAFE_RELEASE(m_PeelRTV[i]);
    }
    SAFE_RELEASE(m_AllPeelsSRV);
    SAFE_RELEASE(m_AllPeels);
    SAFE_RELEASE(m_PeelDepthDSV[0]);
    SAFE_RELEASE(m_PeelDepthDSV[1]);
    SAFE_RELEASE(m_PeelDepthSRV[0]);
    SAFE_RELEASE(m_PeelDepthSRV[1]);
    SAFE_RELEASE(m_PeelDepth);
}

void fp_DepthPeeler::OnD3D10DestroyDevice( void* UserContext ) {
    SAFE_RELEASE(m_OcclusionQuery);
    SAFE_RELEASE(m_VertexBuffer);
    SAFE_RELEASE(m_VertexLayout);
    SAFE_RELEASE(m_Effect);
}

void fp_DepthPeeler::OnD3D10FrameRender(
        ID3D10Device* D3DDevice,
        const D3DXMATRIX*  ViewProjection) {
    HRESULT hr;

    m_DepthComplexity = 0;
    m_FragmentsLeft = true;

    fp_DepthPeelerVertex *depthPeelerVertices;
    m_VertexBuffer->Map(D3D10_MAP_WRITE_DISCARD, 0, (void**)&depthPeelerVertices);
    for(int i = 0; i < m_NumParticles; i++) {
        depthPeelerVertices->m_Position = m_Particles[i].m_Position;
        depthPeelerVertices->m_Index = i + 1;
        depthPeelerVertices++;
    }
    m_VertexBuffer->Unmap();

    D3DDevice->IASetInputLayout(m_VertexLayout);
    UINT stride = sizeof(fp_DepthPeelerVertex);
    UINT offset = 0;
    D3DDevice->IASetVertexBuffers(0, 1, &m_VertexBuffer, &stride, &offset);
    D3DDevice->IASetPrimitiveTopology(D3D10_PRIMITIVE_TOPOLOGY_POINTLIST);

    m_EffectVarViewProjection->SetMatrix((float*) ViewProjection);

    ID3D10RenderTargetView *oldRTV;
    ID3D10DepthStencilView *oldDSV;
    D3DDevice->OMGetRenderTargets(1, &oldRTV, &oldDSV);

    while(m_FragmentsLeft && m_DepthComplexity < m_MaxDepthComplexity) {
        D3DDevice->ClearDepthStencilView(m_PeelDepthDSV[m_DepthComplexity % 2],
                D3D10_CLEAR_DEPTH, 1.0f, 0);
        unsigned int clearVal = 0;
        D3DDevice->ClearRenderTargetView(m_PeelRTV[m_DepthComplexity],
                reinterpret_cast<float*>(&clearVal));

        D3DDevice->OMSetRenderTargets(1, &m_PeelRTV[m_DepthComplexity],
                m_PeelDepthDSV[m_DepthComplexity % 2]);
        
        if(m_DepthComplexity > 0)
            m_EffectVarLastPeelDepth->SetResource(
                    m_PeelDepthSRV[(m_DepthComplexity-1) % 2]);        

        m_TechPeeling->GetPassByIndex(m_DepthComplexity == 0 ? 0 : 1)->Apply(0);

        m_OcclusionQuery->Begin();

        D3DDevice->Draw(m_NumParticles, 0);

        m_OcclusionQuery->End();

        while(S_OK != m_OcclusionQuery->GetData(&m_FragmentsLeft, sizeof(BOOL), 0));

        if(m_FragmentsLeft) 
            m_DepthComplexity++;
    }

    D3DDevice->OMSetRenderTargets(1, &oldRTV, oldDSV);

    SAFE_RELEASE(oldDSV);
    SAFE_RELEASE(oldRTV);
}

int fp_DepthPeeler::MaxDepthComplexity() {
    return m_MaxDepthComplexity;
}

int fp_DepthPeeler::DepthComplexity() {
    return m_DepthComplexity;
}

ID3D10ShaderResourceView* fp_DepthPeeler::GetAllPeelsSRV() {
    return m_AllPeelsSRV;
}

ID3D10ShaderResourceView* fp_DepthPeeler::GetPeelSRV(int PeelIndex) {
    return m_PeelSRV[PeelIndex];
}