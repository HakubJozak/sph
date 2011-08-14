//--------------------------------------------------------------------------------------
// File: fp_render_marching_cubes.fx
//--------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------
// Structs
//--------------------------------------------------------------------------------------

struct RenderMarchingCubesVSIn {
    float3 Pos  	      : POSITION;
    float3 Normal         : NORMAL;    
};

struct RenderMarchingCubesPSIn {
    float4 Pos  	      : SV_POSITION;
    float4 Diffuse        : COLOR0; // vertex diffuse color
};

struct RenderMarchingCubesPSOut {
    float4 Color		  : SV_TARGET;
};

//--------------------------------------------------------------------------------------
// Global variables
//--------------------------------------------------------------------------------------

cbuffer cbOnce {
    float4 g_MaterialAmbientColor;      // Material's ambient color
    float4 g_MaterialDiffuseColor;      // Material's diffuse color        
};

cbuffer cbSometimes {
    float4 g_LightDir[3];               // Light's direction in world space    
    float4 g_LightDiffuse[3];           // Light's diffuse color
    float4 g_LightAmbient;              // Light's ambient color
};

cbuffer cbEveryFrame {
    float4x4 g_ViewProjection;    // View * Projection matrix
};

//--------------------------------------------------------------------------------------
// BlendStates
//--------------------------------------------------------------------------------------

BlendState AlphaBlending {
    AlphaToCoverageEnable = FALSE;
    BlendEnable[0] = TRUE;
    SrcBlend = SRC_ALPHA;
    DestBlend = INV_SRC_ALPHA;
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
    DepthFunc = LESS_EQUAL;
};

//--------------------------------------------------------------------------------------
// Vertex shader for marching cubes
// Input:  worldspace position, normal
// Output: clip space position, vertex color
// Transforms to clipspace, calculates n dot l lighting
//--------------------------------------------------------------------------------------
RenderMarchingCubesPSIn RenderMarchingCubesVS(
        in RenderMarchingCubesVSIn Input,
        uniform int NumLights) {
	RenderMarchingCubesPSIn output;
	
	output.Pos = mul(float4(Input.Pos,1), g_ViewProjection);
	
	// Compute simple directional lighting equation
	float3 totalLightDiffuse = float3(0,0,0);
	for(int i=0; i<NumLights; i++ )
        totalLightDiffuse += g_LightDiffuse[i].rgb * max(0,dot(Input.Normal,
                g_LightDir[i].xyz));
                
    output.Diffuse.rgb = g_MaterialDiffuseColor * totalLightDiffuse + 
                         g_MaterialAmbientColor * g_LightAmbient;   
    output.Diffuse.a = 1.0f; 
	
	return output;
}	

//--------------------------------------------------------------------------------------
// Pixel shader for marching cubes
// Input:  screen space position (not used), pixel color (both p. c. interpolated)
// Output: pixel color
// Does nothing
//--------------------------------------------------------------------------------------
RenderMarchingCubesPSOut RenderMarchingCubesPS(RenderMarchingCubesPSIn Input)  { 
    RenderMarchingCubesPSOut output;
	output.Color = Input.Diffuse;
    return output;
}


//--------------------------------------------------------------------------------------
// Techniques
//--------------------------------------------------------------------------------------
technique10 RenderMarchingCubes1Light {
    pass P0 {
        SetVertexShader(CompileShader(vs_4_0, RenderMarchingCubesVS(1)));
        SetGeometryShader( NULL );
        SetPixelShader(CompileShader(ps_4_0, RenderMarchingCubesPS()));
                
        SetDepthStencilState( EnableDepth, 0 );
    }
}

technique10 RenderMarchingCubes2Lights {
    pass P0 {
        SetVertexShader(CompileShader(vs_4_0, RenderMarchingCubesVS(2)));
        SetGeometryShader( NULL );
        SetPixelShader(CompileShader(ps_4_0, RenderMarchingCubesPS()));
                
        SetDepthStencilState( EnableDepth, 0 );
    }
}

technique10 RenderMarchingCubes3Lights {
    pass P0 {
        SetVertexShader(CompileShader(vs_4_0, RenderMarchingCubesVS(3)));
        SetGeometryShader( NULL );
        SetPixelShader(CompileShader(ps_4_0, RenderMarchingCubesPS()));
                
        SetDepthStencilState( EnableDepth, 0 );
    }
}
