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
	float4 color;
};

static float SHIFT = 0.75f;

cbuffer time_constants : register(b0)
{
	float SYSTEM_TIME_SECONDS;
	float SYSTEM_TIME_DELTA_SECONDS;
};

cbuffer camera_constants : register(b1)
{
	float4x4 PROJECTION; // CAMERA_TO_CLIP
	float4x4 VIEW;		//WORLD_TO_CAMERA
}

cbuffer model_constants : register(b2)
{
	float4x4 MODEL;
}

cbuffer light_constants : register(b3)
{
	float4 AMBIENT;
	light_t LIGHT;
}


Texture2D <float4> tDiffuse	: register(t0);
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
   float4 world_normal = mul( MODEL, local_normal );

   v2f.position = clip_position;
   v2f.color = input.color;
   v2f.uv = input.uv;
   v2f.world_position = world_position.xyz;
   v2f.world_normal = world_normal.xyz;

   return v2f;
}

//--------------------------------------------------------------------------------------
// Fragment Shader
// 
// SV_Target0 at the end means the float4 being returned
// is being drawn to the first bound color target.
float4 FragmentFunction( v2f_t input ) : SV_Target0
{
	float4 texture_color = tDiffuse.Sample( sSampler, input.uv );
	float3 surface_color = ( input.color * texture_color ).xyz;
	float surface_alpha = ( input.color.a * texture_color.a );

	float3 diffuse = AMBIENT.xyz * AMBIENT.w;
	float3 surface_normal = normalize( input.world_normal );

	float3 light_color = LIGHT.color.xyz;
	float3 light_position = LIGHT.world_position;
	float3 dir_to_light = normalize( light_position - input.world_position );
	float dot3 = max( 0.0f, dot( dir_to_light, surface_normal ) ) * LIGHT.intensity;

	diffuse += dot3 * light_color;

	diffuse = saturate( diffuse );
	float3 final_color = diffuse * surface_color;

	return float4( final_color, surface_alpha );
}
