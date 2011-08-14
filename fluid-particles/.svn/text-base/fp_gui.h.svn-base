#pragma once
#ifndef FP_GUI_H
#define FP_GUI_H

#include "DXUT.h"
#include "DXUTcamera.h"
#include "DXUTgui.h"
#include "DXUTsettingsDlg.h"
#include "SDKmisc.h"
#pragma warning(disable:4995)
#include <vector>
#pragma warning(default:4995)

#include "fp_global.h"
#include "fp_util.h"

//--------------------------------------------------------------------------------------
// UI control IDs
//--------------------------------------------------------------------------------------

#define IDC_TOGGLEFULLSCREEN                    1
#define IDC_TOGGLEREF                           3
#define IDC_CHANGEDEVICE                        4
#define IDC_NUM_LIGHTS                          6
#define IDC_NUM_LIGHTS_STATIC                   7
#define IDC_ACTIVE_LIGHT                        8
#define IDC_LIGHT_SCALE                         9
#define IDC_LIGHT_SCALE_STATIC                 10
#define IDC_SPRITE_PARTICLE_SCALE              11
#define IDC_SPRITE_PARTICLE_SCALE_STATIC       12
#define IDC_RESET_SIM                          13
#define IDC_SELECT_RENDER_TYPE                 14
#define IDC_MC_ISO_LEVEL                       15
#define IDC_MC_ISO_LEVEL_STATIC                16
#define IDC_MC_VOXEL_SIZE                      17
#define IDC_MC_VOXEL_SIZE_STATIC               18
#define IDC_MOVE_HORIZONTALLY                  19
#define IDC_RAYTRACE_ISO_LEVEL                  20
#define IDC_RAYTRACE_ISO_LEVEL_STATIC           21
#define IDC_RAYTRACE_STEP_SCALE                 22
#define IDC_RAYTRACE_STEP_SCALE_STATIC          23
#define IDC_RAYTRACE_REFRACTION_RATIO           24
#define IDC_RAYTRACE_REFRACTION_RATIO_STATIC    25
#define IDC_RAYTRACE_SELECT_CUBEMAP             26
#define IDC_STOP_SIM                           27
#define IDC_TIME_FACTOR                        28
#define IDC_TIME_FACTOR_STATIC                 29

#define FP_GUI_HEIGHT              550

//--------------------------------------------------------------------------------------
// Fluid particles GUI
//--------------------------------------------------------------------------------------

enum fp_GUIRenderType {
    FP_GUI_RENDERTYPE_RAYTRACE = 0,
    FP_GUI_RENDERTYPE_MARCHING_CUBES,
    FP_GUI_RENDERTYPE_SPRITE
};

class fp_GUI {
public:
    bool m_ShowHelp; // If true, it renders the UI control text
    D3DXVECTOR3 m_LightDir[FP_MAX_LIGHTS];
    D3DXVECTOR4 m_LightDiffuse[FP_MAX_LIGHTS];
    D3DXCOLOR m_LightDiffuseColor[FP_MAX_LIGHTS];

    void SetCubeMapNames(fp_StringList* CubeMapNames, int CurrentCubeMap);

    fp_GUI();

    bool    RenderSettingsDialog(float ElapsedTime);     
    bool MsgProc(   // Returns true if no further processing is required
            HWND Wnd,
            UINT Msg,
            WPARAM wParam,
            LPARAM lParam,
            bool* NoFurtherProcessing,
            int ActiveLight);
    void    OnKeyboard( UINT Char, bool KeyDown, bool AltDown, void* UserContext );
    void    OnGUIEvent(
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
            float& ParticeScale,
            bool& ResetSim,
            bool& StopSim,
            bool& MoveHorizontally,
            int& RenderType,
            int& SelectedCubeMap,
            float& TimeFactor);

    // DX9 specific
    HRESULT OnD3D9CreateDevice(
            IDirect3DDevice9* D3DDevice,
            const D3DSURFACE_DESC* BackBufferSurfaceDesc,
            void* UserContext );
    HRESULT OnD3D9ResetDevice(
            IDirect3DDevice9* D3DDevice,
            const D3DSURFACE_DESC* BackBufferSurfaceDesc,
            void* UserContext );
    void    OnD3D9FrameRender(
            IDirect3DDevice9* D3DDevice,
            float ElapsedTime,
            const D3DXVECTOR3* EyePt,
            const D3DXMATRIX*  View,
            const D3DXMATRIX*  Proj,
            int NumActiveLights,
            int ActiveLight,
            float LightScale);
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
            float ElapsedTime,
            const D3DXVECTOR3* EyePt,
            const D3DXMATRIX*  View,
            const D3DXMATRIX*  Proj,
            int NumActiveLights,
            int ActiveLight,
            float LightScale);    

private:
    CDXUTDialogResourceManager m_DialogResourceManager; // manager for shared resources of dialogs
    CDXUTDirectionWidget    m_LightControl[FP_MAX_LIGHTS];
    CD3DSettingsDlg         m_D3DSettingsDlg;       // Device settings dialog
    CDXUTDialog             m_HUD;                  // manages the 3D   
    CDXUTDialog             m_SampleUI;             // dialog for app specific controls    

    // Direct3D9 resources
    ID3DXFont*       m_Font9;         // Font for drawing text
    ID3DXSprite*     m_Sprite9;       // Sprite for batching draw text calls
    CDXUTTextHelper*        m_TxtHelper;

    // Direct3D10 resources
    ID3DX10Font*            m_Font10;       
    ID3DX10Sprite*          m_Sprite10;

    std::vector<CDXUTControl*> m_RaytraceSpecificControls;
    std::vector<CDXUTControl*> m_MCSpecificControls;
    std::vector<CDXUTControl*> m_SpriteSpecificControls;

    void    RenderText();
    int ChooseControls();
};

#endif
