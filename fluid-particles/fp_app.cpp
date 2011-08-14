#include "DXUT.h"
#include "DXUTcamera.h"
#include "DXUTgui.h"
#include "DXUTsettingsDlg.h"
#include "SDKmisc.h"
#include "SDKMesh.h"
#include "resource.h"

#include "fp_gui.h"
#include "fp_global.h"
#include "fp_util.h"
#include "fp_cpu_sph.h"
#include "fp_render_sprites.h"
#include "fp_render_marching_cubes.h"
#include "fp_render_raytrace.h"
#include "fp_depth_peeler.h"
#include "fp_thread.h"

//#define DEBUG_VS   // Uncomment this line to debug D3D9 vertex shaders
//#define DEBUG_PS   // Uncomment this line to debug D3D9 pixel shaders



//--------------------------------------------------------------------------------------
// Global variables
//--------------------------------------------------------------------------------------

CModelViewerCamera      g_Camera;               // A model viewing camera
fp_GUI                  g_GUI;

fp_Fluid*               g_Sim = NULL;
fp_RenderSprites*       g_RenderSprites = NULL;
fp_CPUDensityGrid*        g_CPUDensityGrid = NULL;
fp_RenderMarchingCubes* g_RenderMarchingCubes = NULL;
fp_RenderRaytrace*       g_RenderRaytrace = NULL;
fp_DepthPeeler*         g_DepthPeeler = NULL;

float                   g_LightScale;
int                     g_NumActiveLights;
int                     g_ActiveLight;
int                     g_RenderType;
bool                    g_MoveHorizontally;
bool                    g_StopSim;
bool                    g_UpdateVis;
float                   g_TimeFactor;

// Direct3D10 resources
ID3D10EffectVectorVariable* g_LightDir = NULL;
ID3D10EffectVectorVariable* g_LightDiffuse = NULL;
ID3D10EffectScalarVariable* g_Time = NULL;
ID3D10EffectVectorVariable* g_MaterialDiffuseColor = NULL;
ID3D10EffectVectorVariable* g_MaterialAmbientColor = NULL;
ID3D10EffectScalarVariable* g_NumLights = NULL;

fp_WorkerThreadManager g_WorkerThreadMgr;

//--------------------------------------------------------------------------------------
// Forward declarations
//--------------------------------------------------------------------------------------
bool    CALLBACK FP_ModifyDeviceSettings(
        DXUTDeviceSettings* DeviceSettings,
        void* UserContext );
void    CALLBACK FP_OnFrameMove(
        double Time,
        float ElapsedTime,
        void* UserContext );
void    CALLBACK FP_OnFrameMoveInitial(
        double Time,
        float ElapsedTime,
        void* UserContext );
LRESULT CALLBACK FP_MsgProc(
        HWND hWnd,
        UINT uMsg,
        WPARAM wParam,
        LPARAM lParam,
        bool* NoFurtherProcessing,
        void* UserContext );
void    CALLBACK FP_OnKeyboard(
        UINT Char,
        bool KeyDown,
        bool AltDown,
        void* UserContext );
void    CALLBACK FP_OnGUIEvent(
        UINT Event,
        int ControlID,
        CDXUTControl* Control,
        void* UserContext );

extern bool    CALLBACK FP_IsD3D9DeviceAcceptable(
        D3DCAPS9* Caps,
        D3DFORMAT AdapterFormat,
        D3DFORMAT BackBufferFormat,
        bool Windowed,
        void* UserContext );
extern HRESULT CALLBACK FP_OnD3D9CreateDevice(
        IDirect3DDevice9* D3DDevice,
        const D3DSURFACE_DESC* BackBufferSurfaceDesc,
        void* UserContext );
extern HRESULT CALLBACK FP_OnD3D9ResetDevice(
        IDirect3DDevice9* D3DDevice,
        const D3DSURFACE_DESC* BackBufferSurfaceDesc,
        void* UserContext );
extern void    CALLBACK FP_OnD3D9FrameRender(
        IDirect3DDevice9* D3DDevice,
        double Time,
        float ElapsedTime,
        void* UserContext );
extern void    CALLBACK FP_OnD3D9LostDevice( void* UserContext );
extern void    CALLBACK FP_OnD3D9DestroyDevice( void* UserContext );

bool    CALLBACK FP_IsD3D10DeviceAcceptable(
        UINT Adapter,
        UINT Output,
        D3D10_DRIVER_TYPE DeviceType,
        DXGI_FORMAT BackBufferFormat,
        bool Windowed,
        void* UserContext );
HRESULT CALLBACK FP_OnD3D10CreateDevice(
        ID3D10Device* D3DDevice,
        const DXGI_SURFACE_DESC* BackBufferSurfaceDesc,
        void* UserContext );
HRESULT CALLBACK FP_OnD3D10ResizedSwapChain(
        ID3D10Device* D3DDevice,
        IDXGISwapChain *SwapChain,
        const DXGI_SURFACE_DESC* BackBufferSurfaceDesc,
        void* UserContext );
void    CALLBACK FP_OnD3D10ReleasingSwapChain( void* UserContext );
void    CALLBACK FP_OnD3D10DestroyDevice( void* UserContext );
void    CALLBACK FP_OnD3D10FrameRender(
        ID3D10Device* D3DDevice,
        double Time,
        float ElapsedTime,
        void* UserContext );

void    FP_InitApp();
void    FP_FinishApp();
D3DXVECTOR3 CalcRaytraceVolumeStartPos(fp_Fluid* Fluid, fp_RenderRaytrace* RenderRaytrace);

//--------------------------------------------------------------------------------------
// Entry point to the program. Initializes everything and goes into a message processing
// loop. Idle time is used to render the scene.
//--------------------------------------------------------------------------------------
int WINAPI wWinMain(
        HINSTANCE Instance,
        HINSTANCE PrevInstance,
        LPWSTR CmdLine,
        int CmdShow ) {
    // Enable run-time memory check for debug builds.
    #if defined(DEBUG) | defined(_DEBUG)
    _CrtSetDbgFlag( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );
    #endif

    // DXUT will create and use the best device (either D3D9 or D3D10)
    // that is available on the system depending on which D3D callbacks are set below

    // Set DXUT callbacks
    DXUTSetCallbackDeviceChanging( FP_ModifyDeviceSettings );
    DXUTSetCallbackMsgProc( FP_MsgProc );
    DXUTSetCallbackKeyboard( FP_OnKeyboard );
    DXUTSetCallbackFrameMove( FP_OnFrameMoveInitial );

    DXUTSetCallbackD3D9DeviceAcceptable( FP_IsD3D9DeviceAcceptable );
    DXUTSetCallbackD3D9DeviceCreated( FP_OnD3D9CreateDevice );
    DXUTSetCallbackD3D9DeviceReset( FP_OnD3D9ResetDevice );
    DXUTSetCallbackD3D9FrameRender( FP_OnD3D9FrameRender );
    DXUTSetCallbackD3D9DeviceLost( FP_OnD3D9LostDevice );
    DXUTSetCallbackD3D9DeviceDestroyed( FP_OnD3D9DestroyDevice );

    DXUTSetCallbackD3D10DeviceAcceptable( FP_IsD3D10DeviceAcceptable );
    DXUTSetCallbackD3D10DeviceCreated( FP_OnD3D10CreateDevice );
    DXUTSetCallbackD3D10SwapChainResized( FP_OnD3D10ResizedSwapChain );
    DXUTSetCallbackD3D10FrameRender( FP_OnD3D10FrameRender );
    DXUTSetCallbackD3D10SwapChainReleasing( FP_OnD3D10ReleasingSwapChain );
    DXUTSetCallbackD3D10DeviceDestroyed( FP_OnD3D10DestroyDevice );

    FP_InitApp();

    DXUTInit( true, true, NULL ); // Parse the command line, show msgboxes on error, no extra command line params
    DXUTSetCursorSettings( true, true ); // Show the cursor and clip it when in full screen
    DXUTCreateWindow( L"Fluid particles" );
    DXUTCreateDevice( true, 1024, 768 );
    DXUTMainLoop(); // Enter into the DXUT render loop

    FP_FinishApp();

    return DXUTGetExitCode();
}


//--------------------------------------------------------------------------------------
// Initialize the app
//--------------------------------------------------------------------------------------
void FP_InitApp() {
    g_ActiveLight = 0;
    g_NumActiveLights = 1;
    g_LightScale = FP_DEFAULT_LIGHT_SCALE;
    g_TimeFactor = FP_DEFAULT_TIME_FACTOR;

    D3DXVECTOR3 center(0.0f, 0.0f, 0.0f);
    g_Sim = new fp_Fluid(&g_WorkerThreadMgr, FP_NUM_PARTICLES_X, FP_NUM_PARTICLES_Y,
            FP_NUM_PARTICLES_Z, FP_PARTICLE_SPACING_X, FP_PARTICLE_SPACING_Y,
            FP_PARTICLE_SPACING_Z, center, FP_GLASS_RADIUS, FP_GLASS_FLOOR);

    g_UpdateVis = true;
    g_RenderSprites = new fp_RenderSprites(FP_NUM_PARTICLES, g_Sim->m_Particles);
    g_CPUDensityGrid = new fp_CPUDensityGrid(g_Sim);
    g_RenderMarchingCubes = new fp_RenderMarchingCubes(g_CPUDensityGrid, 3);
    g_RenderRaytrace = new fp_RenderRaytrace(g_Sim, FP_RAYTRACE_DEFAULT_VOXEL_SIZE);
    g_DepthPeeler = new fp_DepthPeeler(FP_DEPTH_PEELER_MAX_DEPTH_COMPLEXITY,
            FP_NUM_PARTICLES, g_Sim->m_Particles);

    D3DXVECTOR3 volumeStartPos = CalcRaytraceVolumeStartPos(g_Sim, g_RenderRaytrace);
    g_RenderRaytrace->SetVolumeStartPos(&volumeStartPos);
    g_RenderMarchingCubes->m_NumActiveLights = g_NumActiveLights;

    D3DCOLORVALUE diffuse  = { g_GUI.m_LightDiffuseColor->r,
            g_GUI.m_LightDiffuseColor->g, g_GUI.m_LightDiffuseColor->b, 1.0f };
    D3DCOLORVALUE ambient  = { 0.05f, 0.05f, 0.05f, 1.0f };
    D3DCOLORVALUE specular = { 0.8f, 0.8f, 0.8f, 1.0f };

    ZeroMemory( g_RenderMarchingCubes->m_Lights9, sizeof(D3DLIGHT9) * FP_MAX_LIGHTS );

    for (int i=0; i < FP_MAX_LIGHTS; i++) {
        g_RenderMarchingCubes->m_Lights9[i].Type = D3DLIGHT_DIRECTIONAL;
        g_RenderMarchingCubes->m_Lights9[i].Direction = g_GUI.m_LightDir[i];
        g_RenderMarchingCubes->m_Lights9[i].Diffuse = diffuse;
        g_RenderMarchingCubes->m_Lights9[i].Ambient = ambient;
        g_RenderMarchingCubes->m_Lights9[i].Specular = specular;
    }
}

//--------------------------------------------------------------------------------------
// Initialize the app
//--------------------------------------------------------------------------------------
void FP_FinishApp() {
    delete g_Sim;
    g_Sim = NULL;
    delete g_RenderSprites;
    g_RenderSprites = NULL;
    delete g_CPUDensityGrid;
    g_CPUDensityGrid = NULL;
    delete g_RenderMarchingCubes;
    g_RenderMarchingCubes = NULL;
}

//--------------------------------------------------------------------------------------
// Called right before creating a D3D9 or D3D10 device, allowing the app to modify the
// device settings as needed
//--------------------------------------------------------------------------------------
bool CALLBACK FP_ModifyDeviceSettings(
        DXUTDeviceSettings* DeviceSettings,
        void* UserContext ) {
    if( DXUT_D3D9_DEVICE == DeviceSettings->ver ) {
        D3DCAPS9 Caps;
        IDirect3D9 *pD3D = DXUTGetD3D9Object();
        pD3D->GetDeviceCaps( DeviceSettings->d3d9.AdapterOrdinal,
                DeviceSettings->d3d9.DeviceType, &Caps );

        // If device doesn't support HW T&L or doesn't support 1.1 vertex shaders in HW
        // then switch to SWVP.
        if( (Caps.DevCaps & D3DDEVCAPS_HWTRANSFORMANDLIGHT) == 0 ||
            Caps.VertexShaderVersion < D3DVS_VERSION(1,1) ) {
            DeviceSettings->d3d9.BehaviorFlags = D3DCREATE_SOFTWARE_VERTEXPROCESSING;
        }

        // Debugging vertex shaders requires either REF or software vertex processing
        // and debugging pixel shaders requires REF.
        #ifdef DEBUG_VS
        if( pDeviceSettings->d3d9.DeviceType != D3DDEVTYPE_REF )
        {
            pDeviceSettings->d3d9.BehaviorFlags &= ~D3DCREATE_HARDWARE_VERTEXPROCESSING;
            pDeviceSettings->d3d9.BehaviorFlags &= ~D3DCREATE_PUREDEVICE;
            pDeviceSettings->BehaviorFlags |= D3DCREATE_SOFTWARE_VERTEXPROCESSING;
        }
        #endif
        #ifdef DEBUG_PS
        pDeviceSettings->d3d9.DeviceType = D3DDEVTYPE_REF;
        #endif
    }
    else {
        // Uncomment this to get debug information from D3D10
        //pDeviceSettings->d3d10.CreateFlags |= D3D10_CREATE_DEVICE_DEBUG;
    }

    // For the first device created if its a REF device, optionally display a warning
    // dialog box
    static bool s_bFirstTime = true;
    if( s_bFirstTime ) {
        s_bFirstTime = false;
        if((DXUT_D3D9_DEVICE == DeviceSettings->ver && DeviceSettings->d3d9.DeviceType
                == D3DDEVTYPE_REF) || (DXUT_D3D10_DEVICE == DeviceSettings->ver
                && DeviceSettings->d3d10.DriverType == D3D10_DRIVER_TYPE_REFERENCE))
            DXUTDisplaySwitchingToREFWarning( DeviceSettings->ver );
    }

    return true;
}


//--------------------------------------------------------------------------------------
// Handle updates to the scene.  This is called regardless of which D3D API is used
//--------------------------------------------------------------------------------------
void CALLBACK FP_OnFrameMoveInitial(
        double Time,
        float ElapsedTime,
        void* UserContext ) {
    // Update the camera's position based on user input
    g_Camera.FrameMove( ElapsedTime );
    if(Time > 0.2)
        DXUTSetCallbackFrameMove( FP_OnFrameMove );
}

void CALLBACK FP_OnFrameMove( double Time, float ElapsedTime, void* UserContext ) {
    // Update the camera's position based on user input
    g_Camera.FrameMove( ElapsedTime );
    float mouseDragX, mouseDragY;
    g_Camera.GetMouseDrag(mouseDragX, mouseDragY);
    bool glassMoved = false;
    if(mouseDragX != 0 || mouseDragY != 0)
        glassMoved = true;
    if(glassMoved) {
        g_Sim->m_CurrentGlassPosition.x += 100.0f * mouseDragX;
        if(g_MoveHorizontally)
            g_Sim->m_CurrentGlassPosition.z -= 100.0f * mouseDragY;
        else
            g_Sim->m_CurrentGlassPosition.y -= 100.0f * mouseDragY;
    }
    if(!g_StopSim) {
        for (int i = 0; i < FP_SIMULATION_STEPS_PER_FRAME; i++) {
            g_Sim->Update(ElapsedTime * g_TimeFactor / FP_SIMULATION_STEPS_PER_FRAME);
        }
        g_UpdateVis = true;
    }
    if(g_RenderType == FP_GUI_RENDERTYPE_MARCHING_CUBES && g_UpdateVis) {
        g_CPUDensityGrid->ConstructFromFluid();
        g_RenderMarchingCubes->ConstructMesh();
    } else if (g_RenderType == FP_GUI_RENDERTYPE_RAYTRACE && glassMoved) {
        D3DXVECTOR3 volumeStartPos = CalcRaytraceVolumeStartPos(g_Sim, g_RenderRaytrace);
        g_RenderRaytrace->SetVolumeStartPos(&volumeStartPos);
    }
}

//--------------------------------------------------------------------------------------
// Handle messages to the application
//--------------------------------------------------------------------------------------
LRESULT CALLBACK FP_MsgProc(
        HWND hWnd,
        UINT uMsg,
        WPARAM wParam,
        LPARAM lParam,
        bool* NoFurtherProcessing,
        void* UserContext ) {
    // Pass messages to GUI instance
    if(g_GUI.MsgProc(hWnd, uMsg, wParam, lParam, NoFurtherProcessing, g_ActiveLight))
        return 0;

    // Pass all remaining windows messages to camera so it can respond to user input
    g_Camera.HandleMessages( hWnd, uMsg, wParam, lParam );

    return 0;
}


//--------------------------------------------------------------------------------------
// Handle key presses
//--------------------------------------------------------------------------------------
void CALLBACK FP_OnKeyboard( UINT Char, bool KeyDown, bool AltDown, void* UserContext ){
    if( KeyDown ) {
        switch( Char ) {
            case VK_F1: g_GUI.m_ShowHelp = !g_GUI.m_ShowHelp; break;
        }
    }
}


//--------------------------------------------------------------------------------------
// Handles the GUI events
//--------------------------------------------------------------------------------------
void CALLBACK FP_OnGUIEvent(
        UINT Event,
        int ControlID,
        CDXUTControl* Control,
        void* UserContext ) {
    bool resetSim;
    float mcVoxelSize = -1.0f, mcIsoLevel = -1.0f, raytraceIsoLevel = -1.0f,
            raytraceStepScale = -1.0f, raytraceRefractionRatio = -1.0f, timeFactor = -1.0f;
    int selectedCubeMap = -1;
    float oldSpriteSize = g_RenderSprites->GetSpriteSize();
    float newSpriteSize = oldSpriteSize;
    int oldRenderType = g_RenderType;
    g_GUI.OnGUIEvent(Event, ControlID, Control, g_ActiveLight, g_NumActiveLights,
            raytraceIsoLevel, raytraceStepScale, raytraceRefractionRatio, mcVoxelSize,
            mcIsoLevel, g_LightScale, newSpriteSize, resetSim, g_StopSim,
            g_MoveHorizontally, g_RenderType, selectedCubeMap, timeFactor);
    if(oldRenderType != g_RenderType)
        g_UpdateVis = true;
    if(oldSpriteSize != newSpriteSize)
        g_RenderSprites->SetSpriteSize(newSpriteSize);
    if(selectedCubeMap >= 0)
        g_RenderRaytrace->m_CurrentCubeMap = selectedCubeMap;
    if(raytraceIsoLevel > 0.0f)
        g_RenderRaytrace->SetIsoLevel(raytraceIsoLevel);
    if(raytraceStepScale > 0.0f)
        g_RenderRaytrace->SetStepScale(raytraceStepScale);
    if(raytraceRefractionRatio > 0.0f)
        g_RenderRaytrace->SetRefractionRatio(raytraceRefractionRatio);
    if(mcIsoLevel > 0.0f)
        g_RenderMarchingCubes->m_IsoLevel = mcIsoLevel;
    if(mcVoxelSize > 0.0f && mcVoxelSize != g_CPUDensityGrid->m_VoxelSize)
        g_CPUDensityGrid->SetVoxelSize(mcVoxelSize);
    if(timeFactor > 0.0f)
        g_TimeFactor = timeFactor;
    g_RenderMarchingCubes->m_NumActiveLights = g_NumActiveLights;
    if(resetSim) {
        g_UpdateVis = true;
		mcVoxelSize = g_CPUDensityGrid->m_VoxelSize;
        delete g_Sim;
        delete g_CPUDensityGrid;
        D3DXVECTOR3 center(0.0f, 0.0f, 0.0f);
        g_Sim = new fp_Fluid(&g_WorkerThreadMgr, FP_NUM_PARTICLES_X, FP_NUM_PARTICLES_Y,
            FP_NUM_PARTICLES_Z, FP_PARTICLE_SPACING_X, FP_PARTICLE_SPACING_Y,
            FP_PARTICLE_SPACING_Z, center, FP_GLASS_RADIUS, FP_GLASS_FLOOR);
        g_CPUDensityGrid = new fp_CPUDensityGrid(g_Sim, mcVoxelSize);
        g_RenderSprites->m_Particles = g_Sim->m_Particles;
        g_RenderMarchingCubes->m_DensityGrid = g_CPUDensityGrid;
        g_RenderRaytrace->SetFluid(g_Sim);
        D3DXVECTOR3 volumeStartPos = CalcRaytraceVolumeStartPos(g_Sim, g_RenderRaytrace);
        g_RenderRaytrace->SetVolumeStartPos(&volumeStartPos);
    }
}


//--------------------------------------------------------------------------------------
// Reject any D3D10 devices that aren't acceptable by returning false
//--------------------------------------------------------------------------------------
bool CALLBACK FP_IsD3D10DeviceAcceptable(
        UINT Adapter,
        UINT Output,
        D3D10_DRIVER_TYPE DeviceType,
        DXGI_FORMAT BackBufferFormat,
        bool Windowed,
        void* UserContext ) {
    return true;
}


//--------------------------------------------------------------------------------------
// Create any D3D10 resources that aren't dependent on the back buffer
//--------------------------------------------------------------------------------------
HRESULT CALLBACK FP_OnD3D10CreateDevice(
        ID3D10Device* D3DDevice,
        const DXGI_SURFACE_DESC *BackBufferSurfaceDesc,
        void* UserContext ) {
    //HRESULT hr;

    g_GUI.OnD3D10CreateDevice(D3DDevice, BackBufferSurfaceDesc, UserContext);

    DWORD dwShaderFlags = D3D10_SHADER_ENABLE_STRICTNESS;
    #if defined( DEBUG ) || defined( _DEBUG )
    // Set the D3D10_SHADER_DEBUG flag to embed debug information in the shaders.
    // Setting this flag improves the shader debugging experience, but still allows
    // the shaders to be optimized and to run exactly the way they will run in
    // the release configuration of this program.
    dwShaderFlags |= D3D10_SHADER_DEBUG;
    #endif


    g_RenderSprites->OnD3D10CreateDevice(D3DDevice, BackBufferSurfaceDesc,
            UserContext);
    g_RenderMarchingCubes->OnD3D10CreateDevice(D3DDevice, BackBufferSurfaceDesc,
            UserContext);
    g_RenderRaytrace->OnD3D10CreateDevice(D3DDevice, BackBufferSurfaceDesc,
        UserContext);
    g_DepthPeeler->OnD3D10CreateDevice(D3DDevice, BackBufferSurfaceDesc, UserContext);
    g_GUI.SetCubeMapNames(&g_RenderRaytrace->m_CubeMapNames,
            g_RenderRaytrace->m_CurrentCubeMap);

    // Setup the camera's view parameters
    D3DXVECTOR3 vecEye(0.0f, 0.0f, -15.0f);
    D3DXVECTOR3 vecAt (0.0f, 0.0f, -0.0f);
    g_Camera.SetViewParams( &vecEye, &vecAt );
    g_Camera.SetRadius(FP_OBJECT_RADIUS*3.0f, FP_OBJECT_RADIUS*0.5f,
            FP_OBJECT_RADIUS*100.0f );

    return S_OK;
}


//--------------------------------------------------------------------------------------
// Create any D3D10 resources that depend on the back buffer
//--------------------------------------------------------------------------------------
HRESULT CALLBACK FP_OnD3D10ResizedSwapChain(
        ID3D10Device* D3DDevice,
        IDXGISwapChain *SwapChain,
        const DXGI_SURFACE_DESC* BackBufferSurfaceDesc,
        void* UserContext ) {
    g_GUI.OnD3D10ResizedSwapChain(D3DDevice, SwapChain, BackBufferSurfaceDesc,
            UserContext);
    g_RenderSprites->OnD3D10ResizedSwapChain(D3DDevice, SwapChain, BackBufferSurfaceDesc,
            UserContext);
    g_RenderMarchingCubes->OnD3D10ResizedSwapChain(D3DDevice, SwapChain, BackBufferSurfaceDesc,
            UserContext);
    g_RenderRaytrace->OnD3D10ResizedSwapChain(D3DDevice, SwapChain, BackBufferSurfaceDesc,
        UserContext);
    g_DepthPeeler->OnD3D10ResizedSwapChain(D3DDevice, SwapChain, BackBufferSurfaceDesc,
        UserContext);

    // Setup the camera's projection parameters
    float fAspectRatio = BackBufferSurfaceDesc->Width / (float)BackBufferSurfaceDesc
            ->Height;
    g_Camera.SetProjParams( D3DX_PI/4, fAspectRatio, 2.0f, 4000.0f );
    g_Camera.SetWindow( BackBufferSurfaceDesc->Width, BackBufferSurfaceDesc->Height );
    g_Camera.SetButtonMasks( MOUSE_LEFT_BUTTON, MOUSE_WHEEL, MOUSE_MIDDLE_BUTTON );

    return S_OK;
}


//--------------------------------------------------------------------------------------
// Render the scene using the D3D10 device
//--------------------------------------------------------------------------------------
void CALLBACK FP_OnD3D10FrameRender(
        ID3D10Device* D3DDevice,
        double Time,
        float ElapsedTime,
        void* UserContext ) {
    //HRESULT hr;

    // If the settings dialog is being shown, then render it instead of rendering the
    // app's scene
    if( g_GUI.RenderSettingsDialog(ElapsedTime) )
        return;

    // Clear the render target and depth stencil
    float ClearColor[4] = { FP_CLEAR_COLOR };
    ID3D10RenderTargetView* pRTV = DXUTGetD3D10RenderTargetView();
    D3DDevice->ClearRenderTargetView( pRTV, ClearColor );
    ID3D10DepthStencilView* pDSV = DXUTGetD3D10DepthStencilView();
    D3DDevice->ClearDepthStencilView( pDSV, D3D10_CLEAR_DEPTH, 1.0, 0 );

    D3DXMATRIX  view;
    D3DXMATRIX  projection;
    D3DXMATRIX  worldView;
	D3DXMATRIX  viewProjection;
    D3DXMATRIX  worldViewProjection;
	D3DXMATRIX  invView;

    // Get the projection & view matrix from the camera class
    view = *g_Camera.GetViewMatrix();
    projection = *g_Camera.GetProjMatrix();
    viewProjection = view * projection;
	D3DXMatrixInverse(&invView, NULL, &view);

    for (int i=0; i < FP_MAX_LIGHTS; i++) {
        g_RenderMarchingCubes->m_Lights9[i].Direction = g_GUI.m_LightDir[i];
        D3DCOLORVALUE diffuse = { g_GUI.m_LightDiffuseColor->r,
            g_GUI.m_LightDiffuseColor->g, g_GUI.m_LightDiffuseColor->b, 1.0f };
        g_RenderMarchingCubes->m_Lights9[i].Diffuse = diffuse;
    }

    // For depth peeling use this:
    //g_DepthPeeler->OnD3D10FrameRender(D3DDevice, &viewProjection);

    if(g_RenderType == FP_GUI_RENDERTYPE_SPRITE)
        g_RenderSprites->OnD3D10FrameRender(D3DDevice, &viewProjection, &invView);
    else if(g_RenderType == FP_GUI_RENDERTYPE_MARCHING_CUBES)
        g_RenderMarchingCubes->OnD3D10FrameRender(D3DDevice, &viewProjection);
    else if(g_RenderType == FP_GUI_RENDERTYPE_RAYTRACE)
        g_RenderRaytrace->OnD3D10FrameRender(D3DDevice, &view, &projection,
                &viewProjection, &invView, g_UpdateVis);

    g_UpdateVis = false;

    // Render GUI
    g_GUI.OnD3D10FrameRender(D3DDevice, ElapsedTime, g_Camera.GetEyePt(),
            &view, &projection, g_NumActiveLights,
            g_ActiveLight, g_LightScale);
}


//--------------------------------------------------------------------------------------
// Release D3D10 resources created in OnD3D10ResizedSwapChain
//--------------------------------------------------------------------------------------
void CALLBACK FP_OnD3D10ReleasingSwapChain( void* UserContext ) {
    g_GUI.OnD3D10ReleasingSwapChain(UserContext);
    if(g_RenderSprites) g_RenderSprites->OnD3D10ReleasingSwapChain(UserContext);
    if(g_RenderMarchingCubes) g_RenderMarchingCubes->OnD3D10ReleasingSwapChain(
            UserContext);
    if(g_RenderRaytrace) g_RenderRaytrace->OnD3D10ReleasingSwapChain(UserContext);
    if(g_DepthPeeler) g_DepthPeeler->OnD3D10ReleasingSwapChain(UserContext);
}


//--------------------------------------------------------------------------------------
// Release D3D10 resources created in OnD3D10CreateDevice
//--------------------------------------------------------------------------------------
void CALLBACK FP_OnD3D10DestroyDevice( void* UserContext ) {
    g_GUI.OnD3D10DestroyDevice(UserContext);
    if(g_RenderSprites) g_RenderSprites->OnD3D10DestroyDevice(UserContext);
    if(g_RenderMarchingCubes) g_RenderMarchingCubes->OnD3D10DestroyDevice(UserContext);
    if(g_RenderRaytrace) g_RenderRaytrace->OnD3D10DestroyDevice(UserContext);
    if(g_DepthPeeler) g_DepthPeeler->OnD3D10DestroyDevice(UserContext);
    DXUTGetGlobalResourceCache().OnDestroyDevice();
}

D3DXVECTOR3 CalcRaytraceVolumeStartPos(
        fp_Fluid* Fluid,
        fp_RenderRaytrace* RenderRaytrace) {
    D3DXVECTOR3 result = Fluid->m_CurrentGlassPosition;
    result.y += Fluid->m_GlassFloor - FP_RAYTRACE_VOLUME_BORDER;
    D3DXVECTOR3 volumeSize = RenderRaytrace->GetVolumeSize();
    result.x -= volumeSize.x / 2.0f;
    result.z -= volumeSize.z / 2.0f;
    return result;
}
