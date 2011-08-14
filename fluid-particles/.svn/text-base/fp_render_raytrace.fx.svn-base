//--------------------------------------------------------------------------------------
// File: fp_render_raytrace.fx
//--------------------------------------------------------------------------------------





//--------------------------------------------------------------------------------------
// Global variables
//--------------------------------------------------------------------------------------

cbuffer Immutable {
    float2 g_CornersTex[4] = { 
        float2(-1, 1), 
        float2( 1, 1),
        float2(-1,-1),
        float2( 1,-1),
    };
    float FLOATMAX = 1000000000;
    float g_DensityScale = 10000;
};

cbuffer Once {
    int3 g_VolumeDimensions; // volume dimension in number of voxels
    float3 g_VolumeSizeRatio; // contains ratio "size / sizeMax" for each dimension
    float3 g_TexDelta;
    float g_StepSize;
    int g_NumRefineSteps = 5;
};

cbuffer Sometimes {
    int g_ParticleVoxelRadius;
    float g_HalfParticleVoxelDiameter;
    float4 g_CornersPos[4]; // normalized device space corner offsets
    float3 g_BBoxSize;
    float g_IsoLevel;
    float g_RefractionRatio;
    float g_RefractionRatioSq;
    float g_RefractionRatio_2;
    float g_RefractionRatioSq_2;
    float g_R0; // (1 - refractionRatio)^2 / (1 + refreactionRatio)^2
    float g_R0_2;
    float g_OneMinusR0;
    float g_OneMinusR0_2;
};

cbuffer Often {
    float4x4 g_WorldToNDS;
    float4x4 g_World;
    float4x4 g_WorldView;
    float4x4 g_WorldViewProjection;
    float4x4 g_InvView;
    float3 g_BBoxStart;
};

//*cbuffer cbEveryFrame {
//*};





//--------------------------------------------------------------------------------------
// Textures
//--------------------------------------------------------------------------------------

Texture1D g_WValsMulParticleMass;
Texture2D g_ExitPoint;
Texture2D g_IntersectionPosition0;
Texture2D g_IntersectionPosition1;
Texture2D g_IntersectionNormal0;
Texture2D g_IntersectionNormal1;
Texture3D g_DensityGrid;
TextureCube g_Environment;





//--------------------------------------------------------------------------------------
// Structs
//--------------------------------------------------------------------------------------

struct SplatParticleVSIn {
    float4 ParticlePosDensity       : POSITION_DENSITY;
    uint ParticleSlice              : SV_InstanceID;    
};

struct SplatParticleGSIn {
    float4 ParticlePosTexZDensity    : POSITION_TEXZ_DENSITY;
    int VolumeSlice                  : VOLUMESLICE;
};

struct SplatParticlePSIn {
    float4 VoxelPos                 : SV_Position;
    float ParticleDensity           : DENSITY;
    uint VolumeSlice                : SV_RenderTargetArrayIndex;
    float3 VoxelTex                 : TEXCOORDS;     
};

struct SplatParticlePSOut {
    half DensityValue               : SV_TARGET;
};

struct RaytraceTransformVSOut {
    float4 VolumePosClipDepth       : VOLUMEPOS_CLIPDEPTH;
    float4 Pos                      : SV_POSITION;
};

struct RaytraceFindAndShadeIsoPSOut {
    float4 Color                    : SV_Target0;
    float Depth                     : SV_Depth;
};





//--------------------------------------------------------------------------------------
// Texture samplers
//--------------------------------------------------------------------------------------

SamplerState LinearBorder {
    Filter = MIN_MAG_MIP_LINEAR;
    AddressU = Border;
    BorderColor = float4(0, 0, 0, 0);
};

SamplerState LinearPointClamp
{
    Filter      = MIN_MAG_LINEAR_MIP_POINT;
    AddressU    = Clamp;
    AddressV    = Clamp;
    AddressW    = Clamp;
};

SamplerState LinearClamp
{
    Filter      = MIN_MAG_MIP_LINEAR;
    AddressU    = Clamp;
    AddressV    = Clamp;
    AddressW    = Clamp;
};

//--------------------------------------------------------------------------------------
// Shaders
//--------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------
// Vertex shader for particle splatting
// Input:  world space particle position, slice index inside particle, ParticleDensity
// Output: normalized device space particle position, slice index inside volume,
//         ParticleDensity
// Transforms particle position to n. d. space, calculates slice index inside volume
//--------------------------------------------------------------------------------------
SplatParticleGSIn SplatParticleVS(in SplatParticleVSIn Input) {
    SplatParticleGSIn result;
    result.ParticlePosTexZDensity = mul(float4(Input.ParticlePosDensity.xyz, 1),
            g_WorldToNDS);
    result.VolumeSlice = int((0.5 * result.ParticlePosTexZDensity.z + 0.5)
            * float(g_VolumeDimensions.z)) - g_ParticleVoxelRadius 
            + int(Input.ParticleSlice);
    result.ParticlePosTexZDensity.z = -(1 - float(Input.ParticleSlice)
            / g_HalfParticleVoxelDiameter);
    result.ParticlePosTexZDensity.w = Input.ParticlePosDensity.w;            
    return result;
}

//--------------------------------------------------------------------------------------
// Geometry shader for particle splatting
// Input:  vertex shader for particle splatting output
// Output: two n. d. space triangles (z=0), texture space corner positions, slice index
//         inside volume (render target index), ParticleDensity
// Computes n. d. space- (x,y) and texture space- (x,y,z) positions of corners.
// Volume slice index becomes render target index.
//--------------------------------------------------------------------------------------
[maxvertexcount (4)]
void SplatParticleGS(
        point SplatParticleGSIn Input[1], 
        inout TriangleStream<SplatParticlePSIn> SpriteStream) {        
	if (Input[0].VolumeSlice < 0 || Input[0].VolumeSlice >= g_VolumeDimensions.z)
	    return;
    SplatParticlePSIn output;
    output.VolumeSlice = Input[0].VolumeSlice;
    output.ParticleDensity = Input[0].ParticlePosTexZDensity.w;
	[unroll] for(int i=0; i<4; i++) {
	    output.VoxelPos = float4(g_CornersPos[i].xy
	            + Input[0].ParticlePosTexZDensity.xy, 0, 1);
		output.VoxelTex = float3(g_CornersTex[i],
		        Input[0].ParticlePosTexZDensity.z);
		SpriteStream.Append(output);
	}
	SpriteStream.RestartStrip();	
}

//--------------------------------------------------------------------------------------
// Pixel shader for particle splatting
// Input:  ParticleDensity, slice index inside volume (z), screen/volume space voxel
//         position (x,y) and texture space (x,y,z) voxel position
// Output: Density value
// Fetches the smoothing function value from a 1D texture and calculates the additive
// density value for this voxel and particle
//--------------------------------------------------------------------------------------
SplatParticlePSOut SplatParticlePS(SplatParticlePSIn Input) {
    SplatParticlePSOut result;
    float l = length(Input.VoxelTex);
    float wValMulParticleMass = g_WValsMulParticleMass.SampleLevel(LinearBorder, l, 0).x;
    result.DensityValue = half(g_DensityScale * wValMulParticleMass);
    return result;
}


//--------------------------------------------------------------------------------------
// Vertex shader for transformations
// Input:  worldspace vertex position
// Output: volume space position, clip space depth, clip space position
// -
//--------------------------------------------------------------------------------------
RaytraceTransformVSOut RaytraceTransformVS(in float3 Position : POSITION) {
    RaytraceTransformVSOut result;
	// scale to [0,1]
    result.VolumePosClipDepth.xyz = Position / g_BBoxSize;
    result.Pos = mul(float4(Position, 1), g_WorldViewProjection);
    result.VolumePosClipDepth.w = result.Pos.z;    
    return result;
}

//--------------------------------------------------------------------------------------
// Pixel shader for finding the rayexit
// Input:  volume space position, clip space depth, screen space position
// Output: volume space position, clip space depth
// -
//--------------------------------------------------------------------------------------
float4 RaytraceExitPS(RaytraceTransformVSOut Input) : SV_Target {
    return Input.VolumePosClipDepth;
}

// Function for finding the intersection of ray with a box (in which it starts)
float LengthInBox(in float3 StartVolumePos, in float3 VolumeDir) {
    float tx = FLOATMAX, ty = FLOATMAX, tz = FLOATMAX;
    if(VolumeDir.x < 0)
        tx = -StartVolumePos.x / VolumeDir.x;
    if (VolumeDir.x > 0)
        tx = (1 - StartVolumePos.x) / VolumeDir.x;
    if(VolumeDir.y < 0)
        ty = -StartVolumePos.y / VolumeDir.y;
    if (VolumeDir.y > 0)
        ty = (1 - StartVolumePos.y) / VolumeDir.y;
    if(VolumeDir.z < 0)
        tz = -StartVolumePos.z / VolumeDir.z;
    if (VolumeDir.z > 0)
        tz = (1 - StartVolumePos.z) / VolumeDir.z;
    return tx < ty ? (tx < tz ? tx : tz) : (ty < tz ? ty : tz);
}

// Function for calculating the local stepsize
float CalcLocalStepsize(float3 RayDir, float RayDirLen) {
	float3 scaledRayDir = RayDir * g_VolumeSizeRatio;
	return g_StepSize * RayDirLen / length(scaledRayDir);
}

// Function for refining the iso trace
float3 RefineIsoSurface(float3 TextureOffset, float3 SampleTexturePos, bool FindEntry) {
	TextureOffset /= 2;
	SampleTexturePos -= TextureOffset;
	for (int i = 0; i < g_NumRefineSteps; i++) {
		TextureOffset /= 2;
		float densityVal = g_DensityGrid.SampleLevel(LinearPointClamp, SampleTexturePos, 0).r;
		if (FindEntry ? densityVal >= g_IsoLevel : densityVal <= g_IsoLevel)
			SampleTexturePos = SampleTexturePos - TextureOffset;
		else
			SampleTexturePos = SampleTexturePos + TextureOffset;
	}	
	return float3(SampleTexturePos);
}

// Function for raytracing the iso surface
void RaytraceIsoSurface(
        out float3 RayDir,
        out float3 IntersectionVolumePos,
        out float3 IntersectionVolumeNormal,
        out float IntersectionClipDepth,
        in bool PerPixelStepSize,
        in bool FindEntry,
        in float4 StartVolumePosClipDepth,
        in float4 EndVolumePosClipDepth) {
    // ray entry, exit and direction
    float3 textureOffset = EndVolumePosClipDepth.xyz - StartVolumePosClipDepth.xyz;
    RayDir = textureOffset;
    float volumeRayLen = length(textureOffset);
    float localStepsize = PerPixelStepSize
            ? CalcLocalStepsize(textureOffset, volumeRayLen)
            : g_StepSize;
            
    float3 sampleTexturePos = StartVolumePosClipDepth.xyz;
    textureOffset /= volumeRayLen;
    textureOffset *= localStepsize;
    sampleTexturePos.y = 1 - sampleTexturePos.y; // y in volume is 1-y in texture
    textureOffset.y *= -1; // offset.y must therefore get flipped
    
    int numSteps = ceil(volumeRayLen / localStepsize);        
    float densityVal;
    while(numSteps-- > 0) {
        densityVal = g_DensityGrid.SampleLevel(LinearClamp, sampleTexturePos, 0).r;
        if (FindEntry ? densityVal >= g_IsoLevel : densityVal <= g_IsoLevel)
			break;
        sampleTexturePos += textureOffset;
    }
    if(numSteps <= 0) {
        float3 exitTexturePos = EndVolumePosClipDepth.xyz;
        exitTexturePos.y = 1 - exitTexturePos.y;
        densityVal = g_DensityGrid.SampleLevel(LinearClamp, exitTexturePos, 0).r;
        if (FindEntry ? densityVal < g_IsoLevel : densityVal > g_IsoLevel)
            discard;
		sampleTexturePos = exitTexturePos;
    }
	// if intersection found
    // refine isosurface
	sampleTexturePos = RefineIsoSurface(textureOffset, sampleTexturePos, FindEntry);
	
	IntersectionVolumePos = sampleTexturePos;
	IntersectionVolumePos.y = 1 - IntersectionVolumePos.y;	
	
	// depth is calculated inside this function for memory latency hiding reasons
	// compute depth 		
	IntersectionClipDepth = StartVolumePosClipDepth.w
	        + length(StartVolumePosClipDepth.xyz - IntersectionVolumePos)
	        / volumeRayLen * (EndVolumePosClipDepth.w - StartVolumePosClipDepth.w);  
	        	
	// generate normal
	float3 grad;
	grad.x = g_DensityGrid.SampleLevel(LinearClamp,
	        sampleTexturePos + float3(g_TexDelta.x, 0, 0), 0).x
	        -
            g_DensityGrid.SampleLevel(LinearClamp,
            sampleTexturePos - float3(g_TexDelta.x, 0, 0), 0).x;
	grad.y = g_DensityGrid.SampleLevel(LinearClamp,
	        sampleTexturePos - float3(0, g_TexDelta.y, 0), 0).x
	        -
            g_DensityGrid.SampleLevel(LinearClamp,
            sampleTexturePos + float3(0, g_TexDelta.y, 0), 0).x;
	grad.z = g_DensityGrid.SampleLevel(LinearClamp,
	        sampleTexturePos + float3(0, 0, g_TexDelta.z), 0).x
	        -
            g_DensityGrid.SampleLevel(LinearClamp,
            sampleTexturePos - float3(0, 0 ,g_TexDelta.z), 0).x;        
	IntersectionVolumeNormal = -normalize(grad);    
}

//--------------------------------------------------------------------------------------
// Pixel shader for raytracing and shading the iso surface
// Input:  volume space position, clip space depth, screen space position
// Output: color and depth
// Raytraces the density grid along the view ray to find the first intersection with the
// isosurface. Calculates reflection- and refraction rays. Find's intersection of
// refraction ray with the isosurface. Looks up environment-map for reflected and
// (two-times) refracted rays. Uses Fresnel terms to compose the final color.
//--------------------------------------------------------------------------------------
RaytraceFindAndShadeIsoPSOut RaytraceFindAndShadeIsoPS(
        RaytraceTransformVSOut Input,
        uniform bool PerPixelStepSize) {
    RaytraceFindAndShadeIsoPSOut result;
    
    float4 entryVolumePosClipDepth = Input.VolumePosClipDepth;
    float4 exitVolumePosClipDepth = g_ExitPoint.Load(
            int3(Input.Pos.xy, 0));            
            
    // Raytrace the iso surface            
	float3 rayDir, intersection1VolumePos, intersection1VolumeNormal;
    RaytraceIsoSurface(rayDir, intersection1VolumePos, intersection1VolumeNormal,
            result.Depth, PerPixelStepSize, true, entryVolumePosClipDepth,
            exitVolumePosClipDepth);

    // Calculate fresnel term for first intersection
    float fresnel1 = g_R0 + g_OneMinusR0
            * pow(1 - dot(-rayDir, intersection1VolumeNormal), 5);

	// Calculate external reflection
	float3 reflect1Dir = reflect(rayDir, intersection1VolumeNormal);			    
    
    // Calculate first refraction using snells law
    // sinThetaR = (ni/nr) * sinThetaI
    // => R = ((ni/nr)*(N*V) - sqrt(1 - (ni/nr)^2*(1.0f-(N*V)^2))) * N - (ni/nr) * V
    
    //*float NV = dot(intersection1VolumeNormal, -rayDir);    
    //*float cosThetaR = sqrt(1 - g_RefractionRatioSq * (1 - NV * NV));
    //*float beforeNTerm = g_RefractionRatio * NV - cosThetaR;
    //*float3 refract1Dir = normalize(beforeNTerm * intersection1VolumeNormal
            //*+ g_RefractionRatio * rayDir);
            
    // The intrinsic refract function does exact the same            
    float3 refract1Dir = refract(rayDir, intersection1VolumeNormal, g_RefractionRatio);
            
    float refract1Len = LengthInBox(intersection1VolumePos, refract1Dir);
    float4 refract1Start = float4(intersection1VolumePos + 0.01 * refract1Dir, 1);
    float4 refract1End = float4(intersection1VolumePos + refract1Len * refract1Dir, 1);

    // Find the exit-intersection of the refraction-ray with the iso surface
    float3 intersection2VolumePos, intersection2VolumeNormal, dummy;
    RaytraceIsoSurface(dummy, intersection2VolumePos, intersection2VolumeNormal, dummy.x,
            PerPixelStepSize, false, refract1Start, refract1End);
            
    // Calculate second (internal) reflection
    float3 reflect2Dir = reflect(refract1Dir, -intersection2VolumeNormal);
    
    // Calculate second refraction
    float3 refract2Dir = refract(refract1Dir, -intersection2VolumeNormal,
            g_RefractionRatio_2);
            
    // Sample external and internal reflections
    // Sharp external reflections look best => use high detail mip
    float3 reflectColor = g_Environment.SampleLevel(LinearClamp, reflect1Dir, 0);
    // Smooth internal reflections look better => use lower detail mip
    // Perhaps this is a good replace for subsequent internal reflections
    float3 reflect2Color = g_Environment.SampleLevel(LinearClamp, reflect2Dir, 3);
    
    // Note: This is quite a hack because the internal reflection ray would hit the
    // surface again.
    // But it looks good anyhow.        
    
    float3 refractColor;
    if(any(refract2Dir)) { // Refraction exists
        // Calculate fresnel term for second intersection
        // The approximation doesn't work here
        float c = dot(intersection2VolumeNormal, refract1Dir) * g_RefractionRatio_2;
        float g = sqrt(1 + c * c - g_RefractionRatioSq_2);
        float gmc = g - c;
        float gpc = g + c;
        float gmc_gpcQuotient = gmc / gpc;
        float tmpTerm = (c * gpc - g_RefractionRatioSq_2) / (c * gmc + g_RefractionRatioSq_2);
        float fresnel2 = 0.5 * gmc_gpcQuotient * gmc_gpcQuotient * (1 + tmpTerm * tmpTerm);
        
        // Refraction of a round body magnifies => use high detail mip
        float3 refract2Color = g_Environment.SampleLevel(LinearClamp, refract2Dir, 0);
        
        // Calculate color at second intersection according to the Fresnel equations
        refractColor = fresnel2 * reflect2Color + (1 - fresnel2) * refract2Color;
    } else // Total internal reflection
        refractColor = reflect2Color;    
    
    // Calculate color at first intersection according to the Fresnel equations
    result.Color = float4(fresnel1 * reflectColor + (1 - fresnel1) * refractColor, 1);
  
    return result;
}





//--------------------------------------------------------------------------------------
// Vertex shader for environment rendering
//--------------------------------------------------------------------------------------
void EnvironmentVS(
        in float3 PositionIn : POSITION,
        out float4 PositionOut : SV_POSITION,
        out float3 TexCoordsOut : TEXCOORD0) {
    PositionOut = mul(float4(PositionIn, 1), g_WorldViewProjection);
    TexCoordsOut = mul(float4(PositionIn, 1), g_World).xyz;
}

//--------------------------------------------------------------------------------------
// Pixel shader for environment rendering
//--------------------------------------------------------------------------------------
void EnvironmentPS(in float4 Position : SV_POSITION,
        in float3 TexCoords : TEXCOORD0,
        out float4 Color : SV_TARGET) {
    Color = g_Environment.Sample(LinearClamp, TexCoords);
}





//--------------------------------------------------------------------------------------
// States
//--------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------
// Rasterizer states
//--------------------------------------------------------------------------------------

RasterizerState CullFront {
    CullMode = Front;
};

RasterizerState CullBack {
    CullMode = Back;
};

RasterizerState CullNone {
    CullMode = None;
};

//--------------------------------------------------------------------------------------
// Blend states
//--------------------------------------------------------------------------------------

BlendState AlphaBlending {
    AlphaToCoverageEnable = FALSE;
    SrcBlend = SRC_ALPHA;
    DestBlend = INV_SRC_ALPHA;
    BlendOp = ADD;
    SrcBlendAlpha = ZERO;
    DestBlendAlpha = ZERO;
    BlendOpAlpha = ADD;
    
    BlendEnable[0] = TRUE;
    BlendEnable[1] = TRUE;
    BlendEnable[2] = TRUE;
    BlendEnable[3] = TRUE;
    BlendEnable[4] = TRUE;
    BlendEnable[5] = TRUE;
    BlendEnable[6] = TRUE;
    BlendEnable[7] = TRUE;
    
    RenderTargetWriteMask[0] = 0x0F;
    RenderTargetWriteMask[1] = 0x0F;
    RenderTargetWriteMask[2] = 0x0F;
    RenderTargetWriteMask[3] = 0x0F;
    RenderTargetWriteMask[4] = 0x0F;
    RenderTargetWriteMask[5] = 0x0F;
    RenderTargetWriteMask[6] = 0x0F;
    RenderTargetWriteMask[7] = 0x0F;   
};

BlendState AdditiveBlending
{
    AlphaToCoverageEnable = FALSE;    
    SrcBlend = ONE;
    DestBlend = ONE;
    BlendOp = ADD;
    SrcBlendAlpha = ONE;
    DestBlendAlpha = ONE;
    BlendOpAlpha = ADD;
    
    BlendEnable[0] = TRUE;
    BlendEnable[1] = TRUE;
    BlendEnable[2] = TRUE;
    BlendEnable[3] = TRUE;
    BlendEnable[4] = TRUE;
    BlendEnable[5] = TRUE;
    BlendEnable[6] = TRUE;
    BlendEnable[7] = TRUE;
    
    RenderTargetWriteMask[0] = 0x0F;
    RenderTargetWriteMask[1] = 0x0F;
    RenderTargetWriteMask[2] = 0x0F;
    RenderTargetWriteMask[3] = 0x0F;
    RenderTargetWriteMask[4] = 0x0F;
    RenderTargetWriteMask[5] = 0x0F;
    RenderTargetWriteMask[6] = 0x0F;
    RenderTargetWriteMask[7] = 0x0F;    
}; 

BlendState BlendOver {
    AlphaToCoverageEnable = FALSE;
    SrcBlend = SRC_ALPHA;
    DestBlend = INV_SRC_ALPHA;
    BlendOp = ADD;
    SrcBlendAlpha = ONE;
    DestBlendAlpha = ONE;
    BlendOpAlpha = ADD;

    BlendEnable[0]			 = TRUE;
    BlendEnable[1]			 = TRUE;
    BlendEnable[2]			 = TRUE;
    BlendEnable[3]			 = TRUE;
    BlendEnable[4]			 = TRUE;
    BlendEnable[5]			 = TRUE;
    BlendEnable[6]			 = TRUE;
    BlendEnable[7]			 = TRUE;

    RenderTargetWriteMask[0] = 0x0F;
	RenderTargetWriteMask[1] = 0x0F;
    RenderTargetWriteMask[2] = 0x0F;
    RenderTargetWriteMask[3] = 0x0F;
    RenderTargetWriteMask[4] = 0x0F;
    RenderTargetWriteMask[5] = 0x0F;
    RenderTargetWriteMask[6] = 0x0F;
    RenderTargetWriteMask[7] = 0x0F;
};

BlendState NoBlending {
    AlphaToCoverageEnable = FALSE;
    BlendEnable[0] = FALSE;
    BlendEnable[1] = FALSE;
    BlendEnable[2] = FALSE;
    BlendEnable[3] = FALSE;
    BlendEnable[4] = FALSE;
    BlendEnable[5] = FALSE;
    BlendEnable[6] = FALSE;
    BlendEnable[7] = FALSE;
};

//--------------------------------------------------------------------------------------
// DepthStencil states
//--------------------------------------------------------------------------------------

DepthStencilState EnableDepth {
    DepthEnable = TRUE;
    DepthWriteMask = ALL;
    DepthFunc = LESS_EQUAL;
};

DepthStencilState DisableDepth {
    DepthEnable = false;
    DepthWriteMask = ZERO;
    DepthFunc = Less;

    //Stencil
    StencilEnable = false;
    StencilReadMask = 0xFF;
    StencilWriteMask = 0x00;
};





//--------------------------------------------------------------------------------------
// Techniques
//--------------------------------------------------------------------------------------

technique10 RenderRaytrace {
    pass P0_SplatParticle {
        SetVertexShader(CompileShader(vs_4_0, SplatParticleVS()));
        SetGeometryShader(CompileShader(gs_4_0, SplatParticleGS()));
        SetPixelShader(CompileShader(ps_4_0, SplatParticlePS()));

        SetBlendState(AdditiveBlending, float4(0.0f, 0.0f, 0.0f, 0.0f), 0xFFFFFFFF);
        SetDepthStencilState(DisableDepth, 0);
        SetRasterizerState(CullNone);
    }
    
    pass P1_FindRayExit {
        SetVertexShader(CompileShader(vs_4_0, RaytraceTransformVS()));
        SetGeometryShader(NULL);
        SetPixelShader(CompileShader(ps_4_0, RaytraceExitPS()));  
          
        SetBlendState(NoBlending, float4(0.0f, 0.0f, 0.0f, 0.0f), 0xFFFFFFFF);
        SetDepthStencilState(DisableDepth, 0);
        SetRasterizerState(CullFront);
    }

    pass P2_TraceIsoSurfaceAndShade {
        SetVertexShader(CompileShader( vs_4_0, RaytraceTransformVS()));
        SetGeometryShader(NULL);
        SetPixelShader(CompileShader(ps_4_0, RaytraceFindAndShadeIsoPS(false)));
        
        SetBlendState(NoBlending, float4(0.0f, 0.0f, 0.0f, 0.0f), 0xFFFFFFFF);
        SetDepthStencilState(DisableDepth, 0);
        SetRasterizerState(CullBack);        
    }
    
    pass P3_TraceIsoSurfaceAndShadeWidthPerPixelStepsize {
        SetVertexShader(CompileShader( vs_4_0, RaytraceTransformVS()));
        SetGeometryShader(NULL);
        SetPixelShader(CompileShader(ps_4_0, RaytraceFindAndShadeIsoPS(true)));
        
        SetBlendState(NoBlending, float4(0.0f, 0.0f, 0.0f, 0.0f), 0xFFFFFFFF);
        SetDepthStencilState(DisableDepth, 0);
        SetRasterizerState(CullBack);        
    }
    
    pass P4_RenderEnvironment {
        SetVertexShader(CompileShader( vs_4_0, EnvironmentVS()));
        SetGeometryShader(NULL);
        SetPixelShader(CompileShader(ps_4_0, EnvironmentPS()));
        
        SetBlendState(NoBlending, float4(0.0f, 0.0f, 0.0f, 0.0f), 0xFFFFFFFF);
        SetDepthStencilState(DisableDepth, 0);
        SetRasterizerState(CullNone);        
    }    
}
