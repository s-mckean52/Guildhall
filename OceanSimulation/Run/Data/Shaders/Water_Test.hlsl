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
//	float cos_theta_incident =  dot( incident, world_normal );
//    if( cos_theta_incident > 0.f )
//    {
//        float waterToAirSnell = 1.f / SNELL;
//        float sint = waterToAirSnell * sqrt( max( 0.f, 1.f - cos_theta_incident * cos_theta_incident ) );
//        if( sint > 1.f )
//        {
//            return float4( UPWELLING_COLOR, 1.f );
//        }
//        else
//        {
//            //Depth Sample ( real transmission/refractance )
//            float c2 = sqrt( 1.f - ( waterToAirSnell * waterToAirSnell * ( 1.f - cos_theta_incident ) * ( 1.f - cos_theta_incident ) ) );
//            float3 transmission_dir = ( waterToAirSnell * ( incident + -world_normal * cos_theta_incident ) ) - ( c2 * -world_normal );
//            //transmission_dir = float3( 1.f, 0.f, 0.f );
//            float3 transmission_dir_skybox = mul( rotation_on_x, mul(rotation_on_z, normalize( transmission_dir ) ) );
//            float4 transmission_color = tSkybox.Sample( sSampler, transmission_dir_skybox );
//            return transmission_color;
//        }
//    }
    
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
	//floor_color *= upwelling;

	float backBufferSample = tDepthStencil.Load( int3( perturbedPixel, 0 ) ).x;
    float4 backBufferNDCPosition = float4( 0.f, 0.f, backBufferSample, 1.f );
    float3 backBufferCameraPosition = NDCToCameraSpace( backBufferNDCPosition, INVERSE_PROJECTION );
    float backBufferDepth = backBufferCameraPosition.x;
    float pixelDepth = input.camera_position.x;
	//backBufferDepth = -LinearizeDepth( backBufferDepth, NEAR_PLANE, FAR_PLANE );
    
    float waterFalloff = 1.f;
    
    float refractionDepth = backBufferDepth - pixelDepth;
	float depthFraction = GetWaterFallOffForDepth( refractionDepth, INVERSE_MAX_DEPTH );
	floor_color = lerp( floor_color, UPWELLING_COLOR, depthFraction.xxx );
    if( refractionDepth <= FOAM_THICKNESS && refractionDepth >= 0.f )
    {
        if( FOAMINESS == 0.f )
        {
            return float4( 1.f, 1.f, 1.f, 1.f );
        }
        float4 noise = tDiffuse.Sample( sSampler, ( input.uv * 1.5f ) + SYSTEM_TIME_SECONDS * normalize( float2( 1.f, 1.f ) ) * float2( 0.001f, 0.003f ) );
        float noiseA = noise.w;
        float noiseG = noise.y;;
        float noiseValue = ( noiseA * FOAMINESS ) + ( noiseG * ( 1.f - FOAMINESS ) );
        noiseValue = clamp( noiseValue, 0.1f, 1.f );
        noiseValue = 1.f - ( noiseValue * noiseValue );
        
        float refractionFactor = RangeMap( refractionDepth, 0.f, FOAM_THICKNESS, 1.0f, 0.f );
        float3 foam_color = float3( 0.9f, 0.9f, 0.9f );
        
        foam_color = lerp( floor_color, foam_color, noiseValue * refractionFactor );
        return float4( foam_color, 1.f );
    }

    //Get Specular
    float3 dir_to_eye = normalize( CAMERA_POSITION - input.world_position );
    light_t light = LIGHTS[0];
    float3 light_color      = light.color.xyz;
    float3 light_position   = light.world_position;
    float3 light_direction  = normalize(light.direction);

    float3 pos_to_light             = light_position - input.world_position;
    float dist_to_light_position    = length(pos_to_light);
    float directional_distance      = dot(-pos_to_light, light_direction);

    float3 dir_to_light = lerp(normalize(pos_to_light), -light_direction, light.is_directional);
    float dist_to_light = lerp(dist_to_light_position, directional_distance, light.is_directional);

    float cos_angle_to_light_dir = dot(-dir_to_light, light_direction);

    float att_factor    = smoothstep(light.cos_outter_half_angle, light.cos_inner_half_angle, cos_angle_to_light_dir);
    float3 att_vec      = float3(1.0f, dist_to_light, dist_to_light * dist_to_light);
    float specular_att  = (light.intensity / dot(att_vec, light.spec_attenuation)) * att_factor;

    float facing        = smoothstep(-0.5, 0.0f, dot(dir_to_light, world_normal));
    float3 half_vector  = normalize(dir_to_light + dir_to_eye);
    float specular      = max(0.0f, dot(world_normal, half_vector));
    specular            = SPECULAR_FACTOR * pow(specular, SPECULAR_POWER);
    specular            *= specular_att;
    specular            *= facing;

    float3 specular_color = light_color * specular;
    
	//Water Color
    float3 reflection_color = reflectivity * sky_color.xyz;
    float3 transmission_color = transmissivity * floor_color;
	float3 water_color = reflection_color + transmission_color;
    water_color += specular_color;
	//return float4(water_color, 1.f);


	//Jacobian Foam
	//float turbulance = max(2.0f - input.jacobian.x + dot(0.3f * normalize(world_normal.xy), float2(1.2f, 1.2f)), 0.f);
	float foam_color = float3( 0.2f, 0.4f, 0.5f );
    float jacobianSign = sign( input.jacobian.x );
    float jacobainStep = sqrt( sqrt( saturate(input.jacobian.x) ) );
    //jacobainStep = abs(jacobainStep);// * jacobianSign;
    jacobainStep = saturate( jacobainStep );
    float foam_factor = smoothstep( 1.f, 0.f, jacobainStep );
    water_color += foam_color * foam_factor;
	return float4( water_color, 1.f);
}
