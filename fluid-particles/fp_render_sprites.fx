//--------------------------------------------------------------------------------------
// File: fp_render_sprites.fx
//--------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------
// Structs
//--------------------------------------------------------------------------------------

struct RenderSpritesGSIn {
    float3 Pos            : POSITION;
};

struct RenderSpritesPSIn {
    float4 Pos  	      : SV_POSITION;
    float2 Tex			  : TEXCOORD0;
};

struct RenderSpritesPSOut {
    float4 Color		  : SV_TARGET;
};

//--------------------------------------------------------------------------------------
// Global variables
//--------------------------------------------------------------------------------------

cbuffer cbPerObject {
    float4x4 g_ViewProj;
};

cbuffer cbUser {
    float3 g_SpriteCornersWorldS[4];
};

cbuffer cbImmutable {
    float2 g_SpriteTexCoords[4] = { 
        float2(0,0), 
        float2(1,0),
        float2(0,1),
        float2(1,1),
    };
};

//--------------------------------------------------------------------------------------
// Textures
//--------------------------------------------------------------------------------------

Texture2D g_ParticleDiffuse;

//--------------------------------------------------------------------------------------
// Texture samplers
//--------------------------------------------------------------------------------------

SamplerState g_LinearClamp {
    Filter = MIN_MAG_MIP_LINEAR;
    AddressU = Clamp;
    AddressV = Clamp;
};

//--------------------------------------------------------------------------------------
// BlendStates
//--------------------------------------------------------------------------------------

BlendState AlphaBlending {
    AlphaToCoverageEnable = FALSE;
    BlendEnable[0] = TRUE;
    SrcBlend = SRC_ALPHA;
    DestBlend = ONE;
    BlendOp = ADD;
    SrcBlendAlpha = ZERO;
    DestBlendAlpha = ZERO;
    BlendOpAlpha = ADD;
    RenderTargetWriteMask[0] = 0x0F;
};

BlendState NoBlending {
    AlphaToCoverageEnable = FALSE;
    BlendEnable[0] = FALSE;
};

//--------------------------------------------------------------------------------------
// DepthStencilStates
//--------------------------------------------------------------------------------------

DepthStencilState EnableDepth {
    DepthEnable = TRUE;
    DepthWriteMask = ALL;
};

DepthStencilState DisableDepth {
    DepthEnable = FALSE;
    DepthWriteMask = ZERO;
};

DepthStencilState DisableDepthWrite {
    DepthEnable = TRUE;
    DepthWriteMask = ZERO;
};

DepthStencilState DisableDepthTest {
    DepthEnable = TRUE;
    DepthWriteMask = ALL;
    DepthFunc = ALWAYS;
};

//--------------------------------------------------------------------------------------
// Geometry shader for render sprites
// Input:  world space particle position
// Output: 2 clip space triangles, texture coordinates
//--------------------------------------------------------------------------------------
[maxvertexcount(4)]
void RenderSpritesGS(
		point RenderSpritesGSIn Input[1], 
		inout TriangleStream<RenderSpritesPSIn> SpriteStream) {				
	RenderSpritesPSIn output;
	[unroll] for(int i=0; i<4; i++) {					
		float3 spriteCornerWorldS = Input[0].Pos + g_SpriteCornersWorldS[i];	
		output.Pos = mul(float4(spriteCornerWorldS,1), g_ViewProj);		
		output.Tex = g_SpriteTexCoords[i];
		SpriteStream.Append(output);
	}	
	SpriteStream.RestartStrip();
}	

//--------------------------------------------------------------------------------------
// Pixel shader for render sprites
// Input:  screen space position (not used), texture coordinates (both p. c. interpolated)
// Output: diffuse color
// Lookups the diffuse color in particle texture
//--------------------------------------------------------------------------------------
RenderSpritesPSOut RenderSpritesPS(RenderSpritesPSIn Input)  { 
    RenderSpritesPSOut output;
	output.Color = g_ParticleDiffuse.Sample(g_LinearClamp, Input.Tex);
    return output;
}


//--------------------------------------------------------------------------------------
// Techniques
//--------------------------------------------------------------------------------------
technique10 RenderSprites {
    pass P0 {
        SetGeometryShader(CompileShader(gs_4_0, RenderSpritesGS()));
        SetPixelShader(CompileShader(ps_4_0, RenderSpritesPS()));

        SetBlendState(AlphaBlending, float4( 0.0f, 0.0f, 0.0f, 0.0f ), 0xFFFFFFFF );
        SetDepthStencilState( DisableDepthWrite, 0 );
    }
}
