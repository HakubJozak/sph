#define D3DXFX_LARGEADDRESS_HANDLE
#include "DXUT.h"
#include "DXUTcamera.h"
#include "DXUTgui.h"
#include "DXUTsettingsDlg.h"
#include "SDKmisc.h"
#include "resource.h"

#include "fp_gui.h"
#include "fp_global.h"
#include "fp_cpu_sph.h"
#include "fp_render_sprites.h"
#include "fp_render_marching_cubes.h"
#include "fp_render_raytrace.h"
#include "fp_depth_peeler.h"

//#define DEBUG_VS   // Uncomment this line to debug D3D9 vertex shaders 
//#define DEBUG_PS   // Uncomment this line to debug D3D9 pixel shaders 

//--------------------------------------------------------------------------------------
// Global variables
//--------------------------------------------------------------------------------------
extern CModelViewerCamera      g_Camera;               // A model viewing camera
extern D3DXMATRIXA16           g_CenterMesh;

extern fp_GUI                  g_GUI;
extern fp_RenderSprites*       g_RenderSprites;
extern fp_RenderMarchingCubes* g_RenderMarchingCubes;
extern fp_RenderRaytrace*       g_RenderRaytrace;
extern fp_DepthPeeler*         g_DepthPeeler;

extern float                   g_LightScale;
extern int                     g_NumActiveLights;
extern int                     g_ActiveLight;
extern int                     g_RenderType;

extern D3DXVECTOR3*            g_Particle;
extern int                     g_Paticles;

extern fp_Fluid*               g_Sim;

// Direct3D9 resources
ID3DXEffect*            g_Effect9 = NULL;       // D3DX effect interface
ID3DXMesh*              g_Mesh9 = NULL;         // Mesh object
IDirect3DTexture9*      g_MeshTexture9 = NULL;  // Mesh texture

D3DXHANDLE g_LightDir;
D3DXHANDLE g_LightDiffuse;
D3DXHANDLE g_MaterialDiffuseColor;
D3DXHANDLE g_Time;
D3DXHANDLE g_NumLights;
D3DXHANDLE g_RenderSceneWithTexture1Light;
D3DXHANDLE g_RenderSceneWithTexture2Light;
D3DXHANDLE g_RenderSceneWithTexture3Light;

//--------------------------------------------------------------------------------------
// Forward declarations 
//--------------------------------------------------------------------------------------
bool    CALLBACK FP_IsD3D9DeviceAcceptable(
        D3DCAPS9* Caps, 
        D3DFORMAT AdapterFormat, 
        D3DFORMAT BackBufferFormat, 
        bool Windowed, 
        void* UserContext);
HRESULT CALLBACK FP_OnD3D9CreateDevice( 
        IDirect3DDevice9* D3DDevice, 
        const D3DSURFACE_DESC* BackBufferSurfaceDesc, 
        void* UserContext);
HRESULT CALLBACK FP_OnD3D9ResetDevice(
        IDirect3DDevice9* D3DDevice, 
        const D3DSURFACE_DESC* BackBufferSurfaceDesc, 
        void* UserContext);
void    CALLBACK FP_OnD3D9FrameRender(
        IDirect3DDevice9* D3DDevice,
        double Time,
        float ElapsedTime,
        void* UserContext );
void    CALLBACK FP_OnD3D9LostDevice( void* UserContext );
void    CALLBACK FP_OnD3D9DestroyDevice( void* UserContext );
HRESULT FP_LoadMesh( IDirect3DDevice9* D3DDevice, WCHAR* FileName, ID3DXMesh** Mesh );


//--------------------------------------------------------------------------------------
// Rejects any D3D9 devices that aren't acceptable to the app by returning false
//--------------------------------------------------------------------------------------
bool CALLBACK FP_IsD3D9DeviceAcceptable(
        D3DCAPS9* Caps,
        D3DFORMAT AdapterFormat, 
        D3DFORMAT BackBufferFormat,
        bool Windowed,
        void* UserContext ) {
    // No fallback defined by this app, so reject any device that doesn't support at
    //least ps2.0
    if( Caps->PixelShaderVersion < D3DPS_VERSION(2,0) )
        return false;

    // Skip backbuffer formats that don't support alpha blending
    IDirect3D9* pD3D = DXUTGetD3D9Object(); 
    if( FAILED( pD3D->CheckDeviceFormat( Caps->AdapterOrdinal, Caps->DeviceType,
                    AdapterFormat, D3DUSAGE_QUERY_POSTPIXELSHADER_BLENDING, 
                    D3DRTYPE_TEXTURE, BackBufferFormat ) ) )
        return false;

    return true;
}


//--------------------------------------------------------------------------------------
// Create any D3D9 resources that will live through a device reset (D3DPOOL_MANAGED)
// and aren't tied to the back buffer size 
//--------------------------------------------------------------------------------------
HRESULT CALLBACK FP_OnD3D9CreateDevice(
        IDirect3DDevice9* D3DDevice,
        const D3DSURFACE_DESC* BackBufferSurfaceDesc,
        void* UserContext) {    
    //HRESULT hr;

    g_GUI.OnD3D9CreateDevice(D3DDevice, BackBufferSurfaceDesc, UserContext);
    g_RenderSprites->OnD3D9CreateDevice(D3DDevice, BackBufferSurfaceDesc, UserContext);
    g_RenderMarchingCubes->OnD3D9CreateDevice(D3DDevice, BackBufferSurfaceDesc, UserContext);
    g_RenderRaytrace->OnD3D9CreateDevice(D3DDevice, BackBufferSurfaceDesc, UserContext);

    // Setup the camera's view parameters
    D3DXVECTOR3 vecEye(0.0f, 0.0f, -15.0f);
    D3DXVECTOR3 vecAt (0.0f, 0.0f, -0.0f);
    g_Camera.SetViewParams( &vecEye, &vecAt );
    g_Camera.SetRadius( FP_OBJECT_RADIUS*3.0f, FP_OBJECT_RADIUS*0.5f, FP_OBJECT_RADIUS*100.0f );

    return S_OK;
}


//--------------------------------------------------------------------------------------
// This function loads the mesh and ensures the mesh has normals; it also optimizes the 
// mesh for the graphics card's vertex cache, which improves performance by organizing 
// the internal triangle list for less cache misses.
//--------------------------------------------------------------------------------------
HRESULT FP_LoadMesh( IDirect3DDevice9* D3DDevice, WCHAR* FileName, ID3DXMesh** Mesh ) {
    ID3DXMesh* pMesh = NULL;
    WCHAR str[MAX_PATH];
    HRESULT hr;

    // Load the mesh with D3DX and get back a ID3DXMesh*.  For this
    // sample we'll ignore the X file's embedded materials since we know 
    // exactly the model we're loading.  See the mesh samples such as
    // "OptimizedMesh" for a more generic mesh loading example.
    V_RETURN( DXUTFindDXSDKMediaFileCch( str, MAX_PATH, FileName ) );
    V_RETURN( D3DXLoadMeshFromX(str, D3DXMESH_MANAGED, D3DDevice, NULL, NULL, NULL,
            NULL, &pMesh) );

    DWORD *rgdwAdjacency = NULL;

    // Make sure there are normals which are required for lighting
    if( !(pMesh->GetFVF() & D3DFVF_NORMAL) )
    {
        ID3DXMesh* pTempMesh;
        V( pMesh->CloneMeshFVF( pMesh->GetOptions(), pMesh->GetFVF()
                | D3DFVF_NORMAL, D3DDevice, &pTempMesh ) );
        V( D3DXComputeNormals( pTempMesh, NULL ) );

        SAFE_RELEASE( pMesh );
        pMesh = pTempMesh;
    }

    // Optimize the mesh for this graphics card's vertex cache 
    // so when rendering the mesh's triangle list the vertices will 
    // cache hit more often so it won't have to re-execute the vertex shader 
    // on those vertices so it will improve perf.     
    rgdwAdjacency = new DWORD[pMesh->GetNumFaces() * 3];
    if( rgdwAdjacency == NULL )
        return E_OUTOFMEMORY;
    V( pMesh->GenerateAdjacency(1e-6f,rgdwAdjacency) );
    V( pMesh->OptimizeInplace(D3DXMESHOPT_VERTEXCACHE, rgdwAdjacency, NULL, 
            NULL, NULL) );
    delete []rgdwAdjacency;

    *Mesh = pMesh;

    return S_OK;
}


//--------------------------------------------------------------------------------------
// Create any D3D9 resources that won't live through a device reset (D3DPOOL_DEFAULT) 
// or that are tied to the back buffer size 
//--------------------------------------------------------------------------------------
HRESULT CALLBACK FP_OnD3D9ResetDevice(
        IDirect3DDevice9* D3DDevice, 
        const D3DSURFACE_DESC* BackBufferSurfaceDesc,
        void* UserContext ) {
    HRESULT hr;
    //initPointSprites(D3DDevice);
    g_GUI.OnD3D9ResetDevice(D3DDevice, BackBufferSurfaceDesc, UserContext);
    g_RenderSprites->OnD3D9ResetDevice(D3DDevice, BackBufferSurfaceDesc, UserContext);
    g_RenderMarchingCubes->OnD3D9ResetDevice(D3DDevice, BackBufferSurfaceDesc, UserContext);
    g_RenderRaytrace->OnD3D9ResetDevice(D3DDevice, BackBufferSurfaceDesc, UserContext);

    if( g_Effect9 ) V_RETURN( g_Effect9->OnResetDevice() );
    // Setup the camera's projection parameters
    float fAspectRatio = BackBufferSurfaceDesc->Width / (float)BackBufferSurfaceDesc->Height;
    g_Camera.SetProjParams( D3DX_PI/4, fAspectRatio, 2.0f, 4000.0f );
    g_Camera.SetWindow( BackBufferSurfaceDesc->Width, BackBufferSurfaceDesc->Height );
    g_Camera.SetButtonMasks( MOUSE_LEFT_BUTTON, MOUSE_WHEEL, MOUSE_MIDDLE_BUTTON );    

    return S_OK;
}


//--------------------------------------------------------------------------------------
// Render the scene using the D3D9 device
//--------------------------------------------------------------------------------------
void CALLBACK FP_OnD3D9FrameRender(
        IDirect3DDevice9* D3DDevice, 
        double Time, 
        float ElapsedTime, 
        void* UserContext) {
    // If the settings dialog is being shown, then render it instead of rendering the
    //app's scene
    if( g_GUI.RenderSettingsDialog(ElapsedTime) )
        return;    

    HRESULT hr;
    D3DXMATRIXA16 WorldViewProjection;
    D3DXMATRIXA16 World;
    D3DXMATRIXA16 View;
    D3DXMATRIXA16 Proj;
   
    // Clear the render target and the zbuffer 
    V( D3DDevice->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DXCOLOR(
			FP_CLEAR_COLOR), 1.0f, 0) );

    // Render the scene
    if( SUCCEEDED( D3DDevice->BeginScene() ) )
    {
        // Get the projection & view matrix from the camera class
        World = *g_Camera.GetWorldMatrix();
        Proj = *g_Camera.GetProjMatrix();
        View = *g_Camera.GetViewMatrix();

        WorldViewProjection = World * View * Proj;

        for (int i=0; i < FP_MAX_LIGHTS; i++) {
            g_RenderMarchingCubes->m_Lights9[i].Direction = g_GUI.m_LightDir[i];
            D3DCOLORVALUE diffuse = { g_GUI.m_LightDiffuseColor->r,
                g_GUI.m_LightDiffuseColor->g, g_GUI.m_LightDiffuseColor->b, 1.0f };
            g_RenderMarchingCubes->m_Lights9[i].Diffuse = diffuse;
        }

		D3DDevice->SetTransform( D3DTS_WORLD, &World );
        D3DDevice->SetTransform( D3DTS_PROJECTION, &Proj );
        D3DDevice->SetTransform( D3DTS_VIEW, &View );        
       
        if(g_RenderType == FP_GUI_RENDERTYPE_SPRITE)
            g_RenderSprites->OnD3D9FrameRender(D3DDevice);
        else if(g_RenderType == FP_GUI_RENDERTYPE_MARCHING_CUBES)
            g_RenderMarchingCubes->OnD3D9FrameRender(D3DDevice);
        else if(g_RenderType == FP_GUI_RENDERTYPE_RAYTRACE)
            g_RenderRaytrace->OnD3D9FrameRender(D3DDevice);
        g_GUI.OnD3D9FrameRender(D3DDevice, ElapsedTime, g_Camera.GetEyePt(),
                &View, &Proj, g_NumActiveLights,
                g_ActiveLight, g_LightScale);

        V( D3DDevice->EndScene() );
    }
}


//--------------------------------------------------------------------------------------
// Release D3D9 resources created in the OnD3D9ResetDevice callback 
//--------------------------------------------------------------------------------------
void CALLBACK FP_OnD3D9LostDevice( void* UserContext ) {
    g_GUI.OnD3D9LostDevice(UserContext);
    if(g_RenderSprites) g_RenderSprites->OnD3D9LostDevice(UserContext);
    if(g_RenderMarchingCubes) g_RenderMarchingCubes->OnD3D9LostDevice(UserContext);
    if(g_RenderRaytrace) g_RenderRaytrace->OnD3D9LostDevice(UserContext);
    if(g_Effect9) g_Effect9->OnLostDevice();      
}


//--------------------------------------------------------------------------------------
// Release D3D9 resources created in the OnD3D9CreateDevice callback 
//--------------------------------------------------------------------------------------
void CALLBACK FP_OnD3D9DestroyDevice( void* UserContext )
{
    g_GUI.OnD3D9DestroyDevice(UserContext);
    if(g_RenderSprites) g_RenderSprites->OnD3D9DestroyDevice(UserContext);
    if(g_RenderMarchingCubes) g_RenderMarchingCubes->OnD3D9DestroyDevice(UserContext);
    if(g_RenderRaytrace) g_RenderRaytrace->OnD3D9DestroyDevice(UserContext);
    SAFE_RELEASE(g_Effect9);
    SAFE_RELEASE(g_Mesh9);
    SAFE_RELEASE(g_MeshTexture9);
}
