#pragma once
#ifndef FP_UTIL_H
#define FP_UTIL_H

#include "DXUT.h"
#pragma warning(disable:4995)
#include <vector>
#include <string>
#pragma warning(default:4995)

template<class T>
struct fp_Vec3 {
    int x;
    int y;
    int z;

    fp_Vec3<T>() : x(0), y(0), z(0) {}
    fp_Vec3<T>(int X, int Y, int Z) : x(X), y(Y), z(Z) {}
    fp_Vec3<T>(const fp_Vec3& Other) : x(Other.x), y(Other.y), z(Other.z) {}

    fp_Vec3<T> Min() { return x < y ? (x < z ? x : z) : (y < z ? y : z); }
    T Max() { return x > y ? (x > z ? x : z) : (y > z ? y : z); }
};

template<class T> fp_Vec3<T> operator+(const fp_Vec3<T>& A, const fp_Vec3<T>& B) {
    fp_Vec3<T> result;
    result.x = A.x + B.x;
    result.y = A.y + B.y;
    result.z = A.z + B.z;
    return result;
}

typedef fp_Vec3<int> fp_VolumeIndex;

class fp_RenderTarget2D {
public:
	fp_RenderTarget2D(
            ID3D10Device* D3DDevice, 
            unsigned int Width, 
            unsigned int Height, 
            DXGI_FORMAT Format, 
            unsigned int TargetCount = 1, 
            bool CreateDS = false, 
            const D3D10_SUBRESOURCE_DATA *InitialData = NULL);
	virtual ~fp_RenderTarget2D();

	void Clear(float ClearColor[4]);
	ID3D10DepthStencilView* Bind(
            bool SaveRTs = false, 
            bool KeepDS = false, 
            ID3D10DepthStencilView* KnownDS = false);
	void Unbind();
	ID3D10ShaderResourceView* GetSRV(unsigned int i = 0) {
            assert(i < m_TargetCount); return m_SRV[i]; }
	ID3D10Texture2D* GetTex(unsigned int i = 0) {
            assert(i < m_TargetCount); return m_Texture[i]; }

protected:
	unsigned int						m_TargetCount;
	bool								m_TargetsSaved;

	ID3D10Device						*m_Device;
	ID3D10ShaderResourceView			*m_SRV[D3D10_SIMULTANEOUS_RENDER_TARGET_COUNT];
	ID3D10RenderTargetView				*m_RTV[D3D10_SIMULTANEOUS_RENDER_TARGET_COUNT];
	ID3D10Texture2D					    *m_Texture[D3D10_SIMULTANEOUS_RENDER_TARGET_COUNT];
    
	ID3D10Texture2D					    *m_DSTexture;
	ID3D10DepthStencilView				*m_DSV;

	ID3D10RenderTargetView				*m_OldRTVs[D3D10_SIMULTANEOUS_RENDER_TARGET_COUNT];
	ID3D10DepthStencilView				*m_OldDS;
    D3D10_VIEWPORT                      m_RTViewport;
    D3D10_VIEWPORT                      m_OldViewport;

	void SaveRT();
	void RestoreRT();
};

typedef std::vector<std::wstring> fp_StringList;

// Helper functions
class fp_Util {
public:
    // Stuff a FLOAT into a DWORD argument
    static inline DWORD FtoDW( FLOAT f ) { return *((DWORD*)&f); }

    // Gets a random number between min/max boundaries
    static float GetRandomMinMax( float fMin, float fMax );

    // Generates a random vector where X,Y, and Z components are between -1.0 and 1.0
    static D3DXVECTOR3 GetRandomVector();

	static ID3D10Effect* LoadEffect(
			ID3D10Device* D3DDevice, 
			const LPCWSTR Filename, 
			const D3D10_SHADER_MACRO *ShaderMacros = NULL);

    static int ListDirectory(
            fp_StringList* FilesInDirectory,
            const LPCWSTR DirPath,
            const LPCWSTR Extension = NULL,
            bool IncludeHidden = false);
};

#endif
