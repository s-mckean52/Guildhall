#include "Headers/Common.hlsl"
#include "Headers/OceanUtils.hlsl"

struct vs_input_fullscreen_t
{
    uint vidx : SV_VERTEXID;
};

Texture2D <float4> tBackbuffer      : register(t0);
Texture2D <float4> tDepthStencil    : register(t8);
SamplerState sSampler : register(s0);


struct VertexToFragment_t
{
	float4 position : SV_POSITION;
	float2 uv : UV;
};

//--------------------------------------------------------------------------------------
// constants
//--------------------------------------------------------------------------------------
static float3 POSITIONS[3] = {
   float3(-1.0f, -1.0f, 0.0f),
   float3(3.0f, -1.0f, 0.0f),
   float3(-1.0f,  3.0f, 0.0f)
};

static float2 UVS[3] = {
   float2(0.0f,  1.0f),
   float2(2.0f,  1.0f),
   float2(0.0f,  -1.0f)
};

//--------------------------------------------------------------------------------------
// Vertex Shader
//--------------------------------------------------------------------------------------
VertexToFragment_t VertexFunction(vs_input_fullscreen_t input)
{
	VertexToFragment_t v2f = (VertexToFragment_t)0;

	v2f.position = float4(POSITIONS[input.vidx], 1.0f);
	v2f.uv = UVS[input.vidx];

	return v2f;
}


//--------------------------------------------------------------------------------------
// Pixel Shader
//--------------------------------------------------------------------------------------
float4 FragmentFunction(VertexToFragment_t input) : SV_Target0
{
    float3 floor_color = tBackbuffer.Sample(sSampler, input.uv).xyz;
    float3 depth_color = tDepthStencil.Sample(sSampler, input.uv).xyz;
    
    //return float4( floor_color, 1.f );
    float4 backBufferNDCPosition = float4( 0.f, 0.f, depth_color.x, 1.f );
    float3 backBufferCameraPosition = NDCToCameraSpace( backBufferNDCPosition, WORLD_INVERSE_PROJECTION );
    float backBufferDepth = backBufferCameraPosition.x;
    
    float depthFade = GetWaterFallOffForDepth( backBufferDepth, INVERSE_MAX_DEPTH );
    
    float3 final_color = lerp( floor_color, UPWELLING_COLOR, depthFade.xxx );
    
    return float4( final_color, 1.f );
}