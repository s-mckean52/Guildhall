#include "Headers/Common.hlsl"
#include "Headers/OceanUtils.hlsl"


Texture2D<float4>	tDiffuse			: register(t0);
Texture2D<float4>	tNormal				: register(t1);
Texture2D<float4>	tRefractionStencil	: register(t4);
TextureCube<float4> tSkybox				: register(t5);
Texture2D<float4>	tBackBuffer			: register(t6);
Texture2D<float4>	tDepthStencil		: register(t7);
Texture2D<float4>	tScrollingNormal1	: register(t8);
Texture2D<float4>	tScrollingNormal2	: register(t9);

SamplerState sSampler					: register(s0);


//--------------------------------------------------------------------------------------
// Programmable Shader Stages
//--------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------
// Vertex Shader
v2f_ocean_t VertexFunction(vs_input_ocean_t input)
{
	v2f_ocean_t v2f = (v2f_ocean_t)0;

	float4 local_position = float4(input.position, 1.0f);
	float4 world_position = mul(MODEL, local_position);
	float4 camera_position = mul(VIEW, world_position);
	float4 clip_position = mul(PROJECTION, camera_position);

	float4 local_normal = float4(input.normal, 0.0f);
	float4 local_tangent = float4(input.tangent, 0.0f);
	float4 local_bitangent = float4(input.bitangent, 0.0f);
	float4 world_normal = mul(MODEL, local_normal);
	float4 world_tangent = mul(MODEL, local_tangent);
	float4 world_bitangent = mul(MODEL, local_bitangent);
    
	v2f.position = clip_position;
	v2f.color = input.color * TINT;
	v2f.uv = input.uv;
    
    v2f.camera_position = camera_position.xyz;
	v2f.world_position = world_position.xyz;
	v2f.world_normal = world_normal.xyz;
	v2f.world_tangent = world_tangent.xyz;
	v2f.world_bitangent = world_bitangent.xyz;
	v2f.jacobian = input.jacobian;

	return v2f;
}

//--------------------------------------------------------------------------------------
// Fragment Shader
// 
// SV_Target0 at the end means the float4 being returned
// is being drawn to the first bound color target.
float4 FragmentFunction(v2f_ocean_t input) : SV_Target0
{   
    float3 world_normal = input.world_normal.xyz;
    
    float3 normal1 = float3( 0.f, 0.f, 0.f );
    float3 normal2 = float3( 0.f, 0.f, 0.f );
    if( NORMALS_SCROLL_SPEED.x != 0.f )
    {
	    float2 normal_scroll_uv1 = input.uv + SYSTEM_TIME_SECONDS * NORMALS1_SCROLL_DIR * NORMALS_SCROLL_SPEED.x;
	    float4 normal_color1 = tScrollingNormal1.Sample(sSampler, normal_scroll_uv1);
        normal1 = GetNormalFromColor( normal_color1.xyz );
    }
    if( NORMALS_SCROLL_SPEED.y != 0.f )
    {
	    float2 normal_scroll_uv2 = input.uv + SYSTEM_TIME_SECONDS * NORMALS2_SCROLL_DIR * NORMALS_SCROLL_SPEED.y;
	    float4 normal_color2 = tScrollingNormal2.Sample(sSampler, normal_scroll_uv2);
	    normal2 = GetNormalFromColor( normal_color2.xyz );
    }

    if( NORMALS_SCROLL_SPEED.x != 0.f || NORMALS_SCROLL_SPEED.y != 0.f )
    {
	    float3x3 tbn = GetVertexTBN( input.world_tangent, input.world_bitangent, input.world_normal );

	    world_normal = normalize( mul( normal1, tbn ) );
	    world_normal += normalize( mul( normal2, tbn ) );
	    world_normal = normalize( world_normal );
    }


	float2 backBufferDim;
	tBackBuffer.GetDimensions( backBufferDim.x, backBufferDim.y );

	float3	incident	= normalize(input.world_position - CAMERA_POSITION);
    float cos_theta_incident = abs( dot( incident, world_normal ) );    

    
    //float2 perturbedPixel = input.position.xy;
    //float3 floor_color = tBackBuffer.Load( int3( perturbedPixel, 0 ) );
    float2 perturbedPixel;
    float3 floor_color = GetPerturbedColor( world_normal.xy, 25.f * cos_theta_incident, input.position.xy, tBackBuffer, tRefractionStencil, backBufferDim, perturbedPixel);
	floor_color *= UPWELLING_COLOR;
    
    return float4( floor_color, 1.f );
}
