//--------------------------------------------------------------------------------------
// File: fp_depth_peeler.fx
//--------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------
// Structs
//--------------------------------------------------------------------------------------

struct DepthPeelerVSIn {
    float3 ParticlePosition       : POSITION;
    uint ParticleIndex            : INDEX;    
};

struct DepthPeelerPSIn {
    float4 ParticlePosition       : SV_POSITION;
    uint ParticleIndex            : INDEX;
    float2 ClipDepth              : CLIP_DEPTH;
};

struct DepthPeelerPSOut {
    uint ParticleIndex		      : SV_TARGET;
    float Depth                   : SV_DEPTH;
};

//--------------------------------------------------------------------------------------
// Global variables
//--------------------------------------------------------------------------------------

cbuffer cbPerObject {
    float4x4 g_ViewProjection;
};

//--------------------------------------------------------------------------------------
// Textures
//--------------------------------------------------------------------------------------

Texture2DArray g_LastPeelDepth;

//--------------------------------------------------------------------------------------
// Vertex shader for transformation
// Input:  world space particle position, particle index
// Output: clip space particle position, particle index, clipspace depth (z & w)
//--------------------------------------------------------------------------------------
DepthPeelerPSIn TransformVS(in DepthPeelerVSIn Input) {
    DepthPeelerPSIn output;
    output.ParticlePosition = mul(float4(Input.ParticlePosition, 1), g_ViewProjection);
    output.ParticleIndex = Input.ParticleIndex;
    output.ClipDepth = output.ParticlePosition.zw;
    return output;
}

//--------------------------------------------------------------------------------------
// Pixel shader for depth peeling first pass
// Input:  screen space position, particle index, clipspace depth (z & w)
// Output: particle index, depth
// Passtrough particle index, calculate depth from clipspace depth
//--------------------------------------------------------------------------------------
DepthPeelerPSOut DepthPeelerFirstPS(DepthPeelerPSIn Input)  { 
    DepthPeelerPSOut output;
	output.ParticleIndex = Input.ParticleIndex;
	output.Depth = Input.ClipDepth.x / Input.ClipDepth.y;
    return output;
}

//--------------------------------------------------------------------------------------
// Pixel shader for depth peeling next passes
// Input:  screen space position, particle index, clipspace depth (z & w)
// Output: particle index, depth
// Passtrough particle index, but discard fragments with depth lower or equal then depth
// from last pass
//--------------------------------------------------------------------------------------
DepthPeelerPSOut DepthPeelerNextPS(DepthPeelerPSIn Input)  {
    DepthPeelerPSOut output;
    
    float currentDepth = Input.ClipDepth.x / Input.ClipDepth.y;
    float lastDepth = g_LastPeelDepth.Load(int4(Input.ParticlePosition.xy, 0, 0)).x;
    
    if(currentDepth <= lastDepth) discard;
        
	output.ParticleIndex = Input.ParticleIndex;
	output.Depth = currentDepth;
    return output;
}

//--------------------------------------------------------------------------------------
// States
//--------------------------------------------------------------------------------------

RasterizerState CullNone {
    CullMode = None;
};

BlendState NoBlending {
    AlphaToCoverageEnable = FALSE;
    BlendEnable[0] = FALSE;
};

DepthStencilState EnableDepth {
    DepthEnable = TRUE;
    DepthWriteMask = ALL;
    DepthFunc = LESS_EQUAL;
};

//--------------------------------------------------------------------------------------
// Techniques
//--------------------------------------------------------------------------------------
technique10 DepthPeeling {
    pass P0 {
        SetVertexShader(CompileShader(vs_4_0, TransformVS()));
        SetGeometryShader(NULL);
        SetPixelShader(CompileShader(ps_4_0, DepthPeelerFirstPS()));

        SetBlendState(NoBlending, float4(0.0f, 0.0f, 0.0f, 0.0f), 0xFFFFFFFF);
        SetDepthStencilState(EnableDepth, 0);
        SetRasterizerState(CullNone);
    }
    
    pass P1 {
        SetVertexShader(CompileShader(vs_4_0, TransformVS()));
        SetGeometryShader(NULL);
        SetPixelShader(CompileShader(ps_4_0, DepthPeelerNextPS()));

        SetBlendState(NoBlending, float4(0.0f, 0.0f, 0.0f, 0.0f), 0xFFFFFFFF);
        SetDepthStencilState(EnableDepth, 0);
        SetRasterizerState(CullNone);
    }    
}
