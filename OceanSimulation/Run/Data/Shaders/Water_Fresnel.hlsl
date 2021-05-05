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

	//Rotation Matricies for converting skybox to game basis
	float3x3 rotation_on_x = float3x3(
		float3(1.f,  0.f, 0.f),
		float3(0.f,  0.f, 1.f),
		float3(0.f, -1.f, 0.f));
	float3x3 rotation_on_z = float3x3(
		float3( 0.f, 1.f, 0.f),
		float3(-1.f, 0.f, 0.f),
		float3( 0.f, 0.f, 1.f));


	//Skybox Sample
	float3	incident	= normalize(input.world_position - CAMERA_POSITION);
	float3	reflection	= reflect(incident, world_normal);
			//reflection	= mul( inverseView, reflection );
			reflection	= mul(rotation_on_x, mul(rotation_on_z, reflection));
	float4	sky_color	= tSkybox.Sample(sSampler, reflection);
	//sky_color = float4( 0.69f, 0.84f, 1.f, 1.f );


	float reflectivity;
    float cos_theta_incident = abs( dot( incident, world_normal ) );
    
    float theta_incident = acos( cos_theta_incident );
	float sin_theta_transmission = sin( theta_incident ) / SNELL;
	float theta_transmission = asin( sin_theta_transmission );
    if( theta_incident == 0.0f )
	{
		reflectivity = ( SNELL - 1 ) / ( SNELL + 1 );
		reflectivity = reflectivity * reflectivity;
	}
	else
	{
		float fs = sin( theta_transmission - theta_incident ) / sin( theta_transmission + theta_incident );
		float ts = tan( theta_transmission - theta_incident ) / tan( theta_transmission + theta_incident );
		reflectivity = 0.5 * ( fs * fs + ts * ts );
	}
	float transmissivity = 1.f - reflectivity;
    
    //float2 perturbedPixel = input.position.xy;
    //float3 floor_color = tBackBuffer.Load( int3( perturbedPixel, 0 ) );
    float2 perturbedPixel;
    float3 floor_color = GetPerturbedColor( world_normal.xy, 25.f * cos_theta_incident, input.position.xy, tBackBuffer, tRefractionStencil, backBufferDim, perturbedPixel);
	floor_color *= UPWELLING_COLOR;

//	float backBufferSample = tDepthStencil.Load( int3( perturbedPixel, 0 ) ).x;
//    float4 backBufferNDCPosition = float4( 0.f, 0.f, backBufferSample, 1.f );
//    float3 backBufferCameraPosition = NDCToCameraSpace( backBufferNDCPosition, INVERSE_PROJECTION );
//    float backBufferDepth = backBufferCameraPosition.x;
//    float pixelDepth = input.camera_position.x;
	//backBufferDepth = -LinearizeDepth( backBufferDepth, NEAR_PLANE, FAR_PLANE );
    
    float waterFalloff = 1.f;
    
//  float refractionDepth = backBufferDepth - pixelDepth;
//	float depthFraction = GetWaterFallOffForDepth( refractionDepth, INVERSE_MAX_DEPTH );
//	floor_color = lerp( floor_color, UPWELLING_COLOR, depthFraction.xxx );
    
	//Water Color
    float3 reflection_color = reflectivity * sky_color.xyz;
    float3 transmission_color = transmissivity * floor_color;
	float3 water_color = reflection_color + transmission_color;
	return float4( water_color, 1.f );
}
