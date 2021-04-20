//--------------------------------------------------------------------------------------
// Stream Input
// ------
// Stream Input is input that is walked by the vertex shader.  
// If you say "Draw(3,0)", you are telling to the GPU to expect '3' sets, or 
// elements, of input data.  IE, 3 vertices.  Each call of the VertxShader
// we be processing a different element. 
//--------------------------------------------------------------------------------------

// inputs are made up of internal names (ie: uv) and semantic names
// (ie: TEXCOORD).  "uv" would be used in the shader file, where
// "TEXCOORD" is used from the client-side (cpp code) to attach ot. 
// The semantic and internal names can be whatever you want, 
// but know that semantics starting with SV_* usually denote special 
// inputs/outputs, so probably best to avoid that naming.
struct vs_input_t
{
	// we are not defining our own input data; 
	float3 position      : POSITION;
	float4 color         : COLOR;
	float2 uv            : TEXCOORD;

	float3 tangent		: TANGENT;
	float3 bitangent	: BITANGENT;
	float3 normal		: NORMAL;
};

struct light_t
{
	float3 world_position;
	float intensity;

	float3 direction;
	float cos_inner_half_angle;

	float3 color;
	float cos_outter_half_angle;

	float3 attenuation;
	float is_directional;

	float3 spec_attenuation;
	float padding;
};

cbuffer time_constants : register(b0)
{
	float SYSTEM_TIME_SECONDS;
	float SYSTEM_TIME_DELTA_SECONDS;
	float GAMMA;
	float INVERSE_GAMMA;
};

cbuffer camera_constants : register(b1)
{
	float4x4 PROJECTION;            // CAMERA_TO_CLIP
	float4x4 VIEW;		            //WORLD_TO_CAMERA

	float4x4 CAMERA_MODEL;
	float3 CAMERA_POSITION;
	float padding_00;
    
    float4x4 INVERSE_PROJECTION;    // CLIP_TO_CAMERA
}

cbuffer model_constants : register(b2)
{
	float4x4 MODEL;
	float4 TINT;

	float SPECULAR_FACTOR;
	float SPECULAR_POWER;
	float2 padding_01;
}

cbuffer light_constants : register(b3)
{
	float4 AMBIENT;
	light_t LIGHTS[8];
}

cbuffer terrain_constants : register(b5)
{
    float MIN_HEIGHT;
    float MAX_HEIGHT;
    float2 padding_02;
}


Texture2D<float4>	tDiffuse0	: register(t0);
Texture2D<float4>	tNormal0	: register(t1);
Texture2D<float4>	tDiffuse1   : register(t8);
Texture2D<float4>	tNormal1    : register(t9);

SamplerState sSampler			: register(s0);


//--------------------------------------------------------------------------------------
// Programmable Shader Stages
//--------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------
// for passing data from vertex to fragment (v-2-f)
struct v2f_t
{
	float4 position : SV_POSITION;
	float4 color    : COLOR;
	float2 uv       : UV;

    float3 camera_position  : POSITION0;
	float3 world_position   : POSITION1;
	float3 world_normal     : NORMAL;
	float3 world_tangent    : TANGENT;
	float3 world_bitangent  : BITANGENT;

	float4 jacobian : JACOBIAN;
};

//--------------------------------------------------------------------------------------
float RangeMap( float val, float inMin, float inMax, float outMin, float outMax )
{
	float domain = inMax - inMin;
	float range = outMax - outMin;
	return ((val - inMin) / domain) * range + outMin;
}

//--------------------------------------------------------------------------------------
float3 GetNormalFromColor( float3 sampledColor )
{
	return ( sampledColor * float3( 2.0f, 2.0f, 1.0f ) ) - float3( 1.0f, 1.0f, 0.0f );
}

//--------------------------------------------------------------------------------------
float3 GetColorFromNormalDir( float3 normalizedDir )
{
	return ( normalizedDir + float3( 1.f, 1.f, 1.f ) ) * float3( 0.5f, 0.5f, 0.5f );
}

//--------------------------------------------------------------------------------------
float3x3 GetVertexTBN( v2f_t input )
{
	float3 normal = normalize( input.world_normal );
	float3 tangent = normalize( input.world_tangent );
	float3 bitangent = normalize( input.world_bitangent );
	float3x3 tbn = float3x3( tangent, bitangent, normal );
	return tbn;
}


//--------------------------------------------------------------------------------------
float3 NDCToCameraSpace( float4 ndcPosition, float4x4 inverseProjection )
{
    float4 cameraPos = mul( inverseProjection, ndcPosition );
    cameraPos /= cameraPos.w;
    
    return cameraPos.xyz;
}


//--------------------------------------------------------------------------------------
// Vertex Shader
v2f_t VertexFunction(vs_input_t input)
{
	v2f_t v2f = (v2f_t)0;

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

	return v2f;
}

//--------------------------------------------------------------------------------------
// Fragment Shader
// 
// SV_Target0 at the end means the float4 being returned
// is being drawn to the first bound color target.
float4 FragmentFunction(v2f_t input) : SV_Target0
{   
    //Triplanar
	float2 x_uv = frac( input.world_position.yz );
	float2 y_uv = frac( input.world_position.xz );
    y_uv.x = 1.0f - y_uv.x;
	float2 z_uv = frac( input.world_position.xy );

    float4 x_color;
    float4 y_color;
    float4 z_color;
    
    if( input.world_position.z < -10.f )
    {
	    x_color = tDiffuse1.Sample( sSampler, x_uv );// * float4( 1.f, 0.25f, 0.25f, 1.f );
	    y_color = tDiffuse1.Sample( sSampler, y_uv );// * float4( 0.25f, 1.f, 0.25f, 1.f );
	    z_color = tDiffuse0.Sample( sSampler, z_uv );// * float4( 0.25f, 0.25f, 1.f, 1.f );
    }
    else
    {
	    x_color = tDiffuse1.Sample( sSampler, x_uv );
	    y_color = tDiffuse1.Sample( sSampler, y_uv );
	    z_color = tDiffuse0.Sample( sSampler, z_uv );
    }

	//float3 x_normal = tXNormal.Sample( sSampler, x_uv ).xyz;
	//float3 y_normal = tYNormal.Sample( sSampler, y_uv ).xyz;
	//float3 z_normal = tZNormal.Sample( sSampler, z_uv ).xyz;
    //
	//x_normal = ( x_normal * float3( 2.0f, 2.0f, 1.0f ) ) - float3( 1.0f, 1.0f, 0.0f );
	//y_normal = ( y_normal * float3( 2.0f, 2.0f, 1.0f ) ) - float3( 1.0f, 1.0f, 0.0f );
	//z_normal = ( z_normal * float3( 2.0f, 2.0f, 1.0f ) ) - float3( 1.0f, 1.0f, 0.0f );

    float3 x_normal = float3( 1.f, 0.f, 0.f );
    float3 y_normal = float3( 0.f, 1.f, 0.f );
    float3 z_normal = float3( 0.f, 0.f, 1.f );
    
	float3x3 x_tbn = float3x3(
		float3( 0.0f, 0.0f, -1.0f ),
		float3( 0.0f, 1.0f, 0.0f ),
		float3( 1.0f, 0.0f, 0.0f )
	);
	x_normal = normalize( mul( x_normal, x_tbn ) );
	x_normal *= sign( input.world_normal.x );

	float3x3 y_tbn = float3x3(
		float3( 1.0f, 0.0f, 0.0f ),
		float3( 0.0f, 0.0f, -1.0f ),
		float3( 0.0f, 1.0f, 0.0f )
	);
	y_normal = normalize( mul( y_normal, y_tbn ) );
	y_normal *= sign( input.world_normal.y );

	z_normal *= sign( input.world_normal.z );

	//Final Calculation
	float3 weights = normalize( input.world_normal.xyz );
	weights = weights * weights;

	float4 triplanar_color
		= weights.x * x_color
		+ weights.y * y_color
		+ weights.z * z_color;

	float3 triplanar_normal
		= weights.x * x_normal
		+ weights.y * y_normal
		+ weights.z * z_normal;

	triplanar_normal = normalize( triplanar_normal );
    
    return triplanar_color;
    
    float3 normal = GetColorFromNormalDir( normalize( input.world_normal ) );
    return float4( normal, 1.f );
}
