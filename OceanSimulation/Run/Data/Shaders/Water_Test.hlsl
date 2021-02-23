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

	float3 direction;
	float cos_inner_half_angle;

	float3 color;
	float cos_outter_half_angle;

	float3 attenuation;
	float is_directional;

	float3 spec_attenuation;
	float padding;
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
	light_t LIGHTS[8];
}


Texture2D <float4> tDiffuse	: register(t0);
Texture2D <float4> tNormal	: register(t1);
TextureCube<float4> tSkybox : register(t8);
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

float RangeMap(float val, float inMin, float inMax, float outMin, float outMax)
{
	float domain = inMax - inMin;
	float range = outMax - outMin;
	return ((val - inMin) / domain) * range + outMin;
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
	//Rotation Matricies for converting skybox to game basis
	float4x4 rotation_on_x = float4x4(
		float4(1.f, 0.f, 0.f, 0.f),
		float4(0.f, 0.f, 1.f, 0.f),
		float4(0.f, -1.f, 0.f, 0.f),
		float4(0.f, 0.f, 0.f, 1.f));
	float4x4 rotation_on_z = float4x4(
		float4( 0.f, 1.f, 0.f, 0.f),
		float4(-1.f, 0.f, 0.f, 0.f),
		float4( 0.f, 0.f, 1.f, 0.f),
		float4( 0.f, 0.f, 0.f, 1.f));

	//Water Constants
	float3 upwelling =	float3( 0.f, 0.2f, 0.3f );
	float3 sky =		float3( 0.69f, 0.84f, 1.f );
	float3 air =		float3( 0.1f, 0.1f, 0.1f );
	float nSnell	= 1.34f;
	float kDiffuse	= 0.91f;

	//Skybox Sample
	float3	incident	= normalize(input.world_position - CAMERA_POSITION);
	float3	reflection	= reflect(incident, input.world_normal);
			reflection	= mul(rotation_on_x, mul(rotation_on_z, reflection));
	float4	sky_color	= tSkybox.Sample(sSampler, reflection);


	{
		float reflectivity;
		float3 incident_normal = normalize( incident );
		float3 world_normal = normalize( input.world_normal );
		float costhetai = abs( dot( incident_normal, world_normal ) );
		float thetai = acos( costhetai );
		float sinthetat = sin( thetai ) / nSnell;
		float thetat = asin(sinthetat);
		if (thetai == 0.0)
		{
			reflectivity = (nSnell - 1) / (nSnell + 1);
			reflectivity = reflectivity * reflectivity;
		}
		else
		{
			float fs = sin(thetat - thetai) / sin(thetat + thetai);
			float ts = tan(thetat - thetai)	/ tan(thetat + thetai);
			reflectivity = 0.5 * (fs * fs + ts * ts);
		}
		float3 dPE = CAMERA_POSITION - input.world_position;
		float dist = 0.1f * kDiffuse;
		dist = exp(-dist);
		float3 Ci = dist * ( reflectivity * sky_color
							 + (1 - reflectivity) * upwelling)
							 + (1 - dist) * air;
		return float4( Ci, 1.f );
	}
}
