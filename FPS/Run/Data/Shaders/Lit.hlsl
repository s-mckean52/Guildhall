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
   float3 bitangent		: BITANGENT;
   float3 normal		: NORMAL;
}; 

struct light_t
{
	float3 world_position;
	float intensity;
	
	float3 color;
	float padding_00;

	float3 attenuation;
	float padding_01;

	float3 spec_attenuation;
	float padding_02;
};

static float SHIFT = 0.75f;

cbuffer time_constants : register(b0)
{
	float SYSTEM_TIME_SECONDS;
	float SYSTEM_TIME_DELTA_SECONDS;
	float GAMMA;
	float INVERSE_GAMMA;
};

cbuffer camera_constants : register(b1)
{
	float4x4 PROJECTION; // CAMERA_TO_CLIP
	float4x4 VIEW;		//WORLD_TO_CAMERA

	float4x4 CAMERA_MODEL;
	float3 CAMERA_POSITION;
	float padding_00;
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
	light_t LIGHT;
}


Texture2D <float4> tDiffuse	: register(t0);
Texture2D <float4> tNormal	: register(t1);
SamplerState sSampler		: register(s0);


//--------------------------------------------------------------------------------------
// Programmable Shader Stages
//--------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------
// for passing data from vertex to fragment (v-2-f)
struct v2f_t 
{
   float4 position : SV_POSITION; 
   float4 color : COLOR; 
   float2 uv : UV; 

   float3 world_position : POSITION;
   float3 world_normal : NORMAL;
   float3 world_tangent : TANGENT;
}; 

float RangeMap( float val, float inMin, float inMax, float outMin, float outMax )
{
	float domain = inMax - inMin;
	float range = outMax - outMin;
	return ((val - inMin) / domain) * range + outMin;
}

//--------------------------------------------------------------------------------------
// Vertex Shader
v2f_t VertexFunction( vs_input_t input )
{
   v2f_t v2f = (v2f_t)0;

   float4 local_position = float4( input.position, 1.0f );
   float4 world_position = mul( MODEL, local_position );
   float4 camera_position = mul( VIEW, world_position );
   float4 clip_position = mul( PROJECTION, camera_position );

   float4 local_normal = float4( input.normal, 0.0f );
   float4 local_tangent = float4( input.tangent, 0.0f );
   float4 world_normal = mul( MODEL, local_normal );
   float4 world_tangent = mul( MODEL, local_tangent );

   v2f.position = clip_position;
   v2f.color = input.color * TINT;
   v2f.uv = input.uv;
   v2f.world_position = world_position.xyz;
   v2f.world_normal = world_normal.xyz;
   v2f.world_tangent = world_tangent.xyz;

   return v2f;
}

//--------------------------------------------------------------------------------------
// Fragment Shader
// 
// SV_Target0 at the end means the float4 being returned
// is being drawn to the first bound color target.
float4 FragmentFunction( v2f_t input ) : SV_Target0
{
	float3 dir_to_eye = normalize( CAMERA_POSITION - input.world_position );

	float3 normal = normalize( input.world_normal );
	float3 tangent = normalize( input.world_tangent );
	float3 bitangent = normalize( cross( input.world_normal, input.world_tangent ) );
	float3x3 tbn = float3x3( tangent, bitangent, normal );

	float4 texture_color = tDiffuse.Sample( sSampler, input.uv );
	float4 normal_color = tNormal.Sample( sSampler, input.uv );

	float3 surface_color = pow( texture_color.xyz, INVERSE_GAMMA.xxx );
	surface_color *= input.color.xyz;
	float surface_alpha = ( input.color.a * texture_color.a );

	float3 diffuse = AMBIENT.xyz * AMBIENT.w;
	float3 surface_normal = ( normal_color.xyz * float3( 2.0f, 2.0f, 1.0f ) ) - float3( 1.0f, 1.0f, 0.0f );
	float3 world_normal = mul( surface_normal, tbn );
	//return float4( world_normal, 1.f );

	float3 light_color = LIGHT.color.xyz;
	float3 light_position = LIGHT.world_position;
	float3 vec_to_light = light_position - input.world_position;
	float dist_to_light = length( vec_to_light );
	float3 dir_to_light = normalize( vec_to_light );
	float3 att_vec = float3( 1.0f, dist_to_light, dist_to_light * dist_to_light );

	float diffuse_att = LIGHT.intensity / dot( att_vec, LIGHT.attenuation );
	float specular_att = LIGHT.intensity / dot( att_vec, LIGHT.spec_attenuation );

	//Diffuse
	float dot3 = max( 0.0f, dot( dir_to_light, world_normal ) );

	//Specular
	//Phong
	//float3 dir_to_light_reflect = reflect( -dir_to_light, input.world_normal );
	//float specular = max( 0.0f, dot( dir_to_light_reflect, dir_to_eye ) );

	//Blinn-Phong
	float3 half_vector = normalize( dir_to_light + dir_to_eye );
	float specular = max( 0.0f, dot( world_normal, half_vector ) );
	specular = SPECULAR_FACTOR * pow( specular, SPECULAR_POWER );
	specular *= specular_att;

	diffuse += dot3 * diffuse_att * light_color;
	float3 specular_color = light_color * specular;

	diffuse = saturate( diffuse );

	float3 final_color = diffuse * surface_color + specular_color;
	return float4( final_color, surface_alpha );
}
