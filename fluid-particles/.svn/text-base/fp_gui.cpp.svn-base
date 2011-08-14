#include "DXUT.h"
#include "fp_gui.h"

extern void CALLBACK FP_OnGUIEvent(
        UINT nEvent,
        int nControlID,
        CDXUTControl* pControl,
        void* pUserContext );


void fp_GUI::SetCubeMapNames(fp_StringList* CubeMapNames, int CurrentCubeMap) {
    CDXUTComboBox* comboBox = m_SampleUI.GetComboBox(IDC_RAYTRACE_SELECT_CUBEMAP);
    for(int i=0, size=CubeMapNames->size(); i < size; i++) {
        WCHAR cubeMapName[MAX_PATH];
        StringCchPrintf(cubeMapName, MAX_PATH, L"%s (C)", (*CubeMapNames)[i].c_str());
        comboBox->AddItem(cubeMapName, (LPVOID)i);
    }
    comboBox->SetSelectedByIndex(CurrentCubeMap);
}

//--------------------------------------------------------------------------------------
// Constructor
//--------------------------------------------------------------------------------------
fp_GUI::fp_GUI()
        : m_ShowHelp(false), m_Font10(NULL), m_Font9(NULL), m_Sprite10(NULL),
        m_Sprite9(NULL), m_TxtHelper(NULL) {
    
    for( int i=0; i<FP_MAX_LIGHTS; i++ ) {
        D3DXVECTOR3 direction(
            sinf(D3DX_PI*2*i/FP_MAX_LIGHTS-D3DX_PI/6),
            0,
            -cosf(D3DX_PI*2*i/FP_MAX_LIGHTS-D3DX_PI/6));
        m_LightControl[i].SetLightDirection(direction);
        m_LightDir[i] = direction;
        m_LightDiffuseColor[i] = D3DXCOLOR(1,1,1,1);
    }

    m_D3DSettingsDlg.Init( &m_DialogResourceManager );
    m_HUD.Init( &m_DialogResourceManager );
    m_SampleUI.Init( &m_DialogResourceManager );

    int iYCommon = 10;

    m_HUD.SetCallback( FP_OnGUIEvent );
    m_HUD.AddButton( IDC_TOGGLEFULLSCREEN, L"Toggle full screen (F4)", 35, iYCommon, 125,
            22, VK_F4);
    m_HUD.AddButton( IDC_TOGGLEREF, L"Toggle REF (F3)", 35, iYCommon += 24, 125, 22, VK_F3 );
    m_HUD.AddButton( IDC_CHANGEDEVICE, L"Change device (F2)", 35, iYCommon += 24, 125, 22,
            VK_F2 );

    m_SampleUI.SetCallback( FP_OnGUIEvent );
    
    iYCommon = 10; 

    WCHAR sz[100];
    iYCommon += 24;

    m_SampleUI.AddCheckBox(IDC_STOP_SIM, L"Stop simulation (S)", 35, iYCommon += 24, 125,
            24, false, 'S');

    StringCchPrintf( sz, 100, L"Time factor: %0.1f", FP_DEFAULT_TIME_FACTOR);
    m_SampleUI.AddStatic( IDC_TIME_FACTOR_STATIC, sz, 35, iYCommon += 24, 125, 22);
    m_SampleUI.AddSlider( IDC_TIME_FACTOR, 50, iYCommon += 24, 100, 22, 1, 100,
        (int) (FP_DEFAULT_TIME_FACTOR * 10.0f));

    m_SampleUI.AddButton( IDC_RESET_SIM, L"Reset simulation (R)", 35, iYCommon += 24,
            125, 22, 'R' );    

    m_SampleUI.AddCheckBox(IDC_MOVE_HORIZONTALLY, L"Move horizontally (H)", 35,
            iYCommon += 24, 125, 24, false, 'H');

    CDXUTComboBox *comboBox;
    m_SampleUI.AddComboBox( IDC_SELECT_RENDER_TYPE, 35, iYCommon += 24, 125, 24, L'T',
            false, &comboBox );
    if( comboBox ) {
        comboBox->SetDropHeight( 100 );
        comboBox->AddItem( L"Raytrace (T)", (LPVOID)FP_GUI_RENDERTYPE_RAYTRACE );
        comboBox->AddItem( L"Marching cubes (T)",
                (LPVOID)FP_GUI_RENDERTYPE_MARCHING_CUBES );
        comboBox->AddItem( L"Point sprites (T)", (LPVOID)FP_GUI_RENDERTYPE_SPRITE );        
    }

    int iYRaytrace, iYMC, iYSprite;
    iYRaytrace = iYMC = iYSprite = iYCommon;

    // Raytrace controls
    m_RaytraceSpecificControls.push_back(NULL);
    m_SampleUI.AddComboBox( IDC_RAYTRACE_SELECT_CUBEMAP, 35, iYRaytrace += 24, 125, 24,
            L'C', false, (CDXUTComboBox**) &m_RaytraceSpecificControls.back() );

    iYRaytrace += 24;
    StringCchPrintf( sz, 100, L"Iso level: %0.2f", FP_RAYTRACE_DEFAULT_ISO_LEVEL ); 
    m_RaytraceSpecificControls.push_back(NULL);
    m_SampleUI.AddStatic( IDC_RAYTRACE_ISO_LEVEL_STATIC, sz, 35, iYRaytrace += 24, 125, 22,
            false, (CDXUTStatic**)&m_RaytraceSpecificControls.back());
    m_RaytraceSpecificControls.push_back(NULL);
    m_SampleUI.AddSlider( IDC_RAYTRACE_ISO_LEVEL, 50, iYRaytrace += 24, 100, 22, 1, 200,
            (int) (FP_RAYTRACE_DEFAULT_ISO_LEVEL * 250.0f), false, (CDXUTSlider**)
            &m_RaytraceSpecificControls.back() );

    iYRaytrace += 24;
    StringCchPrintf( sz, 100, L"Step scale: %0.1f", FP_RAYTRACE_DEFAULT_STEP_SCALE ); 
    m_RaytraceSpecificControls.push_back(NULL);
    m_SampleUI.AddStatic( IDC_RAYTRACE_STEP_SCALE_STATIC, sz, 35, iYRaytrace += 24, 125, 22,
            false, (CDXUTStatic**)&m_RaytraceSpecificControls.back());
    m_RaytraceSpecificControls.push_back(NULL);
    m_SampleUI.AddSlider( IDC_RAYTRACE_STEP_SCALE, 50, iYRaytrace += 24, 100, 22, 8, 100,
            (int) (FP_RAYTRACE_DEFAULT_STEP_SCALE * 10.0f), false, (CDXUTSlider**)
            &m_RaytraceSpecificControls.back() );

    iYRaytrace += 24;
    StringCchPrintf( sz, 100, L"Refraction ratio: %0.2f", 
            FP_RAYTRACE_DEFAULT_REFRACTION_RATIO); 
    m_RaytraceSpecificControls.push_back(NULL);
    m_SampleUI.AddStatic( IDC_RAYTRACE_REFRACTION_RATIO_STATIC, sz, 35, iYRaytrace += 24,
            125, 22, false, (CDXUTStatic**)&m_RaytraceSpecificControls.back());
    m_RaytraceSpecificControls.push_back(NULL);
    m_SampleUI.AddSlider( IDC_RAYTRACE_REFRACTION_RATIO, 50, iYRaytrace += 24, 100, 22, 1, 
            99, (int) (FP_RAYTRACE_DEFAULT_REFRACTION_RATIO * 100.0f), false,
            (CDXUTSlider**) &m_RaytraceSpecificControls.back() );

    // Marching cubes controls

    iYMC += 24;    
    StringCchPrintf( sz, 100, L"Voxel size: %0.1f", FP_MC_DEFAULT_DENSITY_GRID_VOXELSIZE ); 
    m_MCSpecificControls.push_back(NULL);
    m_SampleUI.AddStatic( IDC_MC_VOXEL_SIZE_STATIC, sz, 35, iYMC += 24, 125, 22, false,
            (CDXUTStatic**)&m_MCSpecificControls.back());
    m_MCSpecificControls.push_back(NULL);
    m_SampleUI.AddSlider( IDC_MC_VOXEL_SIZE, 50, iYMC += 24, 100, 22, 1, 100,
            (int) (FP_MC_DEFAULT_DENSITY_GRID_VOXELSIZE * 10.0f), false, (CDXUTSlider**)
            &m_MCSpecificControls.back());
    
    iYMC += 24;
    StringCchPrintf( sz, 100, L"Iso level: %0.3f", FP_MC_DEFAULT_ISO_LEVEL ); 
    m_MCSpecificControls.push_back(NULL);
    m_SampleUI.AddStatic( IDC_MC_ISO_LEVEL_STATIC, sz, 35, iYMC += 24, 125, 22, false,
            (CDXUTStatic**)&m_MCSpecificControls.back());
    m_MCSpecificControls.push_back(NULL);
    m_SampleUI.AddSlider( IDC_MC_ISO_LEVEL, 50, iYMC += 24, 100, 22, 1, 100,
            (int) (FP_MC_DEFAULT_ISO_LEVEL * 500.0f), false, (CDXUTSlider**)
            &m_MCSpecificControls.back() );

    iYMC += 24;
    StringCchPrintf( sz, 100, L"# Lights: %d", 1 ); 
    m_MCSpecificControls.push_back(NULL);
    m_SampleUI.AddStatic( IDC_NUM_LIGHTS_STATIC, sz, 35, iYMC += 24, 125, 22, false,
            (CDXUTStatic**)&m_MCSpecificControls.back());
    m_MCSpecificControls.push_back(NULL);
    m_SampleUI.AddSlider( IDC_NUM_LIGHTS, 50, iYMC += 24, 100, 22, 1, FP_MAX_LIGHTS, 1,
            false, (CDXUTSlider**)&m_MCSpecificControls.back() );

    iYMC += 24;
    StringCchPrintf( sz, 100, L"Light scale: %0.2f", FP_DEFAULT_LIGHT_SCALE ); 
    m_MCSpecificControls.push_back(NULL);
    m_SampleUI.AddStatic( IDC_LIGHT_SCALE_STATIC, sz, 35, iYMC += 24, 125, 22, false,
            (CDXUTStatic**)&m_MCSpecificControls.back() );
    m_MCSpecificControls.push_back(NULL);
    m_SampleUI.AddSlider( IDC_LIGHT_SCALE, 50, iYMC += 24, 100, 22, 0, 20,
            (int) (FP_DEFAULT_LIGHT_SCALE * 10.0f), false, (CDXUTSlider**)
            &m_MCSpecificControls.back() );

    iYMC += 24;
    m_MCSpecificControls.push_back(NULL);
    m_SampleUI.AddButton( IDC_ACTIVE_LIGHT, L"Change active light (K)", 35, iYMC += 24,
            125, 22, 'K', false, (CDXUTButton**)&m_MCSpecificControls.back() );

    // Sprite controls

    iYSprite += 24;
    StringCchPrintf( sz, 100, L"Sprite size: %0.2f", FP_RENDER_DEFAULT_SPRITE_SIZE );
    m_SpriteSpecificControls.push_back(NULL);
    m_SampleUI.AddStatic( IDC_SPRITE_PARTICLE_SCALE_STATIC, sz, 35, iYSprite += 24, 125, 22,
            false, (CDXUTStatic**)&m_SpriteSpecificControls.back());
    m_SpriteSpecificControls.push_back(NULL);
    m_SampleUI.AddSlider( IDC_SPRITE_PARTICLE_SCALE, 50, iYSprite += 24, 100, 22, 1, 100,
            (int) (FP_RENDER_DEFAULT_SPRITE_SIZE * 50.0f), false, (CDXUTSlider**)
            &m_SpriteSpecificControls.back() );
}

bool fp_GUI::RenderSettingsDialog(float ElapsedTime) {
    if( !m_D3DSettingsDlg.IsActive() )
        return false;
    m_D3DSettingsDlg.OnRender( ElapsedTime );
    return true;   
}

//--------------------------------------------------------------------------------------
// Handle messages to the application
//--------------------------------------------------------------------------------------
bool fp_GUI::MsgProc(
        HWND Wnd,
        UINT Msg,
        WPARAM wParam,
        LPARAM lParam,
        bool* NoFurtherProcessing,
        int ActiveLight) {
    // Pass messages to dialog resource manager calls so GUI state is updated correctly
    *NoFurtherProcessing = m_DialogResourceManager.MsgProc( Wnd, Msg, wParam,
            lParam );
    if( *NoFurtherProcessing )
        return true;

    // Pass messages to settings dialog if its active
    if( m_D3DSettingsDlg.IsActive() ) {
        m_D3DSettingsDlg.MsgProc( Wnd, Msg, wParam, lParam );
        return true;
    }

    // Give the dialogs a chance to handle the message first
    *NoFurtherProcessing = m_HUD.MsgProc( Wnd, Msg, wParam, lParam );
    if( *NoFurtherProcessing )
        return true;
    *NoFurtherProcessing = m_SampleUI.MsgProc( Wnd, Msg, wParam, lParam );
    if( *NoFurtherProcessing )
        return true;

    m_LightControl[ActiveLight].HandleMessages( Wnd, Msg, wParam, lParam );

    return false;
}


//--------------------------------------------------------------------------------------
// Handle key presses
//--------------------------------------------------------------------------------------
void fp_GUI::OnKeyboard( UINT Char, bool KeyDown, bool AltDown, void* UserContext ){
    if( KeyDown ) {
        switch( Char ) {
            case VK_F1: m_ShowHelp = !m_ShowHelp; break;
        }
    }
}


//--------------------------------------------------------------------------------------
// Handles the GUI events
//--------------------------------------------------------------------------------------
void fp_GUI::OnGUIEvent(
        UINT Event,
        int ControlID,
        CDXUTControl* Control,
        int& ActiveLight,
        int& NumActiveLights,
        float& RaytraceIsoLevel,
        float& RaytraceStepScale,
        float& RaytraceRefractionRatio,
        float& MCVoxelSize,
        float& MCIsoLevel,
        float& LightScale,        
        float& ParticleScale,
        bool& ResetSim,
        bool& StopSim,
        bool& MoveHorizontally,
        int& RenderType,
        int& SelectedCubeMap,
        float& TimeFactor) {   
    WCHAR sz[100];
    ResetSim = false;
    switch( ControlID ) {
        case IDC_TOGGLEFULLSCREEN: DXUTToggleFullScreen(); break;
        case IDC_TOGGLEREF:        DXUTToggleREF(); break;
        case IDC_CHANGEDEVICE:     m_D3DSettingsDlg.SetActive(
                !m_D3DSettingsDlg.IsActive() ); break;

        case IDC_RESET_SIM:
            ResetSim = true;
            break;

        case IDC_SELECT_RENDER_TYPE: {
            DXUTComboBoxItem *item = ((CDXUTComboBox*)Control)->GetSelectedItem();
            if( item ) 
                #pragma warning( disable : 4311)
                RenderType = (int)item->pData;
                #pragma warning( default : 4311)
            break;
        }

        case IDC_TIME_FACTOR: 
            TimeFactor = (float) (m_SampleUI.GetSlider(
                IDC_TIME_FACTOR )->GetValue() * 0.1f);

            StringCchPrintf( sz, 100, L"Time factor: %0.1f", TimeFactor); 
            m_SampleUI.GetStatic( IDC_TIME_FACTOR_STATIC )->SetText( sz );
            break;

        case IDC_STOP_SIM: 
            StopSim = m_SampleUI.GetCheckBox(IDC_STOP_SIM)->GetChecked();
            break;   

        case IDC_MOVE_HORIZONTALLY: 
            MoveHorizontally = m_SampleUI.GetCheckBox(IDC_MOVE_HORIZONTALLY)->GetChecked();
            break;    

        case IDC_RAYTRACE_SELECT_CUBEMAP: {
            DXUTComboBoxItem *item = ((CDXUTComboBox*)Control)->GetSelectedItem();
            if( item ) 
                #pragma warning( disable : 4311)
                SelectedCubeMap = (int)item->pData;
                #pragma warning( default : 4311)
            break;
        }

        case IDC_RAYTRACE_ISO_LEVEL: 
            RaytraceIsoLevel = (float) (m_SampleUI.GetSlider(
                IDC_RAYTRACE_ISO_LEVEL )->GetValue() * 0.004f);

            StringCchPrintf( sz, 100, L"Iso level: %0.3f", RaytraceIsoLevel ); 
            m_SampleUI.GetStatic( IDC_RAYTRACE_ISO_LEVEL_STATIC )->SetText( sz );
            break;

        case IDC_RAYTRACE_STEP_SCALE: 
            RaytraceStepScale = (float) (m_SampleUI.GetSlider(
                IDC_RAYTRACE_STEP_SCALE)->GetValue() * 0.1f);

            StringCchPrintf( sz, 100, L"Step scale: %0.1f", RaytraceStepScale); 
            m_SampleUI.GetStatic( IDC_RAYTRACE_STEP_SCALE_STATIC )->SetText( sz );
            break;

        case IDC_RAYTRACE_REFRACTION_RATIO: 
            RaytraceRefractionRatio = (float) (m_SampleUI.GetSlider(
                    IDC_RAYTRACE_REFRACTION_RATIO)->GetValue() * 0.01f);

            StringCchPrintf( sz, 100, L"Refraction ratio: %0.2f", RaytraceRefractionRatio); 
            m_SampleUI.GetStatic( IDC_RAYTRACE_REFRACTION_RATIO_STATIC )->SetText( sz );
            break;

        case IDC_MC_VOXEL_SIZE: 
            MCVoxelSize = (float) (m_SampleUI.GetSlider(
                IDC_MC_VOXEL_SIZE )->GetValue() * 0.1f);

            StringCchPrintf( sz, 100, L"Voxel size: %0.1f", MCVoxelSize ); 
            m_SampleUI.GetStatic( IDC_MC_VOXEL_SIZE_STATIC )->SetText( sz );
            break;

        case IDC_MC_ISO_LEVEL: 
            MCIsoLevel = (float) (m_SampleUI.GetSlider(
                IDC_MC_ISO_LEVEL )->GetValue() * 0.002f);

            StringCchPrintf( sz, 100, L"Iso level: %0.3f", MCIsoLevel ); 
            m_SampleUI.GetStatic( IDC_MC_ISO_LEVEL_STATIC )->SetText( sz );
            break;

        case IDC_SPRITE_PARTICLE_SCALE: 
            ParticleScale = (float) (m_SampleUI.GetSlider(
                    IDC_SPRITE_PARTICLE_SCALE )->GetValue() * 0.02f);

            StringCchPrintf( sz, 100, L"Sprite size: %0.2f", ParticleScale ); 
            m_SampleUI.GetStatic( IDC_SPRITE_PARTICLE_SCALE_STATIC )->SetText( sz );
            break;

        case IDC_ACTIVE_LIGHT:
            if( !m_LightControl[ActiveLight].IsBeingDragged() ) {
                ActiveLight++;
                ActiveLight %= NumActiveLights;
            }
            break;

        case IDC_NUM_LIGHTS:
            if( !m_LightControl[ActiveLight].IsBeingDragged() ) {
                StringCchPrintf( sz, 100, L"# Lights: %d", m_SampleUI.GetSlider(
                        IDC_NUM_LIGHTS )->GetValue() ); 
                m_SampleUI.GetStatic( IDC_NUM_LIGHTS_STATIC )->SetText( sz );

                NumActiveLights = m_SampleUI.GetSlider(
                        IDC_NUM_LIGHTS )->GetValue();
                ActiveLight %= NumActiveLights;
            }
            break;

        case IDC_LIGHT_SCALE: 
            LightScale = (float) (m_SampleUI.GetSlider(
                    IDC_LIGHT_SCALE )->GetValue() * 0.10f);

            StringCchPrintf( sz, 100, L"Light scale: %0.2f", LightScale ); 
            m_SampleUI.GetStatic( IDC_LIGHT_SCALE_STATIC )->SetText( sz );
            break;
    }
    
}



//--------------------------------------------------------------------------------------
// DX9 specific
//--------------------------------------------------------------------------------------



//--------------------------------------------------------------------------------------
// Create any D3D10 GUI resources that aren't dependant on the back buffer
//--------------------------------------------------------------------------------------
HRESULT fp_GUI::OnD3D10CreateDevice(
        ID3D10Device* D3DDevice,
        const DXGI_SURFACE_DESC *BackBufferSurfaceDesc,
        void* UserContext ) {
    HRESULT hr;

    V_RETURN( m_DialogResourceManager.OnD3D10CreateDevice( D3DDevice ) );
    V_RETURN( m_D3DSettingsDlg.OnD3D10CreateDevice( D3DDevice ) );
    V_RETURN( D3DX10CreateFont( D3DDevice, 15, 0, FW_BOLD, 1, FALSE, DEFAULT_CHARSET, 
            OUT_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, L"Arial",
            &m_Font10 ) );
    V_RETURN( D3DX10CreateSprite( D3DDevice, 512, &m_Sprite10 ) );
    m_TxtHelper = new CDXUTTextHelper( m_Font9, m_Sprite9, m_Font10, m_Sprite10,
            15 );
    
    V_RETURN( CDXUTDirectionWidget::StaticOnD3D10CreateDevice( D3DDevice ) );
    for( int i=0; i<FP_MAX_LIGHTS; i++ )
        m_LightControl[i].SetRadius( FP_OBJECT_RADIUS );

    return S_OK;
}


//--------------------------------------------------------------------------------------
// Create any D3D10 GUI resources that depend on the back buffer
//--------------------------------------------------------------------------------------
HRESULT fp_GUI::OnD3D10ResizedSwapChain(
        ID3D10Device* D3DDevice,
        IDXGISwapChain *SwapChain,
        const DXGI_SURFACE_DESC* BackBufferSurfaceDesc,
        void* UserContext ) {
    HRESULT hr;

    V_RETURN( m_DialogResourceManager.OnD3D10ResizedSwapChain( D3DDevice,
            BackBufferSurfaceDesc ) );
    V_RETURN( m_D3DSettingsDlg.OnD3D10ResizedSwapChain( D3DDevice,
            BackBufferSurfaceDesc ) );

    m_HUD.SetLocation( BackBufferSurfaceDesc->Width-170, 0 );
    m_HUD.SetSize( 170, 170 );
    m_SampleUI.SetLocation( BackBufferSurfaceDesc->Width-170,
            BackBufferSurfaceDesc->Height-FP_GUI_HEIGHT );
    m_SampleUI.SetSize( 170, FP_GUI_HEIGHT );

    return S_OK;
}

//--------------------------------------------------------------------------------------
// Render the GUI using the D3D10 device
//--------------------------------------------------------------------------------------
void fp_GUI::OnD3D10FrameRender(
        ID3D10Device* D3DDevice,
        float ElapsedTime,
        const D3DXVECTOR3* EyePt,
        const D3DXMATRIX*  View,
        const D3DXMATRIX*  Proj,
        int NumActiveLights,
        int ActiveLight,
        float LightScale) {
    HRESULT hr;
    
    int renderType = ChooseControls();
    if(renderType == (int)FP_GUI_RENDERTYPE_MARCHING_CUBES)
        // Render the light arrow so the user can visually see the light dir
        for( int i=0; i<NumActiveLights; i++ ) {
            D3DXCOLOR arrowColor = ( i == ActiveLight )
                    ? D3DXVECTOR4(1,1,0,1)
                    : D3DXVECTOR4(1,1,1,1);
            V( m_LightControl[i].OnRender10( arrowColor, View, Proj, EyePt ) );
            m_LightDir[i] = m_LightControl[i].GetLightDirection();
            m_LightDiffuseColor[i] = LightScale * D3DXCOLOR(1,1,1,1);
        }

    DXUT_BeginPerfEvent( DXUT_PERFEVENTCOLOR, L"HUD / Stats" );
    m_HUD.OnRender( ElapsedTime );
    m_SampleUI.OnRender( ElapsedTime );
    RenderText();
    DXUT_EndPerfEvent();
}


//--------------------------------------------------------------------------------------
// Release D3D10 GUI resources created in OnD3D10ResizedSwapChain 
//--------------------------------------------------------------------------------------
void fp_GUI::OnD3D10ReleasingSwapChain( void* UserContext )
{
    m_DialogResourceManager.OnD3D10ReleasingSwapChain();
}


//--------------------------------------------------------------------------------------
// Release D3D10 GUI resources created in OnD3D10CreateDevice 
//--------------------------------------------------------------------------------------
void fp_GUI::OnD3D10DestroyDevice( void* UserContext )
{
    m_DialogResourceManager.OnD3D10DestroyDevice();
    m_D3DSettingsDlg.OnD3D10DestroyDevice();
    CDXUTDirectionWidget::StaticOnD3D10DestroyDevice();
    SAFE_DELETE( m_TxtHelper );
    SAFE_RELEASE( m_Font10 );
    SAFE_RELEASE( m_Sprite10 );
}


//--------------------------------------------------------------------------------------
// DX9 specific
//--------------------------------------------------------------------------------------


//--------------------------------------------------------------------------------------
// Create any D3D9 GUI resources that will live through a device reset (D3DPOOL_MANAGED)
// and aren't tied to the back buffer size 
//--------------------------------------------------------------------------------------
HRESULT fp_GUI::OnD3D9CreateDevice(
        IDirect3DDevice9* D3DDevice,
        const D3DSURFACE_DESC* BackBufferSurfaceDesc,
        void* UserContext ) {
    HRESULT hr;

    V_RETURN( m_DialogResourceManager.OnD3D9CreateDevice( D3DDevice ) );
    V_RETURN( m_D3DSettingsDlg.OnD3D9CreateDevice( D3DDevice ) );

    // Initialize the font
    V_RETURN( D3DXCreateFont( D3DDevice, 15, 0, FW_BOLD, 1, FALSE, DEFAULT_CHARSET, 
            OUT_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE,
            L"Arial", &m_Font9 ) );

    m_SampleUI.GetStatic( IDC_NUM_LIGHTS_STATIC )->SetVisible( true );
    m_SampleUI.GetSlider( IDC_NUM_LIGHTS )->SetVisible( true );
    m_SampleUI.GetButton( IDC_ACTIVE_LIGHT )->SetVisible( true );

    V_RETURN( CDXUTDirectionWidget::StaticOnD3D9CreateDevice( D3DDevice ) );
    for( int i=0; i<FP_MAX_LIGHTS; i++ )
        m_LightControl[i].SetRadius( FP_OBJECT_RADIUS );

    return S_OK;
}


//--------------------------------------------------------------------------------------
// Create any D3D9 GUI resources that won't live through a device reset
// (D3DPOOL_DEFAULT) or that are tied to the back buffer size 
//--------------------------------------------------------------------------------------
HRESULT fp_GUI::OnD3D9ResetDevice(
        IDirect3DDevice9* D3DDevice, 
        const D3DSURFACE_DESC* BackBufferSurfaceDesc,
        void* UserContext ) {
    HRESULT hr;

    V_RETURN( m_DialogResourceManager.OnD3D9ResetDevice() );
    V_RETURN( m_D3DSettingsDlg.OnD3D9ResetDevice() );

    if( m_Font9 ) V_RETURN( m_Font9->OnResetDevice() );

    // Create a sprite to help batch calls when drawing many lines of text
    V_RETURN( D3DXCreateSprite( D3DDevice, &m_Sprite9 ) );
    m_TxtHelper = new CDXUTTextHelper( m_Font9, m_Sprite9, NULL, NULL, 15 );

    for( int i=0; i<FP_MAX_LIGHTS; i++ )
        m_LightControl[i].OnD3D9ResetDevice( BackBufferSurfaceDesc  );

    m_HUD.SetLocation( BackBufferSurfaceDesc->Width-170, 0 );
    m_HUD.SetSize( 170, 170 );
    m_SampleUI.SetLocation( BackBufferSurfaceDesc->Width-170,
            BackBufferSurfaceDesc->Height-FP_GUI_HEIGHT );
    m_SampleUI.SetSize( 170, FP_GUI_HEIGHT );

    return S_OK;
}


//--------------------------------------------------------------------------------------
// Render the GUI using the D3D9 device
//--------------------------------------------------------------------------------------
void fp_GUI::OnD3D9FrameRender(
        IDirect3DDevice9* D3DDevice,
        float ElapsedTime,
        const D3DXVECTOR3* EyePt,
        const D3DXMATRIX*  View,
        const D3DXMATRIX*  Proj,
        int NumActiveLights,
        int ActiveLight,
        float LightScale) {
    HRESULT hr;
   
    // Render the GUI

    int renderType = ChooseControls();
    if(renderType == (int)FP_GUI_RENDERTYPE_MARCHING_CUBES)
        // Render the light arrow so the user can visually see the light dir
        for( int i=0; i<NumActiveLights; i++ ) {
            D3DXCOLOR arrowColor = ( i == ActiveLight ) ? D3DXCOLOR(1,1,0,1) : D3DXCOLOR(1,1,1,1);
            V( m_LightControl[i].OnRender9( arrowColor, View, Proj, EyePt ) );
            m_LightDir[i] = m_LightControl[i].GetLightDirection();
            m_LightDiffuseColor[i] = LightScale * D3DXCOLOR(1,1,1,1);
        }

    m_HUD.OnRender( ElapsedTime );    
    m_SampleUI.OnRender( ElapsedTime );

    RenderText();
}


//--------------------------------------------------------------------------------------
// Release D3D9 resources created in the OnD3D9ResetDevice callback 
//--------------------------------------------------------------------------------------
void fp_GUI::OnD3D9LostDevice( void* UserContext )
{
    m_DialogResourceManager.OnD3D9LostDevice();
    m_D3DSettingsDlg.OnD3D9LostDevice();
    CDXUTDirectionWidget::StaticOnD3D9LostDevice();
    if( m_Font9 ) m_Font9->OnLostDevice();
    SAFE_RELEASE(m_Sprite9);
    SAFE_DELETE(m_TxtHelper);
    
}


//--------------------------------------------------------------------------------------
// Release D3D9 GUI resources created in the OnD3D9CreateDevice callback 
//--------------------------------------------------------------------------------------
void fp_GUI::OnD3D9DestroyDevice( void* UserContext )
{
    m_DialogResourceManager.OnD3D9DestroyDevice();
    m_D3DSettingsDlg.OnD3D9DestroyDevice();
    CDXUTDirectionWidget::StaticOnD3D9DestroyDevice();    
    SAFE_RELEASE(m_Font9);
}

//--------------------------------------------------------------------------------------
// Render the help and statistics text
//--------------------------------------------------------------------------------------
void fp_GUI::RenderText() {
    m_TxtHelper->Begin();
    m_TxtHelper->SetInsertionPos( 2, 0 );
    m_TxtHelper->SetForegroundColor( D3DXCOLOR( 1.0f, 1.0f, 0.0f, 1.0f ) );
    m_TxtHelper->DrawTextLine( DXUTGetFrameStats( true/*DXUTIsVsyncEnabled()*/ ) );  
    m_TxtHelper->DrawTextLine( DXUTGetDeviceStats() );    
  
    // Draw help
    if( m_ShowHelp ) {
        UINT nBackBufferHeight = ( DXUTIsAppRenderingWithD3D9() )
                ? DXUTGetD3D9BackBufferSurfaceDesc()->Height
                : DXUTGetDXGIBackBufferSurfaceDesc()->Height;
        m_TxtHelper->SetInsertionPos( 2, nBackBufferHeight-15*6 );
        m_TxtHelper->SetForegroundColor( D3DXCOLOR(1.0f, 0.75f, 0.0f, 1.0f ) );
        m_TxtHelper->DrawTextLine( L"Controls:" );
        m_TxtHelper->DrawFormattedTextLine( L"Time: %f", DXUTGetTime() );
        DXUTGetGlobalTimer()->LimitThreadAffinityToCurrentProc();
/*
        m_TxtHelper->SetInsertionPos( 20, nBackBufferHeight-15*5 );
        m_TxtHelper->DrawTextLine( L"Rotate model: Left mouse button\n"
                                L"Rotate light: Right mouse button\n"
                                L"Rotate camera: Middle mouse button\n"
                                L"Zoom camera: Mouse wheel scroll\n" );

        m_TxtHelper->SetInsertionPos( 250, nBackBufferHeight-15*5 );
        m_TxtHelper->DrawTextLine( L"Hide help: F1\n" 
                                L"Quit: ESC\n" );*/
    } else {
        m_TxtHelper->SetForegroundColor( D3DXCOLOR( 1.0f, 1.0f, 1.0f, 1.0f ) );
        //m_TxtHelper->DrawTextLine( L"Press F1 for help" );
        m_TxtHelper->DrawFormattedTextLine( L"Time: %f", DXUTGetTime() );
    }

    m_TxtHelper->End();
}

typedef std::vector<CDXUTControl*>::iterator fp_GUIControlIterator;

//--------------------------------------------------------------------------------------
// Activate only the controls for the specified rendertype
//--------------------------------------------------------------------------------------
int fp_GUI::ChooseControls() {
    int renderType = 0;
    DXUTComboBoxItem *item = m_SampleUI.GetComboBox(IDC_SELECT_RENDER_TYPE)
            ->GetSelectedItem();    
    if(item) 
        #pragma warning( disable : 4311)
        renderType = (int)item->pData;
        #pragma warning( default : 4311)
    fp_GUIControlIterator it;
    switch(renderType) {
        case FP_GUI_RENDERTYPE_RAYTRACE:
            for(it = m_RaytraceSpecificControls.begin();
                    it != m_RaytraceSpecificControls.end(); it++)
                (*it)->SetVisible(true);
            for(it = m_MCSpecificControls.begin();
                it != m_MCSpecificControls.end(); it++)
                (*it)->SetVisible(false);
            for(it = m_SpriteSpecificControls.begin();
                it != m_SpriteSpecificControls.end(); it++)
                (*it)->SetVisible(false);
            break;
        case FP_GUI_RENDERTYPE_MARCHING_CUBES:
            for(it = m_RaytraceSpecificControls.begin();
                it != m_RaytraceSpecificControls.end(); it++)
                (*it)->SetVisible(false);
            for(it = m_MCSpecificControls.begin();
                it != m_MCSpecificControls.end(); it++)
                (*it)->SetVisible(true);
            for(it = m_SpriteSpecificControls.begin();
                it != m_SpriteSpecificControls.end(); it++)
                (*it)->SetVisible(false);
            break;
        case FP_GUI_RENDERTYPE_SPRITE:
            for(it = m_RaytraceSpecificControls.begin();
                it != m_RaytraceSpecificControls.end(); it++)
                (*it)->SetVisible(false);
            for(it = m_MCSpecificControls.begin();
                it != m_MCSpecificControls.end(); it++)
                (*it)->SetVisible(false);
            for(it = m_SpriteSpecificControls.begin();
                it != m_SpriteSpecificControls.end(); it++)
                (*it)->SetVisible(true);
            break;
    }
    return renderType;
}
