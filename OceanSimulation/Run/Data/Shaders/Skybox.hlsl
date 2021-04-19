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

	float4x4 CAMERA_MODEL;
	float3 CAMERA_POSITION;
	float padding_00;
}

cbuffer model_constants : register(b2)
{
	float4x4 MODEL;
	float4 TINT;
}

TextureCube<float4> tSkybox	: register(t0);
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
	float3 world_bitangent : BITANGENT;
}; 


//--------------------------------------------------------------------------------------
// Vertex Shader
v2f_t VertexFunction( vs_input_t input )
{
	v2f_t v2f = (v2f_t)0;

	float4 local_position = float4(input.position.xyz, 1.0f);
	float4 world_position = mul(MODEL, local_position);

	float4x4 view = VIEW;
	view[0][3] = 0.f;
	view[1][3] = 0.f;
	view[2][3] = 0.f;
	view[3][3] = 1.f;

	float4 camera_position = mul(view, world_position);
	float4 clip_position = mul(PROJECTION, camera_position);

	float4 local_normal = float4(input.normal, 0.0f);
	float4 local_tangent = float4(input.tangent, 0.0f);
	float4 local_bitangent = float4(input.bitangent, 0.0f);

	float4 world_normal =		mul(MODEL, local_normal);
	float4 world_tangent =		mul(MODEL, local_tangent);
	float4 world_bitangent =	mul(MODEL, local_bitangent);

	v2f.position = clip_position;
	v2f.color = input.color * TINT;
	v2f.uv = input.uv;
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
float4 FragmentFunction( v2f_t input ) : SV_Target0
{
	// test skybox - will show skybox on outside of sphere
	//float4 sky_color = tSkybox.Sample(sSampler, input.world_normal );
	//return sky_color;

	// sphere of glass
	float3 incident = normalize( input.world_position );
	float3 reflection = incident;// reflect(incident, input.world_normal);

	float4x4 rotation_on_x = float4x4(
		float4( 1.f, 0.f, 0.f, 0.f ),
		float4( 0.f, 0.f, 1.f, 0.f ),
		float4( 0.f, -1.f, 0.f, 0.f ),
		float4( 0.f, 0.f, 0.f, 1.f ) );

	float4x4 rotation_on_z = float4x4(
		float4( 0.f, 1.f, 0.f, 0.f ),
		float4( -1.f, 0.f, 0.f, 0.f ),
		float4( 0.f, 0.f, 1.f, 0.f ),
		float4( 0.f, 0.f, 0.f, 1.f ) );
	reflection = mul( rotation_on_x, mul( rotation_on_z, float4( reflection, 0.f ) ) );
	float4 color = tSkybox.Sample(sSampler, reflection.xyz);
	
	return color;
}
