#include "DXUT.h"
#include "SDKmisc.h"
#pragma warning(disable:4530)
#pragma warning(disable:4995)
#include <sstream>
#pragma warning(default:4995)
#pragma warning(default:4530)
#include "fp_util.h"
  
fp_RenderTarget2D::fp_RenderTarget2D(
        ID3D10Device* D3DDevice,
        unsigned int Width,
        unsigned int Height,
        DXGI_FORMAT Format,
        unsigned int TargetCount,
        bool CreateDS,
        const D3D10_SUBRESOURCE_DATA *InitialData) :
	m_Device(D3DDevice),
    m_OldDS(NULL),
    m_TargetCount(TargetCount),
    m_DSV(NULL),
    m_TargetsSaved(false),
	m_DSTexture(NULL) {

    HRESULT hr;

    m_RTViewport.TopLeftX = 0;
    m_RTViewport.TopLeftY = 0;
    m_RTViewport.MinDepth = 0;
    m_RTViewport.MaxDepth = 1;
    m_RTViewport.Width =  Width;
    m_RTViewport.Height = Height; 

	for (unsigned int i=0; i < D3D10_SIMULTANEOUS_RENDER_TARGET_COUNT; i++) {
		m_OldRTVs[i] = NULL;
		m_SRV[i] = NULL;
		m_RTV[i] = NULL;
		m_Texture[i] = NULL;
	}

	D3D10_TEXTURE2D_DESC RTDesc;
	RTDesc.Width				= Width;
	RTDesc.Height				= Height;
    RTDesc.MipLevels			= 1;
    RTDesc.ArraySize			= 1;
    RTDesc.Format				= Format;
	RTDesc.SampleDesc.Count	= 1;
	RTDesc.SampleDesc.Quality	= 0;
    RTDesc.Usage				= D3D10_USAGE_DEFAULT;
    RTDesc.BindFlags			= D3D10_BIND_RENDER_TARGET | D3D10_BIND_SHADER_RESOURCE;
    RTDesc.CPUAccessFlags		= NULL;
    RTDesc.MiscFlags			= NULL;
	
	D3D10_RENDER_TARGET_VIEW_DESC RTVDesc;
	RTVDesc.Format				= Format;
	RTVDesc.ViewDimension		= D3D10_RTV_DIMENSION_TEXTURE2D;
	RTVDesc.Texture2D.MipSlice = 0;

	D3D10_SHADER_RESOURCE_VIEW_DESC SRVDesc;
	ZeroMemory( &SRVDesc, sizeof(SRVDesc) );
	SRVDesc.Format = Format;
	SRVDesc.ViewDimension = D3D10_SRV_DIMENSION_TEXTURE2D;
	SRVDesc.Texture2D.MipLevels = 1;
	SRVDesc.Texture2D.MostDetailedMip = 0;

	for (unsigned int i=0; i<m_TargetCount; i++) {
        const D3D10_SUBRESOURCE_DATA* init = (InitialData==NULL)
                ? NULL : &(InitialData[i]);

		V(m_Device->CreateTexture2D(&RTDesc, init,  &m_Texture[i]));
		V(m_Device->CreateRenderTargetView(m_Texture[i], &RTVDesc, &m_RTV[i]));
		V(m_Device->CreateShaderResourceView(m_Texture[i], &SRVDesc, &m_SRV[i]));
	}

	if (CreateDS) {
		D3D10_TEXTURE2D_DESC dstex;
		dstex.Width = Width;
		dstex.Height = Height;
		dstex.MipLevels = 1;
		dstex.ArraySize = 1;
		dstex.SampleDesc.Count = 1;
		dstex.SampleDesc.Quality = 0;
		dstex.Format = DXGI_FORMAT_D32_FLOAT;
		dstex.Usage = D3D10_USAGE_DEFAULT;
		dstex.BindFlags = D3D10_BIND_DEPTH_STENCIL;
		dstex.CPUAccessFlags = 0;
		dstex.MiscFlags = 0;

		V(m_Device->CreateTexture2D(&dstex, NULL, &m_DSTexture));

		D3D10_DEPTH_STENCIL_VIEW_DESC DescDS;
		DescDS.Format = DXGI_FORMAT_D32_FLOAT;
		DescDS.ViewDimension = D3D10_DSV_DIMENSION_TEXTURE2D;
		DescDS.Texture2DArray.FirstArraySlice = 0;
		DescDS.Texture2DArray.ArraySize = 1;
		DescDS.Texture2DArray.MipSlice = 0;
		V(m_Device->CreateDepthStencilView(m_DSTexture, &DescDS, &m_DSV));
	}
}

void fp_RenderTarget2D::Clear(float ClearColor[4]) {
	for (unsigned int i=0; i<m_TargetCount; i++) 
		m_Device->ClearRenderTargetView(m_RTV[i], ClearColor);			
}

ID3D10DepthStencilView* fp_RenderTarget2D::Bind(
        bool SaveRTs,
        bool KeepDS,
        ID3D10DepthStencilView* KnownDS) {  
    if (SaveRTs) SaveRT();
    m_Device->RSSetViewports(1,&m_RTViewport);
	if (KeepDS) {
		if (KnownDS)
			m_Device->OMSetRenderTargets(m_TargetCount, m_RTV, KnownDS);
		else {
			if (m_OldDS)
				m_Device->OMSetRenderTargets(m_TargetCount, m_RTV, m_OldDS);
			else {
				ID3D10DepthStencilView* pOldDS;
				m_Device->OMGetRenderTargets(0, NULL, &pOldDS);
				m_Device->OMSetRenderTargets(m_TargetCount, m_RTV, pOldDS);
				SAFE_RELEASE(pOldDS);
			}
		}
	} else
		m_Device->OMSetRenderTargets(m_TargetCount, m_RTV, m_DSV);
	
	return m_OldDS;
}

void fp_RenderTarget2D::Unbind() {
	RestoreRT();
}
	
fp_RenderTarget2D::~fp_RenderTarget2D() {
	for(unsigned int i=0; i<D3D10_SIMULTANEOUS_RENDER_TARGET_COUNT; i++)
        SAFE_RELEASE(m_OldRTVs[i]);
	SAFE_RELEASE(m_OldDS);

	for(unsigned int i=0; i<m_TargetCount; i++ ) {
		SAFE_RELEASE(m_SRV[i]);
		SAFE_RELEASE(m_RTV[i]);
		SAFE_RELEASE(m_Texture[i]);
	}

	SAFE_RELEASE(m_DSTexture);
	SAFE_RELEASE(m_DSV);    
}


void fp_RenderTarget2D::SaveRT() {   
	m_TargetsSaved = true;
    m_Device->OMGetRenderTargets(D3D10_SIMULTANEOUS_RENDER_TARGET_COUNT, m_OldRTVs,
            &m_OldDS);
    UINT cVPs = 1;
    m_Device->RSGetViewports(&cVPs,&m_OldViewport);
}

void fp_RenderTarget2D::RestoreRT() {
	if (!m_TargetsSaved) return;
	m_Device->OMSetRenderTargets(D3D10_SIMULTANEOUS_RENDER_TARGET_COUNT, m_OldRTVs, 
            m_OldDS);
    SAFE_RELEASE(m_OldDS);    
	for(unsigned int i=0; i < D3D10_SIMULTANEOUS_RENDER_TARGET_COUNT; i++)
        SAFE_RELEASE(m_OldRTVs[i]);	
    m_Device->RSSetViewports(1, &m_OldViewport);
	m_TargetsSaved = false;
}


float fp_Util::GetRandomMinMax( float fMin, float fMax ) {
    float fRandNum = (float)rand () / RAND_MAX;
    return fMin + (fMax - fMin) * fRandNum;
}

D3DXVECTOR3 fp_Util::GetRandomVector( void ) {
	D3DXVECTOR3 vVector;

    // Pick a random Z between -1.0f and 1.0f.
    vVector.z = fp_Util::GetRandomMinMax( -1.0f, 1.0f );
    
    // Get radius of this circle
    float radius = (float)sqrt(1 - vVector.z * vVector.z);
    
    // Pick a random point on a circle.
    float t = fp_Util::GetRandomMinMax( -D3DX_PI, D3DX_PI );

    // Compute matching X and Y for our Z.
    vVector.x = (float)cosf(t) * radius;
    vVector.y = (float)sinf(t) * radius;

	return vVector;
}

ID3D10Effect* fp_Util::LoadEffect(
		ID3D10Device* D3DDevice, 
		const LPCWSTR Filename, 
		const D3D10_SHADER_MACRO *ShaderMacros) {
	HRESULT hr;
	ID3D10Effect* effect10 = NULL;
	DWORD dwShaderFlags = D3D10_SHADER_ENABLE_STRICTNESS;
	#if defined( DEBUG ) || defined( _DEBUG )
	dwShaderFlags |= D3D10_SHADER_DEBUG;
	#endif
	WCHAR str[MAX_PATH];
	hr = DXUTFindDXSDKMediaFileCch(str, MAX_PATH, Filename);
	if (FAILED(hr)) {
		std::wstringstream s;
		s << L"Shader Load error: Could not locate \"" << Filename << L"\".";			
		MessageBoxW(NULL, s.str().c_str(), L"Shader Load Error", MB_OK);
		return NULL;
	}
	ID3D10Blob *errors;
	hr = D3DX10CreateEffectFromFile(str, ShaderMacros, NULL, "fx_4_0", dwShaderFlags, 0,
			D3DDevice, NULL, NULL, &effect10, &errors, NULL );
	if (FAILED(hr)) {
		std::wstringstream s;
		if (errors == NULL) {
			s << L"Unknown error loading shader: \"" << str << L"\".";	
		} else {
			s << L"Error loading shader \"" << str << "\"\n " << (CHAR*)errors->GetBufferPointer();	
		}
		MessageBoxW(NULL, s.str().c_str(), L"Shader Load Error", MB_OK);
		return NULL;
	}
	return effect10;
}

//return -1 if directory not found else number of files in dir
int fp_Util::ListDirectory(
        fp_StringList* FilesInDirectory,
        const LPCWSTR DirPath,
        const LPCWSTR Extension,
        bool IncludeHidden) {
	WCHAR pattern[MAX_PATH];
	WIN32_FIND_DATA fd;
	DWORD attr = FILE_ATTRIBUTE_DIRECTORY;
	if(!IncludeHidden) attr |= FILE_ATTRIBUTE_HIDDEN;
    if(Extension != NULL)
        StringCchPrintf(pattern, MAX_PATH, L"%s\\*.%s", DirPath, Extension);
    else
	    StringCchPrintf(pattern, MAX_PATH, L"%s\\*", DirPath);
	HANDLE find = FindFirstFile(pattern, &fd);
	if(find != INVALID_HANDLE_VALUE) {
		int count = 0;
		do {
            if(!(fd.dwFileAttributes & attr))
                FilesInDirectory->push_back(fd.cFileName);
		} while(FindNextFile(find, &fd));
		FindClose(find);
		return count;
	}
	return -1;
}
